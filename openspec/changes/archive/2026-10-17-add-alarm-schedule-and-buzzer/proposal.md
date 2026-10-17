## Why

The product is an alarm clock; the app has a clock but no alarms. The bring-up firmware proved the buzzer driver and four buttons separately. Stage of `openspec/roadmap.md`. Needs: nothing. Unblocks: stages 4, 5, 10.

## What Changes

- Per-weekday alarm table: unique HH:MM per day, enable flag per day, held in RAM and structured so BLE (stage 8) can fill it later.
- Compare time with the table on every minute change; on a match drive the buzzer through the NPN stage.
- Button silences the alarm (the minimal-buttons requirement). Motion dismiss follows in stage 4.
- Verify on a logic analyzer or UART log that the alarm fires at the set HH:MM; record in `notes/log/`.

## Capabilities

### New Capabilities
- `alarm-schedule`: per-weekday schedule model, trigger rule, buzzer drive and button silence.

### Modified Capabilities
<!-- none -->

## Impact

- `firmware/apps/clock/` (or a new app dir), `notes/log/`, possibly `firmware/docs/software-flowchart.*` if the flow changes.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.2 of the archived `plan-thesis-roadmap` change.
