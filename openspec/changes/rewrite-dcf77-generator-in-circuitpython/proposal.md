## Why

The bench DCF-77 generator (`firmware/tools/dcf77-generator`, Nucleo-F411RE) is about 575 lines of C plus a register header, startup code and a linker script, all written by hand with no vendor HAL. The author wants it as CircuitPython if possible. It is lab equipment, not product firmware, so the bare-metal rule in `AGENTS.md` does not apply to it.

**Feasibility is not settled.** Checked 2026-10-17 against the CircuitPython repo: `ports/stm/boards/` has no `nucleo_f411re`. It has `nucleo_f446re`, `nucleo_f746zg`, `nucleo_f767zi`, `stm32f411ce_blackpill`, `stm32f411ve_discovery` and others. The STM port supports the F411 chip family, so a custom board definition is possible, but the Nucleo has no user USB connector, and CircuitPython needs native USB (`PA11`/`PA12`). The ST-LINK virtual COM port will not carry the CircuitPython REPL or `CIRCUITPY` drive. So the first step is a go / no-go.

## What Changes

- Feasibility gate: decide between (A) a custom `nucleo_f411re` CircuitPython build plus a USB breakout on `PA11`/`PA12`, (B) a board that CircuitPython already supports (`stm32f411ce_blackpill`), or (C) no-go, keep the C generator and record why.
- If go: port the generator to CircuitPython (`pwmio`, `digitalio`, `time`, `usb_cdc` / REPL) with the same pins, UART commands, fault keys and carrier levels.
- Move the DCF-77 frame encoder out of C; a pure-Python encoder gets a host check.
- Re-run the recorded generator checks (pulse widths, minute gap, carrier, DK decode) and compare with the 2026-10-06 results.
- Keep the C generator in the tree until the Python version matches the recorded results; then remove it in a separate commit.

## Capabilities

### New Capabilities
<!-- none -->

### Modified Capabilities
- `dcf77-test-generator`: the build-and-flash requirement changes if the port goes ahead; timing and carrier requirements stay.

## Impact

- `firmware/tools/dcf77-generator/` (later a Python dir), `firmware/tests/dcf77_encode_test.c` or a Python equivalent, `firmware/README.md`, `notes/log/`.
- NixOS: flashing a CircuitPython build needs a DFU tool (`dfu-util`) via `nix-shell`; nothing global.
- The DK bring-up DCF test and the `add-dcf77-periodic-sync` change use this generator; keep it working until the port is verified.
- No product firmware change.
