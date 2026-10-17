# Proposal

## Why

The bench DCF-77 generator (`firmware/tools/dcf77-generator`) carries about 170 lines of hand-written boot and register code (`stm32f411_regs.h`, `startup.c`, `stm32f411re.ld`) that the author wants gone. The archived CircuitPython rewrite failed because the Nucleo-F411RE has no CircuitPython board and no user USB connector. STM32Cube, CubeMX and CubeIDE are not acceptable. libopencm3 is a small open-source C library built with plain `make` and the installed `arm-none-eabi-gcc`, so it removes the boot and register code without those tools.

This is an experiment. It must be easy to drop: the register-level version stays in git history and is restored if the port does not match the recorded results.

## What Changes

- Add libopencm3 as a git submodule under `firmware/third_party/libopencm3`, pinned to one commit.
- Replace `stm32f411_regs.h`, `startup.c` and `stm32f411re.ld` with libopencm3 headers, its vector table and its linker script.
- Port `main.c` (clock, GPIO, TIM3 carrier, SysTick, USART2) to libopencm3 calls with unchanged behavior: same pins, UART commands, fault keys, carrier levels and timing.
- Keep `dcf77_encode.c` and its host check unchanged.
- Update the `Makefile` to build the library for `stm32/f4` first, then link the app. Flashing stays `openocd -f board/st_nucleo_f4.cfg`.
- Re-run the 2026-10-06 generator checks (pulse widths, minute gap, carrier 77.5 kHz, DK decode) and compare.
- Work on a branch. Merge only if the checks match. Otherwise drop the branch and keep the register-level C; record the reason in `notes/log/` either way.

## Capabilities

### New Capabilities
<!-- none -->

### Modified Capabilities
- `dcf77-test-generator`: the build requirement now allows a pinned open-source library (libopencm3, submodule) and still forbids vendor HAL, STM32Cube and an RTOS. Timing, carrier, UART and fault requirements stay.

## Impact

- `firmware/tools/dcf77-generator/` (sources, `Makefile`, `README.md`), `.gitmodules`, new `firmware/third_party/libopencm3`, `notes/log/`.
- The DK bring-up DCF test and `add-dcf77-periodic-sync` use this generator; the register-level version stays available until the port is verified.
- Build needs `make` and `arm-none-eabi-gcc` (installed) plus `git submodule update --init`. libopencm3's build scripts need `python3` (installed). No NixOS change.
- No product firmware change.
- Supersedes the dropped CircuitPython route (`archive/2026-10-17-rewrite-dcf77-generator-in-circuitpython`).
