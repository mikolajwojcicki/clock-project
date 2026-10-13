## Context

Promotor rules (Teams chat dump, 2026-10-13): prototype first; chapters written
in order 3,4,5 → 2 → 1,6; keep loose dated notes; show working prototype plus
algorithm flowchart at consultations; schemas must be real schematics.
Latex.pdf is only a layout example (old mwcls/Wydział Elektroniki); the WIT
`szablonwitpd` stays the template.

Where the repo stands:
- Done: breadboard guide + KiCad breadboard schematic; DK bring-up firmware
  (e-paper, LIS3DH, DCF-77 receiver decode, buzzer, 4 buttons); Nucleo DCF-77
  generator (pulse timing, 77.5 kHz carrier) and DCF-1060N bench results.
- Not started: RTC clock + alarm logic, deep-sleep and current measurements,
  energy budget, BLE + web app, product PCB, notes log, thesis chapters.
- LaTeX: template placeholders only (`wstep.tex`, `info.tex`).

## Goals / Non-Goals

**Goals:** one ordered roadmap that updates via tasks.md; evidence captured
from now on; thesis skeleton ready for agent drafts the author rewrites.
**Non-Goals:** writing chapter text; changing locked architecture; building the
PCB in this change (scheduled, not specified).

## Decisions

1. **Evidence in `notes/` inside the monorepo** (not Obsidian/Word). Git history
   already feeds the thesis; same place, same commits. Alternative: separate
   notes repo — rejected, drifts from code.
2. **Backfill from git log + archived OpenSpec changes** once, then log live.
   Past bench work (Oct 4–6) is the freshest, most recoverable.
3. **Prototype order:** RTC+clock display → alarm schedule + buzzer → motion
   dismiss via INT1 → deep sleep + current measurement → DCF periodic sync →
   BLE schedule upload → web app. Reason: sleep numbers drive ch.5 and must
   exist before BLE adds SoftDevice complexity; BLE last because it is
   configuration only.
4. **Thesis chapters follow the faculty layout**, replacing the earlier
   "theory chapter" idea. Theory (DCF-77 coding, low-power methods) moves into
   ch.2/3/4 where used. Promotor overrides `thesis-writing-rules.yaml`.
5. **Schematic per block from KiCad** (MCU, display, sensor, DCF, power,
   buzzer) with print-safe export script/steps; product schematic precedes
   the PCB layout.
6. **Draft, then rewrite.** Agents may draft chapter prose (author decision,
   2026-10-13). A paragraph stays in the thesis only after the author rewrites
   it in their own words. Drafts are labeled and logged in `notes/ai-use.md`.
   Promotor's hard limits that stay: explain every sentence at the defence;
   check sources the agent proposes. The AI-disclosure section is author-only:
   agents never draft or fill it.
7. **Human-subject tests avoided** by default (shake test with instrument or
   self-test logged as engineering measurement); ask promotor if volunteers
   are wanted.

## Risks / Trade-offs

- [2026 harmonogram not confirmed] → use 10.12.2026 as a working date (same
  day as last year). Replace it when the faculty page for this year is out.
- [Prototype slips, thesis starts late] → fixed checkpoints per milestone;
  chapter 3 can start once hardware blocks are frozen.
- [AI-use disclosure incomplete] → agents keep `notes/ai-use.md` only; the author writes the thesis section.
- [PCB lead time] → decide order/no-order for PCB with promotor; breadboard
  prototype is acceptable for ch.5 per faculty note.

## Deadlines

Last year's faculty line (harmonogram WIT): upload to the antiplagiarism
system of the final version agreed with the promotor — 10.12.2025.
Working assumption for this year, same calendar day, **not confirmed**:
**10.12.2026**.

| Milestone | Date |
| --- | --- |
| Full draft to promotor (≥1 week earlier) | 03.12.2026 |
| Promotor's last reading (3 days earlier) | 07.12.2026 |
| Faculty antiplagiarism upload | 10.12.2026 |

## Decisions added during apply

- **Prototype hardware (author, 2026-10-13):** soldered perfboard, no jumpers. Breadboard is bring-up only. A custom PCB is not required.
- **No volunteer tests.** No ethics request.
