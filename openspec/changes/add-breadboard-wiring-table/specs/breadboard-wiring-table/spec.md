# Spec Delta

## Purpose
Give one authoritative per-wire table of the breadboard prototype so the board can be rebuilt and later routed on perfboard.

## ADDED Requirements

### Requirement: Per-wire connection table
The repository SHALL provide `hardware/docs/breadboard-wiring-table.md` with one row for every wire of the breadboard prototype. Each row SHALL give the signal name, the class (`power`, `bus` or `signal`), both endpoints as device plus pin label, and a note.

#### Scenario: Every wire has two endpoints
- **WHEN** a reader looks up any wire in the table
- **THEN** both the DK or module pin and the other device pin are named, using the labels printed on the board

#### Scenario: Table matches the firmware pin map
- **WHEN** the DK pins in the table are compared with `firmware/boards/dk_breadboard_pins.h`
- **THEN** every project signal uses the same DK pin in both places

### Requirement: Bus and signal separation
The table SHALL group wires into power, the shared SPI bus, point-to-point signals and the buzzer stage. For each bus signal it SHALL list every device that shares it.

#### Scenario: SPI sharing is visible
- **WHEN** a reader looks up `SPI_SCK`, `SPI_MOSI` or `SPI_MISO`
- **THEN** the table lists each device on that line and states which DK pin is the single bus source

#### Scenario: Low-drive pins are flagged
- **WHEN** a signal uses a pin in `P0.22` to `P0.31`
- **THEN** the table marks it low-frequency only (at most 10 kHz)

### Requirement: Rebuild order and checks
The document SHALL state a wiring order (ground and 3.3 V first, then bus, then signals, then the buzzer stage) and a continuity check after each group, including no short between `3V3` and `GND`.

#### Scenario: Rebuild from a bare breadboard
- **WHEN** the reader follows the order and checks
- **THEN** the finished wiring matches the table with no extra wires
