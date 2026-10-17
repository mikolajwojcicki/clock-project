# thesis-document-structure Specification

## Purpose
TBD - created by archiving change plan-thesis-roadmap. Update Purpose after archive.

## Requirements

### Requirement: Faculty chapter layout
The thesis SHALL contain: 1 Wprowadzenie (0,5–1,5 p., with cel i zakres and
układ pracy), 2 Przegląd rozwiązań (3–5 p., comparison table, ends with
differences and justification of choices), 3 Projekt części sprzętowej
(≈40 %), 4 Projekt części programowej (≈40 %), 5 Uruchomienie i testy
(3–5 p.), 6 Podsumowanie (0,5–1,5 p.), Literatura (at least a dozen cited
items), AI-use information and optional attachments.

#### Scenario: Skeleton compiles
- **WHEN** the LaTeX tree is built
- **THEN** all six chapters, bibliography and AI-disclosure section appear in the table of contents

### Requirement: Mandatory block diagrams
Chapter 3 SHALL open with a hardware block diagram and chapter 4 with a
software block diagram. Each subsection SHALL carry its own schematic, part
justification and required calculations.

#### Scenario: Power block documented
- **WHEN** the power subsection is written
- **THEN** it has a schematic, regulator/charger justification, a current budget and estimated battery life

### Requirement: Schematics are schematics
Hardware figures SHALL be IEC-symbol schematics exported as vector, black on
white without grid, inputs left, outputs right, power up, ground down. Wiring
pictures (Fritzing-style) SHALL NOT appear.

#### Scenario: Figure exported
- **WHEN** a schematic is added to the thesis
- **THEN** it is a PDF/SVG export from KiCad with reference designators and values

### Requirement: Title consistency
The title on the title page SHALL equal the approved APD title.

#### Scenario: Title checked
- **WHEN** the thesis is prepared for submission
- **THEN** `\stTytulPL` equals the APD title character for character

### Requirement: Rewritten prose
Agents MAY draft chapter prose. Text SHALL enter the submitted thesis only
after the author rewrites it in their own words and expands it. Each kept
draft SHALL be logged in `notes/ai-use.md`. The author SHALL be able to
explain every included sentence at the defence.

#### Scenario: Agent drafts a chapter
- **WHEN** an agent drafts chapter prose
- **THEN** the draft is marked as a draft, logged in `notes/ai-use.md`, and is not treated as final until the author rewrites it

### Requirement: Author-only AI disclosure
The body of the "Informacja o wykorzystaniu narzędzi AI" section SHALL be
written by the author. Agents SHALL NOT draft it, fill it, or generate its
sentences. Agents MAY leave an empty heading stub. The register in
`notes/ai-use.md` is source material for the author, not a generated section.

#### Scenario: Agent reaches the disclosure section
- **WHEN** an agent is asked to write or complete the AI-disclosure section
- **THEN** it refuses and leaves the section for the author

### Requirement: Deadline discipline
The plan SHALL track the faculty deadline, the promotor deadline (3 days
earlier, final) and a full-draft target at least one week before the faculty
date. Research with human participants SHALL NOT start before ethics-commission
approval requested through the promotor.

#### Scenario: Shake-dismiss user test proposed
- **WHEN** a test with human volunteers is proposed
- **THEN** the promotor is asked to file the ethics request first, or the test is replaced by instrument-based motion tests
