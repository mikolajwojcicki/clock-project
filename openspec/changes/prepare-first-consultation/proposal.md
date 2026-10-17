## Why

The promotor requires a working prototype, a hardware block diagram and an algorithm flowchart at consultations, and feedback must be written down. The drawings exist but nobody has shown them yet, so tasks 3.2 and 6.1 are open. Stage of `openspec/roadmap.md`. Needs: nothing. Unblocks: stage 11.

## What Changes

- Collect the consultation pack: DK running the clock app, `hardware/docs/hardware-block-diagram.pdf`, `firmware/docs/software-flowchart.pdf`, the datasheets used (`clock-project-latex/bibliography.bib`).
- Check the flowchart against the firmware that exists at that date and mark which branches are not implemented yet.
- Hold the consultation. Record the date, what was shown and the promotor's feedback in a dated note under `notes/log/`.
- Copy any changed requirement (deadlines, hardware minimum, chapter layout) into `notes/deadlines.md`, `notes/promotor-questions.md` or the roadmap.

## Capabilities

No new or modified capabilities. None. Uses `thesis-document-structure` (mandatory diagrams) and `thesis-evidence-log` from the archived roadmap change.

## Impact

- `notes/log/`, `notes/promotor-questions.md`, `openspec/roadmap.md`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap tasks 3.2 and 6.1 of the archived `plan-thesis-roadmap` change.
