# Tasks

## 1. Pack check

- [x] 1.1 Run `git diff --stat 8c415fe HEAD -- firmware/apps/clock firmware/src firmware/boards web`; verify it is empty, or that every change is reflected in `firmware/docs/diagrams/` (fixes go to `add-firmware-architecture-diagrams`, then re-render with `render.sh`).
- [x] 1.2 Confirm the pack files open: `hardware/docs/hardware-block-diagram.pdf` and every `firmware/docs/diagrams/*.pdf` (`pdfinfo` returns a page count for each); `software-flowchart.*` is not in the pack.
- [x] 1.3 List not-implemented and partial behavior (from `firmware/docs/diagrams/README.md` mismatch table, unused SAADC, no System OFF, no supply switch for LIS3DH and e-Paper) in `notes/log/YYYY-MM-DD_first-consultation.md`; verify each row names a source file.

## 2. Demo rehearsal

- [ ] 2.1 Flash the current clock app to the DK and run the script: boot refresh, set time, set one alarm from the web page, alarm sounds, motion dismiss, DCF sync attempt; verify each step's result (pass/fail and observed behavior) is written in the note.
- [ ] 2.2 Check `hardware/docs/hardware-block-diagram.pdf` against the board actually on the desk (parts and connections); verify mismatches are listed in the note.

## 3. Consultation

- [ ] 3.1 Hold the consultation and write the note the same day: date, what was shown, feedback (promotor's words kept in Polish), decisions, action items; verify the note exists under `notes/log/` and is committed.
- [ ] 3.2 Copy every changed requirement (deadlines, hardware minimum, chapter layout) into `notes/deadlines.md`, `notes/promotor-questions.md` or `openspec/roadmap.md`, linking back to the note; verify each target file contains the change, or the note says "no changes".
- [ ] 3.3 Tick roadmap stage 1 in `openspec/roadmap.md` and verify it points to the note.

## Workflow follow-up

- Archive this change after 3.3. Archive `add-firmware-architecture-diagrams` once no drift fixes are pending.
