# Spec Delta

## Purpose

Give the thesis and reviewers accurate, reproducible diagrams that explain how the clock firmware and its Bluetooth path work, at several levels of detail.

## ADDED Requirements

### Requirement: Three detail levels of one firmware flow
The repository SHALL contain three PlantUML diagrams of the clock firmware behavior, each describing the same system at a different level: overview, module flow, and low-level flow. Each lower level SHALL refine, not contradict, the level above.

#### Scenario: Overview
- **WHEN** a reader opens the level 1 diagram
- **THEN** it shows the firmware power states and the events that move it between them

#### Scenario: Module level
- **WHEN** a reader opens the level 2 diagram
- **THEN** it shows the main loop, interrupt and handler sources, and the DCF-77, display, alarm, motion, schedule storage and BLE modules with their interactions

#### Scenario: Low level
- **WHEN** a reader opens the level 3 diagram
- **THEN** it shows per-handler decision flow, the peripherals used, and how the firmware enters and leaves sleep

### Requirement: As-built accuracy
Every diagram SHALL describe what the code in `firmware/apps/clock` and `firmware/src` does, not what the specs intend. Where code and a main spec disagree, the difference SHALL be listed in the diagram index.

#### Scenario: Code differs from spec
- **WHEN** the code behaves differently from an `openspec/specs` requirement
- **THEN** the diagram follows the code and the index lists that difference

### Requirement: Bluetooth explained in two views
The repository SHALL contain one BLE diagram and one BLE sequence diagram (each MAY be split into labelled parts). The BLE diagram SHALL show the web app, the browser and operating system BLE stack, the SoftDevice, the firmware BLE handler code, and the schedule store. The role of the SoftDevice SHALL be described in a companion file next to the diagrams, not inside the diagram. The sequence diagram SHALL show the ordered messages of a schedule read and a schedule write.

#### Scenario: SoftDevice role
- **WHEN** a reader follows the SoftDevice reference from the BLE diagram
- **THEN** the companion file states what the SoftDevice is, which parts of the stack it owns, and which events the application receives from it

#### Scenario: Both directions between web app and firmware
- **WHEN** a reader follows the BLE sequence diagram
- **THEN** it shows how the web app reaches the firmware (request, connect, GATT calls) and how the firmware responds, including the advertising window and disconnect

### Requirement: Reproducible render
Each diagram SHALL have a committed PlantUML source and committed SVG and PDF exports. The index SHALL give the exact commands that regenerate the exports using the project's PlantUML podman container. Polish characters SHALL render correctly.

#### Scenario: Regenerate
- **WHEN** the documented commands are run on the sources
- **THEN** the SVG and PDF files are produced without errors

### Requirement: Draft thesis captions
The index or a captions file SHALL hold one draft Polish caption per diagram, marked as an agent draft to be rewritten by the author.

#### Scenario: Caption present
- **WHEN** a reader looks up a diagram in the captions file
- **THEN** a draft caption marked as an agent draft exists for it

### Requirement: Glossary and naming
A glossary file next to the diagrams SHALL explain every abbreviation and technical term used in the diagrams. The display SHALL be named "e-Paper" in diagrams and their companion files.

#### Scenario: Unknown term
- **WHEN** a reader meets an abbreviation in any diagram
- **THEN** the glossary explains it
