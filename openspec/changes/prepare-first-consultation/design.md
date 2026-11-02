# Design

## Context

Items to show: `hardware/docs/hardware-block-diagram.pdf` and the layered firmware set in `firmware/docs/diagrams/` (L1 overview, L2 modules a/b, L3 detail a–f, BLE overview a/b, BLE sequence a/b). The set comes from the change `add-firmware-architecture-diagrams` (15/15 tasks, not archived yet) and replaces the older `firmware/docs/software-flowchart.*`, which is not shown. The diagrams README records the source commit (`8c415fe`), fact tables and 16 spec/code mismatches. Prototype state: nRF52 DK running the clock app (`firmware/apps/clock/`) with e-paper, LIS3DH, DCF generator, buzzer, BLE schedule service; Web Bluetooth page in `web/`. Latest measurement note: `notes/log/2026-11-02_refresh-on-demand.md`. Open requirements live in `notes/deadlines.md` (assumed dates) and `notes/promotor-questions.md`. See proposal.md - Why.

## Goals / Non-Goals

**Goals:**
- One short pack the author can show on one screen plus the DK, with every diagram branch labeled implemented or not.
- Feedback captured the same day in a dated note, with changed requirements copied to their home files.

**Non-Goals:**
- No firmware, hardware, web, or LaTeX edits. Gaps found are listed, not fixed.
- No thesis prose; no draft of chapters.
- No new tests with people (ethics rule unchanged).

## Decisions

- **Show the layered diagrams, not the old flowchart.** User decision 2026-11-02. They are drawn as built from the code, so the "check against firmware" step is a drift check: `git diff 8c415fe HEAD -- firmware/apps/clock firmware/src firmware/boards web` must be empty (it was empty on 2026-11-02), otherwise re-read the changed parts and fix the diagrams in `add-firmware-architecture-diagrams` (still open for that purpose), not here. The old `software-flowchart.*` is left unchanged and is not part of the pack.
- **Mark what is not implemented in the note, not in the PDFs.** Taken from the diagrams README (spec mismatches, "SAADC not used", System OFF unused, no supply switch for LIS3DH and e-Paper) plus anything the drift check finds. Keeps the PDFs unchanged and avoids a re-render. Alternative (annotate the `.puml`) rejected: those files belong to the other change.
- **Demo script fixed in advance.** Boot with refresh, set time, set one alarm via the web page, trigger alarm, dismiss by motion, show a DCF sync attempt. Anything not working on the day is stated as such; no faking or pre-recording.
- **Pack = files by path, no new bundle.** A checklist of paths in the note. Datasheets are cited from `clock-project-latex/bibliography.bib`; no copies.
- **Feedback note:** `notes/log/YYYY-MM-DD_first-consultation.md`, sections: attendees/date, shown, feedback (verbatim where possible, Polish kept), decisions, action items. Changed requirements are copied to `notes/deadlines.md`, `notes/promotor-questions.md`, or `openspec/roadmap.md` and linked from the note.
- **Language:** note in English with promotor quotes in original language, matching other logs.

## Risks / Trade-offs

- [DK or a peripheral fails on the day] → run the demo end to end once beforehand; carry the latest PPK2 note and diagrams as fallback evidence.
- [Code changes after `8c415fe`, so diagrams stale] → run the drift check the day before; fix in `add-firmware-architecture-diagrams`, re-render, then show.
- [Too many figures for one meeting] → show L1, the L3 parts and the BLE overview; keep L2 and the sequences in reserve.
- [Feedback only remembered later] → write the note during or right after the meeting; commit it immediately.
- [Promotor changes scope (hardware minimum, chapter order)] → copy to the roadmap in the same session and mention affected open changes.
