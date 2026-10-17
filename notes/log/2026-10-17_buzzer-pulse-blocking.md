# 2026-10-17 — solid first buzzer tone: cause confirmed and fixed

OpenSpec change `fix-alarm-buzzer-pulse-blocking`.

- **Cause (confirmed):** Temporary UART lines (`TMP draw start/end`, `TMP buzz`) with `board_ms()` plus a Saleae capture of `BUZZER_EN` (ch 0, 1 MHz). The minute-change refresh ran seconds in `epd_refresh()`. `wait_idle()` called `app_poll()`, but the SPI row loop in `write_face()` did not. The buzzer toggles only in `app_poll()` via `alarm_step()`. Result before the fix: burst 1 = 3.889 s, then a 5.0 s burst, then 1.0 s pulses. The temporary lines were removed.
- **Fix:** `app_poll()` after each row in `write_face()` (`firmware/src/epd.c`).
- **Evidence after the fix:** capture 30 s, alarm at 12:02:00 with the refresh: bursts 1.001 / 0.996 / 0.999 / 1.001 / 1.000 s on, 1.0 s off, from 9.66 s. No solid tone.
- **Not measured:** the 150 s run to the 59 s timeout (only 30 s captured; the timeout was verified in the earlier note). Button stop during a refresh: author confirmed it stops. Capture CSVs are in `/tmp`, not saved.
- **Evidence refs:** firmware commit `ab0aebe`.
