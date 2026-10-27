# Tasks

## 1. Firmware

- [x] 1.1 Release `EPD_RST` after every refresh in `firmware/src/epd.c` (`07dd2de`). Verify: `make` and `make SLEEP_UART=0` in `firmware/apps/clock/armgcc` build.
- [x] 1.2 In `firmware/src/spi_bus.c` drive `SCK` and `MOSI` high in `spi_bus_init` and again after `nrfx_spim_uninit` in `spi_bus_transfer`; update the comments there. Verify: `make` and `make SLEEP_UART=0` in `firmware/apps/clock/armgcc` build, and `make` in both bringup apps builds. Note 2026-10-27: both bringup apps failed to link on `nrfx_gpiote_in_*` from `epd.c`, a break from `a0ad997`. Fixed in `f1d4d5d` (bringup apps ported to nrfx_gpiote); all four builds now pass.
- [x] 1.3 Commit 1.2 on its own (`firmware:` prefix), body citing C4.

## 2. Bench check (needs the external SWD cable)

- [x] 2.1 Flash the `SLEEP_UART=0` clock build with OpenOCD through the external probe. Verify: the panel shows the time and changes at the next minute. Done 2026-10-27 with `a762be5`: IDC ribbon P19 to P18, PPK2 powering `VDD_nRF`; DCF generator synced the clock, panel redrew each minute.
- [x] 2.2 With a UART build, check the LIS3DH: the clock starts with no `WHO_AM_I` failure, and an alarm is silenced by motion. Verify: UART text saved in the C6 log entry. Done 2026-10-27: the clock app has no `WHO_AM_I` read, so SPI is proven by the motion dismiss (20 events, `ALARM stop (motion)`), logged under C6.
- [ ] 2.3 Run C6: one 3.0 V PPK2 rail, display, DCF receiver and LIS3DH powered and wired, `D0` on `P0.22`, at least 60 s idle between minute refreshes after the DCF window. Pack it with `firmware/tools/ppk-plot/ppk-archive.sh pack` (under 50 MB, use `--from/--to` if needed). Verify: the pack command reports `verified`.
- [ ] 2.4 Add a C6 section to `notes/log/2026-10-24_deep-sleep-current.md`: setup, idle mean, the standby terms (A8 nRF figure, DCF receiver in power-down, panel deep sleep, LIS3DH power-down, breakout LED and regulator) and the remainder. If the remainder is over 10 µA, unplug `P0.03`, `P0.04`, `P0.28` one at a time and log the steps. Verify: the remainder is within 10 µA, or the open leak is named in the log.
- [ ] 2.5 Commit the capture and the log entry (`thesis:` prefix).

## 3. Close-out

- [ ] 3.1 In `openspec/changes/add-deep-sleep-and-current-measurement/tasks.md` add a note under 4.3 that C idle figures before C6 include the pin leaks and that runs after C6 use the fixed firmware. Verify: `openspec validate fix-idle-pin-leaks --strict` and `openspec validate add-deep-sleep-and-current-measurement --strict` pass.

## Workflow follow-up

- Archive this change after C6 passes, before archiving `add-deep-sleep-and-current-measurement`.
