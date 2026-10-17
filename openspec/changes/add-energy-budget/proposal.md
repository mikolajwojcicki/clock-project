## Why

Chapter 5 compares a theoretical budget with measurements, and the budget needs the measured states first. Stage of `openspec/roadmap.md`. Needs: stages 5, 6. Unblocks: stage 12.

## What Changes

- Compute average current and battery life for the chosen cell from datasheet values and duty cycles (sleep, per-minute refresh, daily DCF window, alarm).
- Put the table and the calculation in `hardware/docs/energy-budget.md` next to measured values from `notes/measurements/`, with the difference per state.

## Capabilities

### New Capabilities
- `energy-budget`: states, duty cycles, calculation method and measured-versus-theoretical comparison.

### Modified Capabilities
<!-- none -->

## Impact

- `hardware/docs/energy-budget.md`, `notes/measurements/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 4.6 of the archived `plan-thesis-roadmap` change.
