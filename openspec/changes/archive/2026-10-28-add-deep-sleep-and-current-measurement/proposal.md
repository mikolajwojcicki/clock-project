## Why

The thesis claim is ultra-low power, and the clock app is a polling loop. Sleep currents drive chapter 5, so they are measured before BLE adds a SoftDevice. Stage of `openspec/roadmap.md`. Needs: stages 3, 4. Unblocks: stages 6, 7, 8, 12.

## What Changes

- Replace the polling loop with System ON sleep (WFE) woken by RTC1, GPIOTE and timers, with peripherals power-gated when idle (e-paper powered only for refresh, DCF off).
- Measure current in: deep sleep, e-paper refresh, DCF window, alarm (buzzer on). Record instrument, temperature, supply and firmware commit.
- Store CSV and conditions under `notes/measurements/` and a dated log note.
- Update the software flowchart if the sleep structure differs from it.

## Capabilities

### New Capabilities
- `low-power-sleep-policy`: which wake sources exist, what is powered when, and how current is measured and recorded.

### Modified Capabilities
<!-- none -->

## Impact

- Firmware app, `notes/measurements/`, `notes/log/`, `firmware/docs/software-flowchart.*`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.4 of the archived `plan-thesis-roadmap` change.
