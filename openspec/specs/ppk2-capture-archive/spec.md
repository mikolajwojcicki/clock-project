# ppk2-capture-archive Specification

## Purpose
Keeps PPK2 current captures small enough to track in git without losing any
sample, so raw sleep-current evidence for the thesis lives in the repository.

## Requirements

### Requirement: Lossless packing of PPK2 exports
The project SHALL provide a pack command that turns a PPK2 Data Logger CSV
export into a compressed capture file plus a JSON sidecar. Both CSV layouts the
PPK2 app writes SHALL be accepted: `Timestamp(ms),Current(uA)` and
`Timestamp(ms),Current(uA),D0-D7`.

#### Scenario: Export with digital channels
- **WHEN** a CSV with a `D0-D7` column is packed
- **THEN** a capture file and sidecar exist next to it, and the sidecar states that eight digital channels are stored

#### Scenario: Export without digital channels
- **WHEN** a CSV with only timestamp and current is packed
- **THEN** the capture holds current only and the sidecar states that no digital channels are stored

### Requirement: Pack verified before success
The pack command SHALL read the written capture back and compare every sample
with the source CSV: current at the CSV's three decimal places and every
digital bit. It SHALL report success only when all samples match, and SHALL
never delete or modify the source CSV.

#### Scenario: All samples match
- **WHEN** packing finishes and the read-back equals the CSV for every row
- **THEN** the command exits 0 and prints sample count, duration, source and packed sizes

#### Scenario: Any sample differs
- **WHEN** one sample read back differs from the CSV
- **THEN** the command exits non-zero, names the first differing row, and leaves no capture or sidecar behind

### Requirement: Uniform sample timing
The capture SHALL store no per-sample timestamps; timing SHALL be rebuilt from
the sample rate and start offset in the sidecar. Packing SHALL refuse a CSV
whose timestamps are not evenly spaced at one rate.

#### Scenario: Gap in timestamps
- **WHEN** a CSV has a missing or repeated timestamp
- **THEN** packing fails with the row of the first irregular step

### Requirement: Sidecar describes the capture
The sidecar SHALL record format version, sample rate, start time offset,
sample count, current sample type, digital channel count and bit order, source
CSV file name, size and SHA-256, and the source CSV's header line.

#### Scenario: Sidecar read without the tool
- **WHEN** someone opens the sidecar in a text editor
- **THEN** it states enough to decode the capture without reading the tool source

### Requirement: Unpack to CSV
The project SHALL provide an unpack command that writes a CSV with the source
header and, for every row, the same current text at three decimal places and
the same digital bits as the source CSV.

#### Scenario: Round trip
- **WHEN** a packed capture is unpacked
- **THEN** the current and digital columns equal the original CSV row for row, and timestamps match to 0.01 ms

### Requirement: Plotting from packed captures
`ppk-plot` SHALL accept a packed capture wherever it accepts a CSV, with the
same options and the same numeric output.

#### Scenario: Same mean from both inputs
- **WHEN** `ppk-plot --mean` runs on a CSV and on its packed capture
- **THEN** both print the same sample count, minimum, maximum and mean

### Requirement: Tracked capture size limit
A packed capture committed to git SHALL be under 50 MB. The pack command SHALL
warn when its output is 50 MB or more, and SHALL accept a start and end time so
a long export can be packed as shorter windows.

#### Scenario: Long capture
- **WHEN** a capture packs to 50 MB or more
- **THEN** the pack command prints a warning, and the file is not committed until it is packed again as shorter windows

#### Scenario: Window packed
- **WHEN** a start and end time are given
- **THEN** only samples in that window are packed and verified, and the sidecar records the window's start offset in the source
