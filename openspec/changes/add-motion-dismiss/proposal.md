## Why

Dismissing the alarm must need physical activity detected by the accelerometer, without polling while the MCU sleeps. This is in the APD scope. Stage of `openspec/roadmap.md`. Needs: stage 3. Unblocks: stages 5, 10.

## What Changes

- Configure LIS3DH INT1 (activity/high-G threshold) and wake the MCU on the GPIOTE edge; INT2 stays reserved.
- Count activity events while the alarm sounds; silence after N events (N configurable). Buttons keep working as the emergency silence.
- Verify with LED/UART log that the MCU wakes only on motion and not while the board is still. Instrument or self-test only; no volunteers.

## Capabilities

### New Capabilities
- `motion-dismiss`: activity-count dismissal, INT1 configuration and wake behaviour.

### Modified Capabilities
<!-- none -->

## Impact

- Firmware app, `notes/log/`, `notes/measurements/` if a shake count log is kept.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.3 of the archived `plan-thesis-roadmap` change.
