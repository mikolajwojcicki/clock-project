## Why

Chapters 3 (hardware) and 4 (software) are written first. The agent drafts from `notes/`; the author rewrites. Stage of `openspec/roadmap.md`. Needs: stage 10 and stages 3 to 6. Unblocks: stage 13.

## What Changes

- Draft chapters 3 and 4 in `clock-project-latex/chapters/` from the log notes, schematics, diagrams and code; follow `thesis-writing-rules.yaml`.
- Reference every figure and listing in the text; add datasheet and source citations to the bibliography.
- Author rewrites each paragraph in own words; unrewritten drafts are removed before the PDF is submitted.
- Log every kept draft in `notes/ai-use.md`.

## Capabilities

No new or modified capabilities. None. Uses `thesis-document-structure`.

## Impact

- `clock-project-latex/` (separate repo; bump the submodule pointer), `notes/ai-use.md`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 6.2 of the archived `plan-thesis-roadmap` change.
