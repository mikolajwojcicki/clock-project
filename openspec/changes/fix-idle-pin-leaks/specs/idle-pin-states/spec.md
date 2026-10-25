# Spec Delta

## Purpose

Defines the state of each MCU line to a peripheral while the MCU sleeps, so that no line drives current into a peripheral and the board idles near the nRF-only current.

## ADDED Requirements

### Requirement: Display reset line is released while idle
After every e-paper refresh, whether it succeeded or failed, the firmware SHALL leave the display reset line undriven (input, no pull, input buffer disconnected). The next refresh SHALL drive the line again and start with a reset pulse.

#### Scenario: Idle after a refresh
- **WHEN** a refresh has finished and the MCU sleeps
- **THEN** the MCU does not drive the display reset line

#### Scenario: Refresh failed on BUSY timeout
- **WHEN** a refresh ends early because the panel did not answer
- **THEN** the display reset line is released in the same way

#### Scenario: Next minute
- **WHEN** the next minute refresh runs
- **THEN** the panel wakes from deep sleep and shows the new time

### Requirement: SPI clock and data-out are high between transfers
Between SPI transfers, from bus init on, the firmware SHALL drive the shared SPI clock and data-out lines high. This holds for every device on the bus and every SPI mode.

#### Scenario: Idle with the LIS3DH breakout fitted
- **WHEN** the MCU sleeps and the breakout pull-ups hold its SPI inputs high
- **THEN** the clock and data-out lines are driven high, so no current flows through those pull-ups

#### Scenario: Last transferred bit was zero
- **WHEN** a transfer ends with data-out low
- **THEN** data-out is driven high before the MCU sleeps

#### Scenario: LIS3DH unplugged
- **WHEN** the breakout is removed and the MCU sleeps
- **THEN** the clock and data-out lines are still at a defined high level at the display inputs

### Requirement: Peripheral traffic is unchanged
The display and the accelerometer SHALL work as before the change: the panel refreshes every minute, the accelerometer identifies itself, and motion during an alarm still silences it.

#### Scenario: Accelerometer after the change
- **WHEN** the clock starts with the LIS3DH fitted
- **THEN** the accelerometer passes its identity check and motion dismiss works

### Requirement: Full-board idle has no pin leak left
With every peripheral powered and wired, the idle current between refreshes SHALL equal the nRF-only idle figure (run A8) plus the standby currents of the powered parts (datasheet figures and known breakout loads such as its power LED), within 10 µA. The log SHALL list each term of that sum.

#### Scenario: Run C with all parts fitted
- **WHEN** the board idles for at least 60 s between refreshes on one 3.0 V PPK2 rail with display, DCF receiver and LIS3DH wired
- **THEN** the mean idle current minus the listed standby terms is within 10 µA of the A8 figure, and the capture and log entry record it

#### Scenario: Remainder too large
- **WHEN** the remainder exceeds 10 µA
- **THEN** the log records it as an open leak and the change is not archived
