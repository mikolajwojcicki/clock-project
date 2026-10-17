## Context

`firmware/apps/clock` is a polling loop: `app_poll()` reads UART and advances civil time from `board_ms()`. `firmware/src/buzzer.c` already provides `buzzer_tone_start/off` and `buzzer_steady_on`. Weekdays in `clock_time.wday` are 0 = Monday. The DK buttons sit on P0.13–16 (`dk_breadboard_pins.h`). Deep sleep comes later (stage 5), so this change keeps the polling loop.

## Goals / Non-Goals

**Goals:** per-weekday table, UART setup, trigger on minute change, 1 s/1 s buzzer with 60 s timeout, button silence, host-testable logic.

**Non-Goals:** persistence (flash), e-paper alarm UI, motion dismiss (stage 4), BLE (stage 8), deep sleep (stage 5), final-board button mapping (stage 10).

## Decisions

1. **New `firmware/src/alarm.[ch]`, no hardware access.** Holds the table, parses nothing, and exposes `alarm_set`, `alarm_get`, `alarm_on_minute(const clock_time*)` and `alarm_step(now_ms, button_pressed)` returning the wanted buzzer state. Reason: the trigger rule, once-per-minute rule and timeouts are testable on the host like `clock_test.c`, with no DK.
2. **Buzzer driven by the app, not by alarm.c.** `main.c` calls `buzzer_tone_start/off` from the returned state. The tone (2.7 kHz) is the AP-1205V-P1 drive from the bring-up change; 1 s on/off gating is done by `alarm_step`.
3. **Minute hook in `poll_clock`.** When the minute changes (or the clock is set), call `alarm_on_minute`. Polling loop stays; stage 5 replaces it with an RTC event.
4. **Weekday mapping.** Console 1–7 is ISO (1 = Monday); internal index is `n - 1`, equal to `clock_time.wday`.
5. **One button function.** `board_any_button_pressed()` in `board.[ch]` returns true if any of DK_BUTTON1–4 is low (active low, internal pull-up). Stage 10 re-implements it for the perfboard pins. No event layer.
6. **Once per minute.** `alarm.c` remembers the last (wday, hh, mm) it fired for; the same minute cannot restart it after a silence. Setting the clock to the same minute again does restart (a different set event), and a new match during sound restarts the 60 s timer.
7. **Console parsing** lives in `main.c` next to `accept_line`; a line starting with `alarm` goes to a small `alarm_parse` in `alarm.c` (pure function, host-tested) that returns ok/usage.

## Risks / Trade-offs

- Polling loop blocks during e-paper refresh (a few seconds), so a button press or a 1 s toggle can be late. `board_wait_ms` already calls `app_poll`, so `app_poll` also calls `alarm_step`; accept ±100 ms except during refresh. The logic-analyzer check is done while the display is idle.
- RAM-only table: every reset needs the commands re-sent. Accepted until stage 8 or a later persistence change.
- Buzzer draws ~50 mA while sounding (locked decision); no mitigation here.

## Migration Plan

None. New code; `firmware/apps/clock/armgcc/Makefile` gains `alarm.c`. Roll back by reverting the commits.
