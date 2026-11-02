# Design

## Context

Items to show exist: `hardware/docs/hardware-block-diagram.pdf`, `firmware/docs/software-flowchart.{puml,svg,pdf}`, and the newer layered set in `firmware/docs/diagrams/` (L1 overview, L2 modules, L3 detail a–f, BLE overview, BLE sequence; `README.md` records the source commit and spec/code mismatches). Prototype state: nRF52 DK running the clock app (`firmware/apps/clock/`) with e-paper, LIS3DH, DCF generator, buzzer, BLE schedule service; Web Bluetooth page in `web/`. Latest measurement note: `notes/log/2026-11-02_refresh-on-demand.md`. Open requirements live in `notes/deadlines.md` (assumed dates) and `notes/promotor-questions.md`. See proposal.md - Why.

## Goals / Non-Goals

**Goals:**
- One short pack the author can show on one screen plus the DK, with every diagram branch labeled implemented or not.
- Feedback captured the same day in a dated note, with changed requirements copied to their home files.

**Non-Goals:**
- No firmware, hardware, web, or LaTeX edits. Gaps found are listed, not fixed.
- No thesis prose; no draft of chapters.
- No new tests with people (ethics rule unchanged).

## Decisions

- **Flowchart check against code, not against specs.** `software-flowchart.puml` is compared with `firmware/apps/clock/src/main.c` and `firmware/src/` at the current HEAD. Output: a table in the consultation note (branch, implemented / partial / not implemented, source file). Alternative (compare to the layered L3 diagrams) rejected: those were derived from the same code and would hide drift in the older chart. If the old flowchart is materially stale, point the promotor at the L1/L3 set and say so; do not redraw here.
- **Mark unimplemented branches in the note, not in the PDF.** Keeps the promotor's copy unchanged and avoids a re-render. Alternative (annotate the `.puml`) rejected: the diagram is a fixed artifact until stage 10.
- **Demo script fixed in advance.** Boot with refresh, set time, set one alarm via the web page, trigger alarm, dismiss by motion, show a DCF sync attempt. Anything not working on the day is stated as such; no faking or pre-recording.
- **Pack = files by path, no new bundle.** A checklist of paths in the note. Datasheets are cited from `clock-project-latex/bibliography.bib`; no copies.
- **Feedback note:** `notes/log/YYYY-MM-DD_first-consultation.md`, sections: attendees/date, shown, feedback (verbatim where possible, Polish kept), decisions, action items. Changed requirements are copied to `notes/deadlines.md`, `notes/promotor-questions.md`, or `openspec/roadmap.md` and linked from the note.
- **Language:** note in English with promotor quotes in original language, matching other logs.

## Risks / Trade-offs

- [DK or a peripheral fails on the day] → run the demo end to end once beforehand; carry the latest PPK2 note and diagrams as fallback evidence.
- [Flowchart much older than code] → the code-check table makes it explicit; the L1/L3 set is the fallback.
- [Feedback only remembered later] → write the note during or right after the meeting; commit it immediately.
- [Promotor changes scope (hardware minimum, chapter order)] → copy to the roadmap in the same session and mention affected open changes.
