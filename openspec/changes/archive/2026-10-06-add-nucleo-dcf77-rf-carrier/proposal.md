# Proposal

## Why

The logic-level generator (archived change `2026-10-06-add-nucleo-dcf77-generator`)
tests the DK decoder path but bypasses the DCF-1060N-800 receiver. Indoor
reception gave no valid pulses (bring-up task 4.3). So far nothing proves that
the module, its ferrite antenna, and the `PON` / `OUT` wiring can receive and
demodulate a DCF-77 signal at all. A controllable 77.5 kHz test signal answers
that and gives repeatable data for the thesis measurement chapter (output
delay, pulse-width distortion, and the lowest level that still decodes).

This replaces the RF non-goal of the previous change, which the user has now
explicitly requested.

## What Changes

- The Nucleo-F411RE generator emits an exact 77.5 kHz carrier on `PA6`
  (Arduino `D12`) from a timer PWM. It reduces the amplitude to about 15%
  during each pulse, as the DCF-77 transmitter does, in step with the existing
  `PA0` logic output.
- The system clock moves to 77.5 MHz from the PLL, so the carrier divides
  exactly from the external 8 MHz clock. The SysTick and UART are retimed. If
  the external clock is missing, the carrier stays off, because the internal
  oscillator is too far from 77.5 kHz for the receiver's crystal filter.
- New UART keys: `l` cycles four carrier levels (0, -6, -12, -20 dB) and `r`
  toggles the carrier on and off. The existing fault keys act on the carrier
  too (for example, silence becomes an unmodulated carrier).
- A coupling loop (a few turns of hookup wire with a series resistor from
  `PA6` to GND) is placed near the module's ferrite antenna. The guide gets
  the wiring, placement, and a reception test through the DK bring-up DCF test.
- `PA0` keeps working as a reference trace, so the analyzer can compare the
  sent pulses with the module's `OUT`.

Non-goals:

- DCF-77 phase modulation (pseudo-random phase keying). AM receivers such as
  the SP6007 do not use it.
- Range or field-strength calibration in absolute units (µV/m). The levels are
  relative to full drive.
- Any product role for STM32 or for transmitting. Lab equipment only.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `dcf77-test-generator`: adds the modulated 77.5 kHz carrier output, its
  clock dependency, level and on/off control, and the fault behavior on the
  carrier. Existing requirements keep their text, and the logic output stays
  as specified.

## Impact

- Code: `firmware/tools/dcf77-generator/src/main.c` (clock tree, TIM3 PWM, key
  handling) and `src/stm32f411_regs.h` (PLL, FLASH, TIM3 registers).
- Docs: the generator README (pins, keys, test record) and a new subsection in
  `hardware/docs/breadboard-prototype-guide.md` section 11.
- Hardware: about 1 m of hookup wire and one resistor (1 kΩ to 10 kΩ) from the
  proto kit. No new purchase and no NixOS change.
