## Why

The promotor (Jarosław Emilianowicz) posted the official thesis process in the
17-student Teams chat. It sets a work order (prototype first, prose last),
requires dated evidence collected during bring-up, fixes the chapter layout,
and forbids AI-generated chapters. The repo has strong hardware/firmware
bring-up history but no evidence log, a placeholder LaTeX tree, and no
roadmap tied to these rules.

## What Changes

- Add a dated evidence log (`notes/`) with screenshots, console logs, CSV
  measurements, photos, sources and AI-use entries, per promotor rules.
- Restructure `clock-project-latex/` to the 6-chapter layout (Wprowadzenie,
  Przegląd rozwiązań, Projekt sprzętowy, Projekt programowy, Uruchomienie i
  testy, Podsumowanie) plus AI-disclosure and attachments. Empty skeletons
  only; no generated prose.
- Add the mandatory block diagrams (hardware, software) and a hand-drawn or
  drawn algorithm flowchart for consultations.
- Replace Fritzing-style wiring pictures with a vector, print-ready KiCad
  schematic exported per block.
- Define the remaining prototype firmware milestones (RTC clock, alarm logic,
  sleep + current measurement, energy budget, BLE schedule upload) and the
  order they land in.
- Track deadlines (faculty date, own date = 3 days earlier, full draft one
  week earlier) and the ethics-commission trigger.
- **BREAKING** (agent workflow): AGENTS.md says agents draft thesis prose.
  Promotor forbids AI-generated chapters. Agents limited to structure
  scaffolding, checks, notes and code; the author writes all chapter text.

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
