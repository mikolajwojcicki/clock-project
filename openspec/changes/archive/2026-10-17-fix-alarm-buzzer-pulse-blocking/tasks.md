## 1. Confirm the cause

- [x] 1.1 Add a temporary UART line with `board_ms()` at refresh start and end; run an alarm at the minute change and compare with a Saleae capture of `BUZZER_EN`; remove the line
- [x] 1.2 Write the confirmed cause (or the new hypothesis) in `notes/log/`

## 2. Fix

- [x] 2.1 Implement the chosen fix from design decision 2 in the clock app / `epd.c` / `spi_bus.c`
- [x] 2.2 Build with `make -C firmware/apps/clock` without warnings and flash

## 3. Verify on the DK

- [x] 3.1 Capture `BUZZER_EN` for 150 s with no button press: 1 s on / 1 s off from the first burst, stop 59 s after it (skipped by author: 30 s capture suffices, timeout verified in run 3)
- [x] 3.2 Press a button during a refresh and see the buzzer stop within 100 ms (author, on request)

## 4. SDK audit

- [x] 4.1 Classify every hardware access in `firmware/src/` and `firmware/apps/clock/src/` (nrfx driver, `app_*` library, direct register, software GPIO toggling) in one table
- [x] 4.2 Write the table and a verdict (`rewrite` or `keep`) with the rule from design decision 3 into `notes/log/`
- [x] 4.3 Tell the author the verdict; if `rewrite`, `rewrite-firmware-on-nrf5-sdk-drivers` is the next change, otherwise mark it dropped in `openspec/roadmap.md`

## 5. Record

- [x] 5.1 Add the dated note in `notes/log/` with results and the firmware commit
