## Why

Time must be corrected by periodic DCF-77 sync. The decode path and a Nucleo generator exist from bring-up, but real DCF-77 gave no valid pulses in the room, so the generator is the proof path. Stage of `openspec/roadmap.md`. Needs: stage 5. Unblocks: stages 7, 11.

## What Changes

- Timed daily sync window: power the DCF-1060N, decode a minute frame, correct RTC1, power it off (PON).
- Reject frames with bad parity or markers and keep the RTC unchanged; log the reason.
- Drive a sync from the Nucleo generator (`firmware/tools/dcf77-generator`) and log the result; measure the window current as part of stage 5 data.

## Capabilities

### New Capabilities
- `dcf77-time-sync`: sync window scheduling, frame acceptance, RTC correction, receiver power gating.

### Modified Capabilities
<!-- none -->

## Impact

- Firmware app, `firmware/tools/dcf77-generator/`, `notes/log/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.5 of the archived `plan-thesis-roadmap` change.
