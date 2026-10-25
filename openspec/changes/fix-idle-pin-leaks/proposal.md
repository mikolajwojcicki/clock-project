# Proposal

## Why

Runs C4 and C5 (2026-10-25, `notes/log/2026-10-24_deep-sleep-current.md`) showed that the whole board idled at about 3.9 mA, against 2 µA for the nRF alone. Two GPIO states caused it: `EPD_RST` held high cost 2.07 mA, and the shared SPI lines held low against the LIS3DH breakout pull-ups cost about 1.8 mA. The current measurement in `add-deep-sleep-and-current-measurement` (tasks 4.1-4.6) cannot give useful figures until both are fixed.

## What Changes

- `EPD_RST` is released (reset-default input, no pull) after every e-paper refresh, on success and on failure. Already in the tree as `07dd2de`, built but not flashed; this change adopts it and adds the bench check.
- `SCK` and `MOSI` are driven high between SPI transfers, at init and after each transfer, instead of low (mode 0 idle) or the last data bit. The user chose driving over releasing (2026-10-25): driven lines never float, so the display inputs stay defined also when the LIS3DH breakout is unplugged.
- A PPK2 run on the full board (run C, LIS3DH fitted) checks that idle drops to the nRF-only level, and the log records it.

## Capabilities

### New Capabilities
- `idle-pin-states`: the level or state of each MCU line to a peripheral while the MCU sleeps, chosen so that no line sources or sinks current through a peripheral.

### Modified Capabilities
<!-- none: low-power-sleep-policy is still an open change and not yet in openspec/specs/ -->

## Impact

- Firmware: `firmware/src/epd.c` (done in `07dd2de`), `firmware/src/spi_bus.c`. Shared by `apps/clock`, `apps/bringup`, `apps/bringup-4button`.
- Notes: a C6 entry in `notes/log/2026-10-24_deep-sleep-current.md` and a packed capture under `notes/measurements/`.
- Blocked on flashing: SWD from the DK interface MCU is cut (SB20/SB21), the external probe cable is expected Thursday.
- No hardware change. The perfboard keeps the Adafruit breakout and its level-shifter pull-ups.
- Unblocks tasks 4.1-4.6 of `add-deep-sleep-and-current-measurement`.
