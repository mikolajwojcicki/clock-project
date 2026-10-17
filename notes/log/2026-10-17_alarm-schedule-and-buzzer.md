# 2026-10-17 — alarm schedule, buzzer pattern, button silence

OpenSpec change `add-alarm-schedule-and-buzzer`: alarm works on the DK. Defect found: solid first tone. Photo pending (author).

- **Done:** Per-weekday table (7 slots, RAM, all disabled at reset) in `firmware/src/alarm.[ch]`. UART commands `alarm <1-7> HH:MM on|off` and `alarm?` (1 = Monday). Alarm checked on each minute change; fires once per minute. Buzzer 1 s on / 1 s off, 2.7 kHz tone on `BUZZER_EN`, 60 s timeout. Any DK button (P0.13–16, pull-up, active low) silences it.
- **Evidence:**
  - Author set `alarm 6 12:02 on` at `2026-10-17 12:00:00`: alarm sounded, a button stopped it.
  - Saleae, `BUZZER_EN` on channel 0, run 3, 150 s, no button: 26 bursts of 1.000 s on / 1.000 s off, last burst ends 59.0 s after the first. Stop at timeout confirmed.
  - Author confirmed: a button press with no alarm does nothing; reset during an alarm silences the buzzer.
- **Failed / open:**
  - First 4–8 s of every alarm is one solid tone, not 1 s pulses (3.9 s and 5 s in run 2, 7.8 s in run 3). It coincides with the e-paper refresh at the minute change. Likely cause: refresh blocks `app_poll()`, so `alarm_step()` does not run. Not confirmed. Tracked in `fix-alarm-buzzer-pulse-blocking`.
  - The 100 ms stop time was not measured with the analyzer; author saw the stop work. The analyzer also cannot show it during a refresh (same defect).
  - Host test `alarm_test.c` was dropped on the author's request (no host `gcc`); trigger logic is verified on the DK only.
  - UART logging to a file from the host failed (`/tmp/uart.txt` empty), so the `ALARM` lines were seen by the author, not logged.
- **Why:** Roadmap stage 3; the product needs an alarm before motion dismiss (stage 4) and deep sleep (stage 5).
- **Evidence refs:** commits `82f0527` (alarm module), `1554014` (button input and clock app wiring), `c4ee7b9` (tasks ticked). Capture CSVs were in `/tmp` and are not saved.
- **Photo:** author adds `notes/media/YYYY-MM-DD_clock_epaper-upright.jpg` (ribbon at bottom, weekday on top).
