## 1. Alarm module (host-testable)

- [x] 1.1 Add `firmware/src/alarm.h` and `alarm.c`: table of 7 slots, `alarm_set`/`alarm_get`, `alarm_parse` (`alarm <1-7> HH:MM on|off`, `alarm?`), `alarm_on_minute`, `alarm_step` (1 s on/off, 60 s timeout, once per minute, restart on new match, silence on button)

## 2. Firmware integration

- [x] 2.1 Add `board_any_button_pressed()` to `board.[ch]` (DK_BUTTON1–4, active low, pull-up configured in `board_safe_state` or a button init)
- [x] 2.2 Wire `alarm.c` into `firmware/apps/clock/armgcc/Makefile` and `main.c`: console `alarm` commands, minute hook in `poll_clock`, `alarm_step` in `app_poll`, buzzer calls, `ALARM` and stop log lines
- [x] 2.3 Build with `make -C firmware/apps/clock` without warnings

## 3. Verify on the DK

- [x] 3.1 Flash, set the clock, set an alarm 2 minutes ahead over UART, and see the `ALARM` line at the minute change
- [x] 3.2 Capture BUZZER_EN with the Saleae analyzer: 1 s on/off and a stop at 60 s with no button; record the capture
- [x] 3.3 Press a button during the alarm (author, on request) and see it stop in 100 ms; press with no alarm and see nothing
- [x] 3.4 Reset during an alarm and see the buzzer off
- [x] 3.5 Add a dated note in `notes/log/` with the result and the firmware commit; author takes the upright-clock photo into `notes/media/` as `YYYY-MM-DD_clock_epaper-upright.jpg`
