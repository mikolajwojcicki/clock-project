# Proposal

## Why

The DCF-1060N-800 receiver produced no valid pulses at the bench during the
breadboard bring-up (task 4.3, 2026-10-06), and the user's commercial DCF-77
clock also receives poorly in that room and has shown wrong dates. The future
DCF-77 decoder needs a repeatable, controllable signal source now, including
deliberately broken frames, so its parity and frame-agreement checks can be
tested without waiting for good reception.

## What Changes

- Add a bare-metal C firmware for the user's STM32 Nucleo-F411RE that outputs a
  logic-level DCF-77 time signal: one pulse per second, 100 ms for a `0` bit,
  200 ms for a `1` bit, no pulse at second 59, with correct BCD fields and
  even parity. It uses the same polarity as the receiver (`DCF_OUT` high
  during the pulse).
- Encode a start time that the operator sets over the ST-LINK virtual COM port,
  and advance it one minute per frame.
- Let the operator inject faults over UART: a parity error, a single frame with
  a wrong date, a missing pulse, a short glitch, and silence.
- Print each frame (decoded fields and bit string) over UART before it is
  sent, so the DK log can be compared with the generator log.
- Add a host-side check of the frame encoder against a known reference frame.
- Document the wiring between the Nucleo output and the DK `P0.25`, which
  replaces the receiver output for decoder tests.

Non-goals:

- RF generation (a 77.5 kHz carrier into a loop antenna). It would test the
  receiver module rather than the decoder, and it adds analog and regulatory
  work that is out of thesis scope.
- The DCF-77 decoder on the nRF52832. That is a separate change, and it uses
  this generator as its test bench.
- Any product role for STM32. The Nucleo is lab equipment only; the clock's MCU
  stays the nRF52832 (`MDBT42Q-512KV2`), and the superseded STM32L0 product
  choice is not reintroduced.

## Capabilities

### New Capabilities

- `dcf77-test-generator`: a bench signal source that emits logic-level DCF-77
  frames with operator-set time and injectable faults, for testing the clock's
  DCF-77 decoding without radio reception.

### Modified Capabilities

None. The bring-up firmware's DCF-77 test already accepts any push-pull
active-high signal on `P0.25`, so its requirements do not change.

## Impact

- New code under `firmware/tools/dcf77-generator/` (own Makefile, linker
  script, startup code, and register definitions; no STM32Cube HAL or other
  download).
- New host check under `firmware/tests/`.
- Tooling: the already-installed `arm-none-eabi-gcc` and OpenOCD
  (`board/st_nucleo_f4.cfg`); the openocd udev rule is already present. No
  NixOS changes.
- Docs: a wiring section in `hardware/docs/breadboard-prototype-guide.md` and
  a row for the Nucleo in the prototype inventory.
