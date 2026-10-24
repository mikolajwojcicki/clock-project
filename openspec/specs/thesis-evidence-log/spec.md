# thesis-evidence-log Specification

## Purpose
Define how bring-up evidence, measurements, sources and AI use are recorded for the thesis.

## Requirements

### Requirement: Dated evidence log
The repository SHALL hold a `notes/` log where every entry carries an ISO date
and records what was done, what failed, how it was solved, and why a part or
library was chosen.

#### Scenario: Bring-up session recorded
- **WHEN** a bring-up or design session ends
- **THEN** a dated note for that session exists in `notes/`

### Requirement: Descriptive artifact naming
Screenshots, oscillograms and logic-analyzer captures SHALL be named
`YYYY-MM-DD_<topic>_<detail>.<ext>` and stored under `notes/media/`.

#### Scenario: Analyzer capture saved
- **WHEN** a capture of the DCF generator is saved
- **THEN** its name matches e.g. `2026-10-06_dcf_generator_pulse_timing.png`

### Requirement: Text logs and print-friendly captures
Console and serial output SHALL be stored as text. Any screenshot SHALL use a
light theme (dark text on white).

#### Scenario: UART output kept
- **WHEN** the bring-up banner or a test result is captured
- **THEN** it is saved as a `.txt` log, not only as an image

### Requirement: Measurements with conditions
Measurements SHALL be stored as raw CSV beside a record of instrument,
temperature, supply voltage and firmware commit. For a PPK2 current capture,
a verified packed capture with its sidecar, which unpacks to the same values,
SHALL count as the raw CSV; the CSV export itself then stays local.

#### Scenario: Sleep current measured
- **WHEN** a sleep-current run completes
- **THEN** a CSV or a packed capture with sidecar, and its conditions record, exist in `notes/measurements/`

#### Scenario: Packed capture used as evidence
- **WHEN** a log note cites a PPK2 run stored as a packed capture
- **THEN** the note links the packed file, and unpacking it gives the CSV the note's numbers were computed from

### Requirement: Sources with access date
Each datasheet, document or article used SHALL be added to
`clock-project-latex/bibliography.bib` with full description and access date.

#### Scenario: New datasheet used
- **WHEN** a datasheet drives a design decision
- **THEN** a bib entry with `urldate` exists before the note is closed

### Requirement: AI-use register
Every use of an AI tool SHALL be logged in `notes/ai-use.md` with date, tool,
purpose and how the output was verified.

#### Scenario: Agent writes a driver or a prose draft
- **WHEN** an AI agent generates code or chapter prose that is kept
- **THEN** `notes/ai-use.md` has an entry naming the tool, the output and how the author checked or rewrote it
