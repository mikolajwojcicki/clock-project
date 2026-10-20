# Tasks

## 1. Event counter

- [x] 1.1 Add `firmware/src/motion_count.{c,h}` with `MOTION_N 20`, `MOTION_WINDOW_MS 30000`, `MOTION_GAP_MS 300`, `motion_count_reset()` and `motion_count_event(now_ms)` (true when the Nth counted event lands inside the window); verify it builds in `firmware/apps/clock` with `make`
- [x] 1.2 Add `motion_count.c` to `firmware/apps/clock/armgcc/Makefile`; verify `make` links it

## 2. Alarm stop path

- [x] 2.1 Add `ALARM_STOP_MOTION` and `alarm_stop_motion()` to `firmware/src/alarm.{c,h}`, same effect as the button path; verify `make` builds and bench task 4.2 prints `ALARM stop (motion)`
- [x] 2.2 Confirm the button path still stops the alarm with zero events; verify with bench task 4.3

## 3. INT1 wake in the clock app

- [x] 3.1 In `firmware/apps/clock/src/main.c`, enable `lis3dh_motion_int1_enable()` and GPIOTE on `P0.23` when an alarm starts, and disarm plus `lis3dh_power_down()` plus `motion_count_reset()` when it stops; verify `make` in `firmware/apps/clock` builds
- [x] 3.2 Handler sets a flag and timestamp only; main loop reads `lis3dh_int1_source()`, feeds `motion_count_event`, calls `alarm_stop_motion()` on true; verify the UART prints each counted event and `ALARM stop (motion)`
- [x] 3.3 Document the behaviour and constants in `firmware/README.md`; verify the text matches the `#define`s in `motion_count.h`

## 4. Bench verification (self-test, no volunteers)

- [ ] 4.1 Run with the DK on UART and the Saleae on `P0.23` (INT1) and `BUZZER_EN`: alarm sounding, board still for 30 s; verify no counted events and the buzzer keeps its 1 s / 1 s pattern. If the buzzer triggers INT1, raise the driver threshold and repeat
- [ ] 4.2 Shake until the alarm stops; verify `ALARM stop (motion)` appears, `BUZZER_EN` goes quiet, and the capture shows 20 counted events inside 30 s
- [ ] 4.3 Trigger an alarm and press a button with no motion; verify `ALARM stop (button)`
- [ ] 4.4 Shake the board with no alarm sounding; verify INT1 stays low in the capture and no wake is logged
- [ ] 4.5 Write the dated note in `notes/log/` and save the capture and UART log in `notes/measurements/` with instrument, supply and firmware commit; verify the note shows still, shake-to-silence, button, and no-alarm cases
