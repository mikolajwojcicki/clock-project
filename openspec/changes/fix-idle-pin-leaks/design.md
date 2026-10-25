# Design

## Context

See proposal.md for the measured leaks (C4, C5).

- `firmware/src/spi_bus.c` is shared by `apps/clock`, `apps/bringup` and `apps/bringup-4button`. `spi_bus_init` drives `SCK` and `MOSI` low. Each transfer runs `nrfx_spim_init`, the transfer and `nrfx_spim_uninit`. In nRF5 SDK 17.1.0 `nrfx_spim_init` sets `SCK` to the idle level of the mode (low for mode 0, high for mode 3), and `nrfx_spim_uninit` resets only `MISO` to the default state. So after a transfer `SCK` stays an output at its idle level and `MOSI` stays at the last bit sent.
- The Adafruit LIS3DH breakout (product 2809) has pull-ups to its 3.3 V side on `SCL`/`SDA`/`SDO`/`CS` (`R3G$1..4`). A line held low by the nRF sinks current through them. C4: about 0.57 mA per line.
- In C4 the `SDA` and `SDO` wires were swapped by mistake (user, 2026-10-25), so the step logged as `SDO` was the `MOSI` wire. Two driven lines (`SCK`, `MOSI`) explain the result. `MISO` is an nRF input with no pull and needs no change.
- `EPD_RST`: fixed in `07dd2de`. `epd_refresh` wraps the old body and calls `nrf_gpio_cfg_default(EPD_RST)` on every exit path. The panel is in deep sleep mode 1 after a refresh. Only a hardware reset wakes it, and every refresh starts with one.
- Flashing needs the external SWD cable (SB20/SB21 cut on the DK).

## Goals / Non-Goals

**Goals:**
- Remove both pin leaks with firmware only.
- Keep one SPI idle convention for every app that links `spi_bus.c`.

**Non-Goals:**
- No hardware change on the breakout (no pull-up or LED removal).
- No change to the SPI speed, modes or CS handling.
- The four-state current measurement stays in `add-deep-sleep-and-current-measurement`.

## Decisions

1. **`SCK` and `MOSI` driven high between transfers (user choice).** `spi_bus_init` sets both high. After `nrfx_spim_uninit`, `spi_bus_transfer` sets both high again. Alternative: release both to input with no pull. That also removes the leak while the breakout is fitted, but with the breakout removed (as in C5) the lines float at the display inputs. Driven high is never undefined. Cost: a driven line back-feeds an unpowered part. In this design every peripheral is always powered, so this does not happen.
2. **The order around CS stays.** For mode 0, `nrfx_spim_init` pulls `SCK` low while every CS is still high. The falling edge happens with no device selected, so neither device sees it as a clock edge. Mode 3 transfers already idle high.
3. **`EPD_RST` stays released, not driven high.** Driving high is the state that cost 2.07 mA (C5). The controller ignores `RST` while in deep sleep mode 1 until a low pulse arrives. The HAT measured 19 kΩ on `RST` unpowered, so the line idles at a defined level with no MCU drive `[INFERENCE]`. This choice is already committed (`07dd2de`). This change only adds the bench check.
4. **No new API.** The idle state is set inside `spi_bus.c` and `epd.c`. Callers do not change.

## Risks / Trade-offs

- [The HAT input draws current also when `SCK`/`MOSI` sit high, as `EPD_RST` did] → C5 pulled `P0.03`/`P0.04` with display power already off, so this is not measured. The C6 run separates it: if idle stays high, pull `P0.03` and `P0.04` one at a time.
- [`RST` floats and the panel resets by noise] → The panel is in deep sleep and a stray reset only wakes the controller, which costs power but not content. C6 shows it as current steps between minute refreshes.
- [Driving high back-feeds a breakout that has lost power] → Only on the bench. Keep GND and power wired before signal wires, as in the C4/C5 procedure.

## Migration Plan

1. Change `spi_bus.c` and commit. Build `make` and `make SLEEP_UART=0`.
2. Flash with the external probe (OpenOCD), `SLEEP_UART=0` build.
3. Run C6 and log it. Rollback: `git revert` the `spi_bus.c` commit and, if needed, `07dd2de`.
