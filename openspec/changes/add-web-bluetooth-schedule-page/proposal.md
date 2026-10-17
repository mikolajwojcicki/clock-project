## Why

`web/` is a scaffold only. The user needs a page that writes the schedule over Web Bluetooth. Stage of `openspec/roadmap.md`. Needs: stage 8.

## What Changes

- Web Bluetooth page in `web/` that connects to the DK, edits the per-weekday schedule and writes it to the GATT service.
- Verify: an upload from the browser changes the alarm time on the DK.

## Capabilities

### New Capabilities
- `web-schedule-upload`: page behaviour, encoding shared with the firmware service.

### Modified Capabilities
<!-- none -->

## Impact

- `web/`, `notes/log/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.8 of the archived `plan-thesis-roadmap` change.
