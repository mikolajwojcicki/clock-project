# Firmware

Bare-metal C/C++ for the nRF52832 alarm clock (no Zephyr/RTOS application builds).
Built with nRF5 SDK 17.1.0 (`NRF5_SDK_DIR`), `arm-none-eabi-gcc`, and OpenOCD.

| Path | Purpose |
| --- | --- |
| `apps/blinky/` | Controller-only LED test (breadboard guide section 7) |
| `apps/bringup/` | Breadboard peripheral tests: display, LIS3DH, DCF-77, buzzer |
| `src/` | Shared register-level drivers used by the apps |
| `boards/` | Pin maps (`dk_breadboard_pins.h` for the nRF52 DK breadboard) |
| `tests/` | Host-side checks for pure logic (built with host `gcc`) |
| `tools/` | Bench equipment firmware (not product MCU), e.g. Nucleo DCF-77 generator |
| `docs/` | Bring-up notes, memory map, power states |

Stack and tooling constraints: repo-root [`AGENTS.md`](../AGENTS.md). Prototyping hardware: [`hardware/bom/proto-bom.md`](../hardware/bom/proto-bom.md).

## Host checks

From the repo root (if `gcc` is missing, wrap in `nix-shell -p gcc --run '...'`):

```bash
gcc -I firmware/src firmware/tests/dcf77_classify_test.c \
    firmware/src/dcf77_classify.c -o /tmp/dcf_test && /tmp/dcf_test

gcc -Wall -Wextra -I firmware/tools/dcf77-generator/src \
    firmware/tests/dcf77_encode_test.c \
    firmware/tools/dcf77-generator/src/dcf77_encode.c \
    -o /tmp/dcf_encode_test && /tmp/dcf_encode_test
```
