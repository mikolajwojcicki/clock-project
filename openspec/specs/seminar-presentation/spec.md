# seminar-presentation Specification

## Purpose
TBD - created by archiving change add-seminar-presentation. Update Purpose after archive.

## Requirements

### Requirement: Separate Beamer compile target
The thesis repo `clock-project-latex` SHALL contain `presentation.tex`, a Beamer document built independently of `main.tex`. Building one SHALL NOT require or change the other.

#### Scenario: Presentation builds alone
- **WHEN** `presentation.tex` is compiled with `pdflatex`
- **THEN** `presentation.pdf` is produced with no unresolved references and no missing-file errors

#### Scenario: Thesis build unaffected
- **WHEN** `main.tex` is compiled after this change
- **THEN** `main.pdf` is identical in structure to the build before the change

### Requirement: Ten-minute length
The deck SHALL have at most 11 frames (one title frame and up to 10 content frames) so that it fits 10 minutes at about one minute per content frame. It SHALL contain no speaker notes.

#### Scenario: Frame budget
- **WHEN** the PDF page count is read
- **THEN** it is 11 or fewer

### Requirement: Polish language and project title
All slide text SHALL be in Polish and the title frame SHALL show the approved APD title, the author, the supervisor and the programme exactly as in `main.tex`.

#### Scenario: Title matches thesis
- **WHEN** the title frame is compared with `\stTytulPL`, `\stAutor` and `\stOpiekun` in `main.tex`
- **THEN** the strings are equal

### Requirement: Required content arc
The deck SHALL cover, in order: the problem and goal, the requirements, the architecture block diagram, the hardware prototype, the low-power firmware approach, DCF-77 synchronisation, the author's own DCF-77 generator, e-paper and motion dismiss, current measurements, and one closing frame combining the status and the direction.

#### Scenario: Own generator explained
- **WHEN** the generator frame is read
- **THEN** it says that real DCF-77 could not be received indoors, that a Nucleo board generates the time frame, and that pulsing one pin switches the 77.5 kHz carrier amplitude to imitate the radio signal into the receiver's antenna

#### Scenario: Closing frame is brief
- **WHEN** the last frame is read
- **THEN** it gives a short, general statement of what is done and a short, general statement of where the work is headed, with no task lists and no dates, and shows a "stan na" date

### Requirement: Claims backed by the repo
Every number, photo and plot on a slide SHALL come from the repo (`notes/log/`, `notes/measurements/`, `notes/media/`, `temp-resources/` photos). A figure with no recorded source SHALL NOT appear.

#### Scenario: Measurement traceable
- **WHEN** a current value appears on a slide
- **THEN** the same value appears in a dated note under `notes/log/` and the slide names the run

### Requirement: Real photos, no placeholders
The deck SHALL include real photographs of the project hardware and SHALL NOT contain placeholder boxes or missing-image fallbacks. Images used SHALL be stored under `clock-project-latex/figures/prezentacja/`.

#### Scenario: Submodule is self-contained
- **WHEN** `clock-project-latex` is cloned alone and `presentation.tex` is compiled
- **THEN** every `\includegraphics` path resolves inside that repo

### Requirement: CI artifact
The `clock-project-latex` CI workflow SHALL compile `presentation.tex` on every push and pull request and upload it as artifact `presentation-pdf`. The existing `thesis-pdf` artifact SHALL remain.

#### Scenario: Artifact retrievable
- **WHEN** a CI run finishes
- **THEN** `gh run download -R mikolajwojcicki/clock-project-latex -n presentation-pdf` yields `presentation.pdf`

### Requirement: AI use logged
Any AI-drafted slide text kept in the deck SHALL be logged in `notes/ai-use.md`, and the author SHALL review it before the presentation.

#### Scenario: Draft logged
- **WHEN** the deck is committed with AI-drafted text
- **THEN** `notes/ai-use.md` has a dated entry naming `presentation.tex`
