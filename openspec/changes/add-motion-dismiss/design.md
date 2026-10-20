# Design

## Context

See `proposal.md` for motivation and `specs/motion-dismiss/spec.md` for behaviour.

Existing pieces:
- `firmware/src/lis3dh.c` already has `lis3dh_motion_int1_enable()` (100 Hz, ±2 g, high-pass, about 256 mg, latched), `lis3dh_int1_source()` (read releases the latch) and `lis3dh_power_down()`. The bring-up app proves INT1 on `P0.23`.
- `firmware/src/alarm.c` is a pure state machine: `alarm_step(now_ms, button_pressed)` and `alarm_take_stop()` with reasons `NONE`, `BUTTON`, `TIMEOUT`. Timeout is 60 s.
- `apps/clock/src/main.c` calls `poll_alarm()` and switches the buzzer on `alarm_step` result.

## Goals / Non-Goals

**Goals:**
- Add a motion stop reason to the alarm path with no change to the 1 s on / 1 s off pattern.
- Keep the event counter free of hardware so it can be checked on the host.

**Non-Goals:**
- Runtime or BLE setting of N and T (belongs to the BLE schedule change).
- INT2, free-fall detection, motion detection while no alarm sounds.
- Deep-sleep entry itself (separate change `add-deep-sleep-and-current-measurement`). This change only makes INT1 a wake source.

## Decisions

**1. Event counter is a separate module with no hardware access (`motion_count`), alarm stays unchanged in structure.**
`motion_count_event(now_ms)` returns true when the 20th counted event lands inside the 30 s window. It keeps a ring of the last N timestamps and a last-counted timestamp for the 300 ms gap. `main.c` feeds it from the INT1 handler flag and, on true, tells the alarm to stop.
Alternative: put counting inside `alarm.c`. Rejected: mixes hardware events into the schedule/pattern logic and makes the host check harder.

**2. Sliding window by timestamp ring, not by idle reset.**
A ring of N = 20 `uint32_t` timestamps. Event is the Nth when `now - ring[oldest] <= T`. Memory 80 bytes, no timer. The old idea "reset after 5 s idle" is dropped; the window gives it for free.
Alternative: fixed tumbling window. Rejected: an event pair split across the boundary would be lost.

**3. 300 ms gap is applied in software on the INT1 handler timestamp.**
The sensor latches INT1, so several shake pulses merge into one wake anyway. Gap check covers re-arm after `lis3dh_int1_source()` read.
Alternative: use the LIS3DH `INT1_DURATION` register. Rejected: it filters length of a motion, not spacing between events, and adds driver surface.

**4. INT1 on GPIOTE, enabled only during an alarm.**
`alarm_active()` rising edge: `lis3dh_motion_int1_enable()` then arm GPIOTE `P0.23` (high). Falling edge: disarm, `lis3dh_power_down()`, clear counter. The sensor is on SPI (`spi_bus`); the handler only sets a volatile flag and records `board_ms()`. The `INT1_SRC` read that releases the latch happens in the main loop.
Alternative: leave sensor armed always. Rejected: costs sleep current and gives no benefit outside an alarm (see spec).

**5. Stop reason `ALARM_STOP_MOTION`.**
New enum value and `alarm_stop_motion()` entry point that behaves like the button path (sets `s_active = false`, `s_stop = ALARM_STOP_MOTION`). `main.c` log line prints `motion`. The button path is untouched, so the emergency silence keeps working with zero events.

**6. N = 20, T = 30 s, gap = 300 ms as `#define`s in `motion_count.h`.**
T (30 s) is below `ALARM_TIMEOUT_MS` (60 s), so the window can fill twice in one alarm.

**7. Verification.**
No host test (no host gcc in the agent environment). Bench only: DK on UART plus Saleae on INT1 (`P0.23`) and `BUZZER_EN`, self-shaken: still, shake-to-silence, button-with-no-motion. Notes in `notes/log/`, capture in `notes/measurements/`, per the spec.

## Risks / Trade-offs

- [256 mg threshold fires on buzzer vibration of the board] → check "still during alarm" scenario with the buzzer on; raise threshold in the driver if it self-triggers.
- [Handler timestamp jitter makes the 300 ms gap fuzzy] → acceptable, gap only needs order-of-magnitude accuracy.
- [20 shakes in 30 s may be too hard or too easy] → constants are in one header; thesis records the chosen value and why.
- [Sensor power-up delay at alarm start] → LIS3DH turn-on is about 10 ms at 100 Hz, far below one shake interval.
