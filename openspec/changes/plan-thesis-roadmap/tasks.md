## 1. Process setup

- [ ] 1.1 Look up WIT harmonogram dyplomowania; write the faculty date, promotor date (−3 d) and draft date (−7 d or earlier) into `notes/deadlines.md`. Verify: file lists three dates.
- [ ] 1.2 Create `notes/` (`README.md`, `media/`, `measurements/`, `ai-use.md`, `sources.md` pointer to bib) and a dated-entry template. Verify: tree exists, committed.
- [ ] 1.3 Backfill notes from `git log` and archived OpenSpec changes (Sep 12 – Oct 6): one dated note per bring-up milestone with problems and solutions. Verify: each archived change has a note.
- [ ] 1.4 Backfill `notes/ai-use.md` (agents wrote firmware/docs/specs since start). Verify: entries per tool and purpose.
- [ ] 1.5 Update `AGENTS.md`/`project-principles.md`: author writes prose, agents scaffold only, evidence-log duty, ethics rule. Verify: grep shows rule; no conflict with writing-rules YAML.
- [ ] 1.6 Ask promotor: PCB expected? human-volunteer tests wanted? Record answers in `notes/`. Verify: answers noted.

## 2. Thesis skeleton (empty, no prose)

- [ ] 2.1 Replace template chapters with six chapter files + AI-disclosure + attachments stubs containing only section headings from the promotor structure. Verify: CI `thesis-pdf` builds and TOC shows all chapters.
- [ ] 2.2 Check `\stTytulPL` equals APD title; confirm `\stCzyMgr=0`. Verify: diff against `temp-resources/my-thesis.pdf`.
- [ ] 2.3 Seed `bibliography.bib` with datasheets already used (nRF52832, MDBT42Q, LIS3DH, SSD1680, DCF-1060N, MCP73831, AP2112K) with `urldate`. Verify: biber has no warnings.

## 3. Diagrams for consultation

- [ ] 3.1 Draw hardware block diagram (KiCad/vector). Verify: PDF in `hardware/docs/`.
- [ ] 3.2 Draw software algorithm flowchart (sleep → wake source → handler → sleep; alarm path; DCF sync window). Verify: shown to promotor.
- [ ] 3.3 Export breadboard schematic blocks as print-safe vector (no grid, black/white). Verify: figures open at print size and are legible.

## 4. Prototype milestones (DK breadboard)

- [ ] 4.1 RTC clock on LFXO with time/day state and e-paper time display. Verify: console log + photo; drift logged over 24 h to CSV.
- [ ] 4.2 Per-weekday alarm table, buzzer trigger, button silence. Verify: alarm fires at set HH:MM on analyzer/log.
- [ ] 4.3 LIS3DH INT1 motion dismiss with required activity count while MCU sleeps. Verify: LED/log shows wake only on motion.
- [ ] 4.4 Deep sleep policy and current measurement (sleep, e-paper refresh, DCF window, alarm). Verify: CSV + conditions in `notes/measurements/`.
- [ ] 4.5 Periodic DCF-77 sync: powered window, decode, RTC correction, PON off. Verify: Nucleo generator drives a sync with logged result.
- [ ] 4.6 Energy budget calculation vs measured values. Verify: table in `hardware/docs/energy-budget.md`.
- [ ] 4.7 SoftDevice BLE service for alarm schedule. Verify: nRF Connect writes a schedule, firmware echoes it.
- [ ] 4.8 Web Bluetooth page in `web/` uploads schedule. Verify: browser upload changes alarm time on DK.

## 5. Product hardware

- [ ] 5.1 Product schematic in KiCad per AGENTS.md architecture (MCU, display header, LIS3DH, DCF, charger, LDO, buzzer, SWD). Verify: ERC clean.
- [ ] 5.2 PCB layout with antenna keep-out, DC-DC parts, 32.768 kHz crystal. Verify: DRC clean (only if promotor expects a PCB).

## 6. Consultations and writing (author)

- [ ] 6.1 First consultation: working prototype, block diagram, flowchart, datasheets. Verify: promotor feedback noted.
- [ ] 6.2 Author writes ch.3 and ch.4 from notes. Verify: draft in LaTeX; each figure/listing referenced.
- [ ] 6.3 Author writes ch.5 from measurements. Verify: tables/charts cite CSV files.
- [ ] 6.4 Author writes ch.2, then ch.1 and ch.6. Verify: ≥12 cited sources, all cited in text.
- [ ] 6.5 Fill AI-disclosure section from `notes/ai-use.md`. Verify: matches register.
- [ ] 6.6 Full draft to promotor ≥1 week before faculty date; final ≤3 days before. Verify: dates from `notes/deadlines.md`.
