## Why

Chapter 5 (uruchomienie i testy) reports measurements and must cite the CSV files. Stage of `openspec/roadmap.md`. Needs: stages 5, 6, 7. Unblocks: stage 13.

## What Changes

- Draft chapter 5 from `notes/measurements/` and `hardware/docs/energy-budget.md`.
- Tables and charts cite their CSV files; conditions (instrument, temperature, supply, firmware commit) appear with each result.
- Author rewrites; log in `notes/ai-use.md`.

## Capabilities

No new or modified capabilities. None. Uses `thesis-document-structure` and `thesis-evidence-log`.

## Impact

- `clock-project-latex/chapters/`, `notes/ai-use.md`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 6.3 of the archived `plan-thesis-roadmap` change.
