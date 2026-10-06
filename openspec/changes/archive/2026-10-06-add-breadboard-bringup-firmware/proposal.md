# Proposal

## Why

`hardware/docs/breadboard-prototype-guide.md` defines wiring and test
observations for the e-paper display, LIS3DH, DCF-77 receiver, and buzzer, but
sections 9.2, 10.2, 11.3, and 12.3 all say "needs a later driver". The only
in-tree firmware is `firmware/apps/blinky/`, so the breadboard prototype cannot
pass its own completion checklist. The guide's safe control states
(section 6.2) also require firmware that does not exist yet.

## What Changes

- Add one bare-metal nRF5 SDK 17.1.0 bring-up app, `firmware/apps/bringup/`,
  built and flashed exactly like blinky (`make`, `make flash`, `make erase`).
- Drive the guide's safe control states (`DCF_PON` high, `EPD_CS` high,
  `SENSOR_CS` high, `BUZZER_EN` low) as the first action after reset.
- Add small shared drivers under `firmware/src/` that later clock firmware can
  reuse: board pin map, shared SPI bus, Waveshare 2.13" e-paper, LIS3DH,
  DCF-77 pulse capture, and buzzer.
- Select one test at a time with DK buttons 1 to 4; report every step and
  result as text over the DK virtual COM port (UART, read with `picocom`).
- Tests: e-paper full-refresh pattern, LIS3DH `WHO_AM_I` + `INT1` motion
  events, DCF-77 receiver enable and pulse-width logging, short buzzer pulse.
- Add VS Code tasks for bring-up build/flash and a serial-console task.
- No host-side program, no BLE, no web app work in this change.

## Capabilities

### New Capabilities

- `prototype-bringup-firmware`: firmware that puts the breadboard prototype into
  a safe state at reset and runs observable, operator-selected tests for each
  peripheral in the breadboard guide, with results reported over UART.

### Modified Capabilities

None. The guide's requirements already describe the tests; this change supplies
the firmware they reference. Updating guide text to name the new commands is a
follow-up.

## Impact

- New: `firmware/apps/bringup/` (Makefile, `armgcc/`, `config/sdk_config.h`,
  `src/main.c`, README).
- New: `firmware/src/` drivers and `firmware/boards/` DK pin map header.
- Changed: `.vscode/tasks.json` (new tasks), `firmware/README.md` (layout now
  fixed on nRF5 SDK).
- Uses only installed tooling: `arm-none-eabi-gcc`, `NRF5_SDK_DIR`, OpenOCD,
  `picocom`. No new Nix packages. SoftDevice-free.
