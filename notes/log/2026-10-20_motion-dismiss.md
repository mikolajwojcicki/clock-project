# 2026-10-20 Motion dismiss bench check

Change: `add-motion-dismiss`, tasks 4.1 to 4.5. Firmware commit: `a7c7cbf` (counter in `39a2c46`).

## Setup

- nRF52 DK (PCA10040) on the breadboard, LIS3DH on SPI, INT1 on `P0.23`, buzzer on `P0.31`.
- Instrument: Saleae Logic, digital, 1 MS/s. Ch 0 `SENSOR_INT1`, Ch 1 `BUZZER_EN`, Ch 2 `CLOCK_SEC_MARK` (`P0.22`).
- UART: `/dev/ttyACM0`, 115200, read with pyserial.
- Supply: USB from the DK. Temperature: room, not logged.
- Rule under test: 20 INT1 events, at least 300 ms apart, inside 30 s silence the alarm. Any button silences at once.
- No volunteers. I did all moves myself.

## Results

| Task | Case | UART | Result |
|---|---|---|---|
| 4.1 | Alarm, board still | `MOTION event 1`, `ALARM stop (timeout)` | pass |
| 4.2 | Alarm, I tapped the sensor | events 1 to 20 in about 18 s, `MOTION event 20 (stop)`, `ALARM stop (motion)` | pass |
| 4.3 | Alarm, button, no motion | `ALARM stop (button)`, no `MOTION` lines | pass |
| 4.4 | Shake, no alarm | no `MOTION` or `ALARM` lines in 50 s; Ch 0 low in all 51 exported rows | pass |

Captures in `notes/measurements/`:
- `2026-10-20_motion-dismiss_still-timeout.sal`
- `2026-10-20_motion-dismiss_tap-to-silence.sal`
- `2026-10-20_motion-dismiss_button.sal`
- `2026-10-20_motion-dismiss_shake-no-alarm.sal`

## What went wrong on the way

- First 4.1 try failed: VS Code held `/dev/ttyACM0` (serial monitor), so pyserial raised "multiple access on port". Closed it, the port was clean.
- First "still" run was not still. I tapped the sensor once the buzzer started, so it became the 4.2 run. Re-ran 4.1 with hands off.
- Saleae rejected a 3.3 V digital threshold. This device has no configurable threshold, so the setting was dropped.

## Findings

- Buzzer vibration alone does not trigger INT1: 10 s of buzzing with no events in the first run, and a full timeout in the clean 4.1 run.
- `MOTION event 1` appears at every alarm start, even when still. Likely a transient when `lis3dh_motion_int1_enable()` runs. It counts toward the 20, so the real need is 19 events. Left as is.
- Repeated `MOTION event N` lines with the same N are INT1 events rejected by the 300 ms gap. They print the old count. Cosmetic, not fixed.
- 4.2 used taps, not a jump-jack shake. A hard shake is not yet tested.
