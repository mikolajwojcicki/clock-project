## 1. Process setup

- [x] 1.1 Write `notes/deadlines.md`: working faculty date 10.12.2026 (same day as last year's 10.12.2025 line, not confirmed for this year), promotor date 07.12.2026, full-draft target 03.12.2026. Verify: file lists those three dates and says the year is assumed.
- [x] 1.2 Create `notes/` (`README.md`, `media/`, `measurements/`, `ai-use.md`, `sources.md` pointer to bib) and a dated-entry template. Verify: tree exists, committed.
- [x] 1.3 Backfill notes from `git log` and archived OpenSpec changes (Sep 12 – Oct 6): one dated note per bring-up milestone with problems and solutions. Verify: each archived change has a note.
- [x] 1.4 Backfill `notes/ai-use.md` (agents wrote firmware/docs/specs since start). Verify: entries per tool and purpose.
- [x] 1.5 Update `AGENTS.md`/`project-principles.md`: agents may draft prose; author rewrites every kept paragraph; log drafts in `notes/ai-use.md`; evidence-log duty; ethics rule. Verify: grep shows the rewrite rule.
- [x] 1.6 Record the hardware minimum and the volunteer decision in `notes/promotor-questions.md`. Verify: answers noted.

## 2. Thesis skeleton

- [x] 2.1 Replace template chapters with six chapter files + an empty AI-disclosure heading (no body text) + attachments stubs with section headings from the promotor structure. Verify: CI `thesis-pdf` builds and TOC shows all chapters. The disclosure body stays empty.
- [x] 2.2 Check `\stTytulPL` equals APD title; confirm `\stCzyMgr=0`. Verify: diff against `temp-resources/my-thesis.pdf`.
- [x] 2.3 Seed `bibliography.bib` with datasheets already used (nRF52832, MDBT42Q, LIS3DH, SSD1680, DCF-1060N, MCP73831, AP2112K) with `urldate`. Verify: biber has no warnings.

## 3. Diagrams for consultation

- [x] 3.1 Draw hardware block diagram (KiCad/vector). Verify: PDF in `hardware/docs/`.
- [x] 3.2 Software algorithm flowchart drawn (`firmware/docs/software-flowchart.*`). Showing it to the promotor moved to change `prepare-first-consultation`.
- [x] 3.3 Export breadboard schematic blocks as print-safe vector (no grid, black/white). Verify: figures open at print size and are legible.

## 4. Prototype milestones (DK breadboard)

Moved out of this planning change into their own changes; order and dependencies are in `openspec/roadmap.md`.

- [x] 4.1 -> `add-rtc-clock-drift-log`
- [x] 4.2 -> `add-alarm-schedule-and-buzzer`
- [x] 4.3 -> `add-motion-dismiss`
- [x] 4.4 -> `add-deep-sleep-and-current-measurement`
- [x] 4.5 -> `add-dcf77-periodic-sync`
- [x] 4.6 -> `add-energy-budget`
- [x] 4.7 -> `add-ble-alarm-schedule-service`
- [x] 4.8 -> `add-web-bluetooth-schedule-page`

## 5. Product hardware

- [x] 5.1 Perfboard build moved to change `build-perfboard-prototype`.
- [x] 5.2 Custom PCB is not required. Recorded in `notes/promotor-questions.md` and `design.md`. Verify: both files say perfboard is the minimum.

## 6. Consultations and writing (author)

Moved out as well.

- [x] 6.1 -> `prepare-first-consultation`
- [x] 6.2 -> `draft-thesis-chapters-3-4`
- [x] 6.3 -> `draft-thesis-chapter-5`
- [x] 6.4 -> `draft-thesis-chapters-2-1-6`
- [x] 6.5, 6.6 -> `submit-thesis`

## 7. Roadmap artifact

- [x] 7.1 Write `openspec/roadmap.md`: ordered stages, dependencies, schedule, cut line. Verify: every stage names an existing change.
