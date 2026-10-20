# Firmware

Bare-metal C/C++ for the nRF52832 alarm clock (no Zephyr/RTOS application builds).
Built with nRF5 SDK 17.1.0 (`NRF5_SDK_DIR`), `arm-none-eabi-gcc`, and OpenOCD.

| Path | Purpose |
| --- | --- |
| `apps/blinky/` | Controller-only LED test (breadboard guide section 7) |
| `apps/bringup/` | Breadboard peripheral tests: display, LIS3DH, DCF-77, buzzer |
| `apps/clock/` | LFXO civil clock, UART set, e-paper HH:MM |
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

gcc -Wall -Wextra -I firmware/src firmware/tests/clock_test.c \
    firmware/src/clock.c -o /tmp/clock_test && /tmp/clock_test

gcc -Wall -Wextra -I firmware/tools/dcf77-generator/src \
    firmware/tests/dcf77_encode_test.c \
    firmware/tools/dcf77-generator/src/dcf77_encode.c \
    -o /tmp/dcf_encode_test && /tmp/dcf_encode_test

gcc -Wall -Wextra -I firmware/src firmware/tests/clock_face_test.c \
    firmware/src/clock_face.c -o /tmp/clock_face_test && /tmp/clock_face_test
```

## Motion dismiss

While an alarm sounds, the clock app arms the LIS3DH motion interrupt on INT1 (`P0.23`) and powers it down when the alarm stops. The MCU sleeps between events. The INT1 handler only sets a flag. The main loop reads `INT1_SRC` and feeds `motion_count_event()`.

| Constant (`src/motion_count.h`) | Value | Meaning |
|---|---|---|
| `MOTION_N` | 20 | counted events needed to silence |
| `MOTION_WINDOW_MS` | 30000 | all N events must fall inside this window |
| `MOTION_GAP_MS` | 300 | pulses closer than this count once |

Any button still silences the alarm at once. The UART prints `MOTION event n`, then `ALARM stop (motion)`. There is no host test for this module. It is checked on the bench (see the OpenSpec change `add-motion-dismiss`, section 4).
