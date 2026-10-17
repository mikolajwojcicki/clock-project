## Context

The C generator uses SysTick at 1 ms, an ISR that opens a pulse at each second, TIM3 CH1 as the 77.5 kHz carrier with duty switching at each pulse edge, USART2 on the ST-LINK VCP, and a pure encoder `dcf77_encode.c` with a host check. The PLL is set up by hand for an exact 77.5 MHz core clock so TIM3 divides to 77.5 kHz.

CircuitPython facts checked on 2026-10-17: the STM port is documented at docs.circuitpython.org; `ports/stm/boards/` lists `nucleo_f446re`, `nucleo_f746zg`, `nucleo_f767zi`, `nucleo_h743zi_2`, `stm32f411ce_blackpill`, `stm32f411ve_discovery` and more, but no `nucleo_f411re`.

## Goals / Non-Goals

**Goals:** a go / no-go with evidence; if go, the same behavior in CircuitPython; the recorded checks re-run.

**Non-Goals:** any product firmware change; MicroPython unless CircuitPython is ruled out and the author agrees; a new generator feature set.

## Decisions

1. **Feasibility gate first.** Options: (A) custom CircuitPython board for the Nucleo-F411RE plus a USB breakout on `PA11`/`PA12`, built from source; (B) the supported `stm32f411ce_blackpill` (needs that board, USB-C included); (C) no-go. Pick by what hardware is on hand and the effort of a source build. Ask the author at the start of apply.
2. **Carrier stays hardware.** Use `pwmio.PWMOut` at 77.5 kHz and change `duty_cycle` at pulse edges. `pwmio` frequency is set from the timer clock; the exact 77.5 kHz depends on the clock tree of the chosen board and MUST be measured. If the achievable frequency is off by more than the receiver tolerance, record it; do not hack the clock tree from Python.
3. **Pulse timing is software.** `time.monotonic_ns()` schedules edges; target ±2 ms for pulses and the minute gap (as recorded). Python and garbage collection can add jitter; call `gc.collect()` and `gc.disable()` after init and measure. The dip lag will be larger than the recorded 12 us; measure it and decide if it matters for the receiver.
4. **Console.** CircuitPython serial is USB CDC, not the ST-LINK VCP. Commands (`YYYY-MM-DD HH:MM:SS`, fault keys `g m s p`, carrier keys `l r`) are read from `usb_cdc.console` or `sys.stdin` without blocking.
5. **Encoder in Python** with a host check run in `nix-shell -p python3`; same bit strings as the C encoder for fixed inputs. Cross-check by running both on the same times.
6. **Keep the C generator until verified,** then remove it and its Makefile, linker script, startup and register header in one separate commit. Reason: the DK DCF test and `add-dcf77-periodic-sync` depend on it.
7. **Not bit-banging in the strict sense.** The C version already uses a hardware timer for the carrier and a hardware USART. The gain from Python is less code and no register header, not fewer hardware peripherals. Record that in the note so the thesis does not overclaim.

## Risks / Trade-offs

- Nucleo-F411RE has no CircuitPython board; option A needs a source build and a USB breakout (hardware work).
- Software pulse timing may miss ±2 ms under load. Mitigation: measure before accepting; fall back to no-go.
- Python is slower; carrier dip lag may exceed one carrier period. Needs the receiver test (DCF-1060N-800) to show it still locks.
- The existing results were measured with the C generator; the Python version must be re-measured, not assumed equal.
