# Spec Delta

## Purpose

Provides a bench signal source on an STM32 Nucleo-F411RE that emits
logic-level DCF-77 time frames with an operator-set time and injectable faults,
so the clock's DCF-77 decoding can be tested repeatably without radio
reception.

## ADDED Requirements

### Requirement: Generator builds and flashes with installed tools
The generator firmware SHALL build with the installed `arm-none-eabi-gcc` and flash through the Nucleo's on-board ST-LINK with the installed OpenOCD, using its `board/st_nucleo_f4.cfg`. It SHALL NOT need STM32Cube, a vendor HAL, an RTOS, a NixOS change, or a download.

#### Scenario: Operator builds and flashes
- **WHEN** the operator runs `make` and then `make flash` in the generator directory with the Nucleo connected over USB
- **THEN** the build completes without errors and OpenOCD programs, verifies, and resets the board

### Requirement: Output follows DCF-77 pulse timing
The generator SHALL drive output `PA0` (Arduino header `A0`) as a 3.3 V push-pull signal that is high during each pulse and low otherwise. A pulse SHALL start at each second 0 to 58 and last 100 ms for a `0` bit or 200 ms for a `1` bit. Second 59 SHALL have no pulse, so that the next pulse marks the minute.

#### Scenario: Normal frame on the analyzer
- **WHEN** the generator runs from the external 8 MHz clock with no fault selected and a logic analyzer records `PA0` for two minutes
- **THEN** pulse starts are 1000 ms apart, except one 2000 ms gap per minute; each pulse is 100 ms or 200 ms wide; and the timing error is at most 2 ms

#### Scenario: Clock source is reported
- **WHEN** the generator starts
- **THEN** it reports over UART whether its timebase runs from the external 8 MHz clock or the internal oscillator

### Requirement: Frames encode time per the DCF-77 format
Each frame SHALL encode, for the minute that starts at the next minute mark: bit 0 = `0`, bits 1 to 16 = `0`, the time-zone bits (17 = CEST, 18 = CET), bit 19 = `0`, bit 20 = `1`, then BCD minute, hour, day of month, weekday (1 = Monday to 7 = Sunday), month, and two-digit year. Each field group SHALL have its even-parity bit at 28, 35, and 58.

#### Scenario: Frame for a known time
- **WHEN** the generator is set to 2026-10-06 20:15 CEST
- **THEN** the frame ending at the 20:15 minute mark carries minute 15, hour 20, day 6, weekday 2, month 10, year 26, bit 17 set, bit 18 clear, and three correct parity bits

#### Scenario: Minute rollover
- **WHEN** a frame for 23:59 on the last day of a month is followed by the next frame
- **THEN** the next frame encodes 00:00 on the first day of the next month, with the weekday advanced by one, and a year change at 31 December

### Requirement: Operator sets the start time over UART
The generator SHALL accept a command that sets the date, time, and time zone (CET or CEST) over the ST-LINK virtual COM port. It SHALL compute the weekday from the date. A new time SHALL take effect at the next frame start, and the generator SHALL reject invalid input with a message and keep the current time.

#### Scenario: Valid time command
- **WHEN** the operator sends a valid date, time, and zone
- **THEN** the generator confirms the new time, and the next full frame encodes it

#### Scenario: Invalid time command
- **WHEN** the operator sends an impossible date such as 2026-02-30, or malformed text
- **THEN** the generator reports an error and continues the current frame sequence unchanged

#### Scenario: No time command after reset
- **WHEN** the generator starts without a time command
- **THEN** it encodes a fixed default time from its build and reports it in the startup banner

### Requirement: Operator injects faults over UART
The generator SHALL accept single-key commands that inject a fault: a wrong parity bit in the next frame, a next frame whose date differs from the sequence while its parity stays correct, a missing pulse in the next second, a short glitch pulse of at most 20 ms, and continuous silence (output held low) until the command repeats. A clear command SHALL cancel pending faults. One-shot faults SHALL apply once and then clear.

#### Scenario: Parity fault
- **WHEN** the operator selects a parity fault
- **THEN** exactly the next frame carries one wrong parity bit, the frame log marks it, and the frame after it is correct

#### Scenario: Wrong-date fault
- **WHEN** the operator selects a wrong-date fault
- **THEN** the next frame encodes a different but valid date with correct parity, and the following frame returns to the true sequence

#### Scenario: Silence
- **WHEN** the operator selects silence and later selects it again
- **THEN** `PA0` stays low in between, and the signal resumes at the next frame start

### Requirement: Generator logs each frame over UART
The generator SHALL print human-readable text at 115200 baud, 8N1, on the ST-LINK virtual COM port. It SHALL print a startup banner with the firmware name, clock source, default time, and command list. Before each frame it SHALL print the encoded date, time, weekday, zone, the 59-bit string, and any fault applied to that frame.

#### Scenario: Comparing logs
- **WHEN** the generator drives the DK's DCF-77 input and both serial consoles are open
- **THEN** each frame line in the generator log can be matched to the pulses in the DK log by its bit string

### Requirement: Frame encoder has a host check
The frame encoder SHALL have a host-side check that builds with the host `gcc`, like the existing DCF-77 classifier check. It SHALL compare the encoder output with a reference frame derived by hand from the DCF-77 format, and it SHALL check the minute, month, and year rollovers.

#### Scenario: Encoder regression
- **WHEN** the encoder changes and the host check runs
- **THEN** the check fails if any bit of the reference frame or any rollover result differs
