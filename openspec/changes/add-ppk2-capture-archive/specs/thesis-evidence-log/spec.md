# Spec Delta

## MODIFIED Requirements

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
