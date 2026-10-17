## Why

The clock app (`firmware/apps/clock`) keeps civil time on LFXO + RTC1 and draws it on the e-paper, but the roadmap still lacks the evidence: a photo of the upright panel and a 24 h drift record. Chapter 5 needs the drift number. Stage of `openspec/roadmap.md`. Needs: stage 5 (runs on the deep-sleep build). Unblocks: stage 12.

## What Changes

- Add a host-side logger that stamps every UART line with host time and writes CSV under `notes/measurements/` (instrument, temperature, supply and firmware commit recorded beside it).
- Run the clock for 24 h on the DK after setting it from the host and compare the last line with a reference clock.
- Take the panel photo (light theme rules do not apply to photos; name `YYYY-MM-DD_topic_detail.jpg`) into `notes/media/`.
- Write the dated log note with the drift in ppm and what it says about the 32.768 kHz crystal.

## Capabilities

### New Capabilities
- `rtc-clock`: LFXO-based time, date and weekday state, e-paper display, and the evidence format for drift.

### Modified Capabilities
<!-- none -->

## Impact

- `firmware/apps/clock/`, a logger under `firmware/tools/`, `notes/measurements/`, `notes/media/`, `notes/log/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.1 of the archived `plan-thesis-roadmap` change.
