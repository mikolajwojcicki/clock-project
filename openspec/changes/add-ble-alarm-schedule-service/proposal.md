## Why

Configuration is over BLE only. It comes after the sleep numbers because the SoftDevice adds power and complexity. Stage of `openspec/roadmap.md`. Needs: stage 5. Unblocks: stage 9.

## What Changes

- Add Nordic SoftDevice (nRF5 SDK) and a GATT service that carries the per-weekday schedule; keep the application power policy in project code.
- Advertise only when needed; measure the BLE current alongside stage 5 data.
- Verify: nRF Connect writes a schedule and the firmware echoes it over UART.

## Capabilities

### New Capabilities
- `ble-schedule-service`: GATT layout, schedule encoding, advertising policy.

### Modified Capabilities
<!-- none -->

## Impact

- Firmware app and linker config (SoftDevice), `notes/log/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.7 of the archived `plan-thesis-roadmap` change.
