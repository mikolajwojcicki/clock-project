## Why

The promotor (Jarosław Emilianowicz) posted the official thesis process in the
17-student Teams chat. It sets a work order (prototype first, prose last),
requires dated evidence collected during bring-up, fixes the chapter layout,
and requires disclosure of AI use. The repo has strong hardware/firmware
bring-up history but no evidence log, a placeholder LaTeX tree, and no
roadmap tied to these rules.

## What Changes

- Add a dated evidence log (`notes/`) with screenshots, console logs, CSV
  measurements, photos, sources and AI-use entries, per promotor rules.
- Restructure `clock-project-latex/` to the 6-chapter layout (Wprowadzenie,
  Przegląd rozwiązań, Projekt sprzętowy, Projekt programowy, Uruchomienie i
  testy, Podsumowanie) plus AI-disclosure and attachments. Agents may draft
  prose; only text the author has rewritten in their own words stays.
- Add the mandatory block diagrams (hardware, software) and a hand-drawn or
  drawn algorithm flowchart for consultations.
- Replace Fritzing-style wiring pictures with a vector, print-ready KiCad
  schematic exported per block.
- Define the remaining prototype firmware milestones (RTC clock, alarm logic,
  sleep + current measurement, energy budget, BLE schedule upload) and the
  order they land in.
- Track deadlines (faculty date, own date = 3 days earlier, full draft one
  week earlier) and the ethics-commission trigger.
- Working rule (author decision, 2026-10-13): agents still draft chapter
  prose. The author rewrites every kept paragraph in their own words and
  expands it. Drafts that were not rewritten do not go into the submitted
  PDF. The author must be able to explain every sentence at the defence.
- The "Informacja o wykorzystaniu narzędzi AI" section is written only by
  the author. Agents do not draft it and do not fill it from the log.
- Faculty antiplagiarism deadline given by the author: 10.12.2025
  (ostateczna, uzgodniona z promotorem wersja). Promotor date: 07.12.2025.
  Full-draft target: 03.12.2025.

## Capabilities

### New Capabilities
- `thesis-evidence-log`: what is recorded, naming, formats, and AI-use tracking
  during bring-up.
- `thesis-document-structure`: required chapters, mandatory diagrams,
  schematic and editorial constraints, deadlines, AI-authorship limits.

### Modified Capabilities
<!-- none; existing specs cover bring-up firmware/hardware and stay valid -->

## Impact

- New `notes/` tree; `clock-project-latex/chapters/*` skeletons; `AGENTS.md`
  thesis-writing section; `hardware/kicad/` schematic exports; `firmware/docs/`.
- No change to locked hardware/firmware architecture.
