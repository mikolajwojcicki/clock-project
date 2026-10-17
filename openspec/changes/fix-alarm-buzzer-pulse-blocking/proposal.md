## Why

The Saleae captures of `add-alarm-schedule-and-buzzer` (2026-10-17) show the first 4 to 8 s of every alarm as one solid tone instead of 1 s on / 1 s off pulses. From about 75.4 s on, the pattern is correct and the alarm stops 59 s after the start. The alarm starts on the minute change, which is also when the e-paper refreshes. Cause is not yet confirmed: the leading hypothesis is that the refresh blocks `app_poll()` so `alarm_step()` is not called and the buzzer stays on.

The same capture work raised a second question: is the firmware written on top of the nRF5 SDK drivers, or does it reach for registers and spin loops by hand? `firmware/src/spi_bus.c`, `buzzer.c` and `board.c` touch `NRF_SPIM0`, `NRF_PWM0` and `NRF_RTC1` directly. This change records the answer; the answer decides whether `rewrite-firmware-on-nrf5-sdk-drivers` runs.

## What Changes

- Confirm the cause of the solid tone with a capture and by reading the refresh path in `epd.c`, `spi_bus.c` and `apps/clock/src/main.c`.
- Fix it so the buzzer pattern is 1 s on / 1 s off from the first second, also while the display refreshes.
- Re-capture `BUZZER_EN` with no button press and record the result.
- Audit firmware for nRF5 SDK use versus hand-written register access and software pin toggling; write the finding to `notes/log/` and set the go / no-go for `rewrite-firmware-on-nrf5-sdk-drivers`.

## Capabilities

### New Capabilities
- `alarm-buzzer-timing`: the buzzer on/off pattern holds its timing while the display refreshes.

### Modified Capabilities
<!-- none: alarm-schedule is still inside the unarchived add-alarm-schedule-and-buzzer change -->

## Impact

- `firmware/apps/clock/src/main.c`, possibly `firmware/src/epd.c`, `firmware/src/spi_bus.c`, `firmware/src/alarm.c`.
- `notes/log/` (capture result, SDK audit).
- Needs: `add-alarm-schedule-and-buzzer` committed (done, `c4ee7b9`). Unblocks the go / no-go of `rewrite-firmware-on-nrf5-sdk-drivers`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0).
