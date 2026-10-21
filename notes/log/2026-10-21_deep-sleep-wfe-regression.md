# 2026-10-21 WFE sleep loop: clock, alarm and motion-dismiss regression

Change: `add-deep-sleep-and-current-measurement`, tasks 1.1 to 1.4. Firmware commit: `438a29e` (sleep loop). Compare with `notes/log/2026-10-20_motion-dismiss.md` (firmware `a7c7cbf`, busy-poll loop).

## Setup

- nRF52 DK (PCA10040) on the breadboard, LIS3DH on SPI, buzzer on `P0.31`, UART on the DK virtual COM port.
- Instrument: Saleae Logic, digital, 1 MS/s. Ch 0 `CLOCK_SEC_MARK` (`P0.22`), Ch 1 `BUZZER_EN`, Ch 2 `SENSOR_INT1` (`P0.23`).
- Supply: USB from the DK. Temperature: room, not logged.
- Test alarm: Wednesday 12:01, clock set to 12:00:30 over UART, so the alarm starts 30 s later.
- No volunteers. I did all moves myself.

## Results

| Task | Case | Result |
|---|---|---|
| 1.2 | Clock tick | 131 UART lines in 130.00 s, spacing 0.999 to 1.002 s. `CLOCK_SEC_MARK` toggles every 0.9997 to 1.0007 s. |
| 1.3 | Alarm start | Buzzer starts 1.4 ms after the 12:01:00 tick. |
| 1.3 | Button silence | A short tap and a hold each gave `ALARM stop (button)` 4.9 s and 6.3 s after start. |
| 1.3 | Timeout | `ALARM stop (timeout)` exactly 60.00 s after `ALARM 12:01`. Buzzer shows 1 s on / 1 s off. |
| 1.4 | Alarm, board still | No INT1 rising edge in 60 s, no `MOTION` line, timeout. |
| 1.4 | Taps | 20 events, `MOTION event 20 (stop)` and `ALARM stop (motion)` 37.09 s into the run, 7.5 s after alarm start. |
| 1.4 | Button, no motion | `MOTION event 1` at start, `ALARM stop (button)` 4.9 s later, no other `MOTION` line. |
| 1.4 | Shake, no alarm | 45 s UART with no `MOTION` or `ALARM` line. Ch 1 and Ch 2 had 0 edges in 50 s. |

Captures in `notes/measurements/`:
- `2026-10-21_deep-sleep_clock-sec-mark.sal`
- `2026-10-21_deep-sleep_alarm-timeout.sal`
- `2026-10-21_deep-sleep_motion-tap-to-silence.sal`
- `2026-10-21_deep-sleep_motion-button.sal`
- `2026-10-21_deep-sleep_motion-shake-no-alarm.sal`

Same outcomes as the 2026-10-20 note for all four motion cases. `MOTION event 1` at alarm start is the same arming transient seen then.

## What went wrong on the way

- Two button runs showed no stop because the stop line arrived after my UART window had closed. The next run read it as a stale line at 0.00 s. A longer read window fixed it.
- A first timeout run read only 85 s of a 90 s alarm. Re-ran with a 100 s window.
- Saleae again rejected a 3.3 V threshold, so the setting was dropped.
- **Unexplained:** two tap runs (captures taken in the same session, not kept) showed no INT1 rising edge and no `MOTION` line while the buzzer sounded, though the user reported tapping in the second one. I flashed the pre-change firmware `a7c7cbf` to check: it stopped the alarm by motion with the same tapping, so I suspected the sleep loop. I then flashed a build with a temporary `WHO_AM_I` and pin print: `who=0x33`, events counted, alarm stopped by motion, also on a second alarm in the same boot. I removed the diagnostic and reflashed the committed code: first alarm after reset also stopped by motion. I could not reproduce the failure in four later runs, so the cause is not found. Treat INT1 on repeated alarms as watch-listed for task 2.4.

## Findings

- The WFE loop kept clock, alarm, button and motion-dismiss behaviour unchanged against the 2026-10-20 results.
- Button wake works for short taps, so the GPIOTE PORT wake with four low-power slots is enough.
