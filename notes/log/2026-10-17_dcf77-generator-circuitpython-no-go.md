# 2026-10-17 — DCF-77 generator in CircuitPython: verdict `no-go`

OpenSpec change `rewrite-dcf77-generator-in-circuitpython`, tasks 1.1–1.3. Scope: `firmware/tools/dcf77-generator/` (Nucleo-F411RE, C).

## Facts checked

- CircuitPython `ports/stm/boards/` has no `nucleo_f411re` (checked 2026-10-17). Nearest supported F411 boards: `stm32f411ce_blackpill`, `stm32f411ve_discovery`.
- CircuitPython needs native USB on `PA11`/`PA12`. The Nucleo has no user USB connector. The ST-LINK virtual COM port cannot carry the REPL or the `CIRCUITPY` drive.

## Options (design decision 1)

| Option | Needs | Result |
|---|---|---|
| A: custom `nucleo_f411re` build + USB breakout on `PA11`/`PA12` | source build of CircuitPython, wiring work | not chosen |
| B: `stm32f411ce_blackpill` | buying a Black Pill (not on hand) | not chosen |
| C: no-go | nothing | **chosen by the author** |

## Decision

No-go. The author chose to keep the C generator. Reason: the port needs extra hardware (B) or a source build plus rewiring (A); the generator is lab equipment and the C version already meets the recorded timing and carrier results (2026-10-06).

Task 1.2 (measure `pwmio` at 77.5 kHz) was not run: it needs a board running CircuitPython. So the `pwmio` frequency and the software pulse-jitter risks in the design stay unmeasured. Not a negative result, only an untested one.

Consequence: nothing changes in `firmware/`. The DK bring-up DCF test and `add-dcf77-periodic-sync` keep using the C generator.

Thesis note: the C generator already uses a hardware timer (TIM3) for the carrier and a hardware USART. Do not describe it as bit-banged.
