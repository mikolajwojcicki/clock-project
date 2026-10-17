## MODIFIED Requirements

### Requirement: Generator builds and flashes with installed tools
The generator SHALL run on a board and runtime recorded in the feasibility decision. If the port goes ahead, it SHALL be written in CircuitPython, SHALL be installed by copying files or by a documented DFU command using tools from a `nix-shell`, and SHALL NOT need a global package install. If the port does not go ahead, the existing C generator SHALL remain the generator.

#### Scenario: Operator builds and flashes
- **WHEN** the generator stays in C and the operator runs `make` and then `make flash` with the Nucleo connected over USB
- **THEN** the build completes without errors and OpenOCD programs, verifies, and resets the board

#### Scenario: Install on a clean host
- **WHEN** the README steps are followed on this NixOS host
- **THEN** the generator runs and prints its banner on the console

#### Scenario: No-go
- **WHEN** the feasibility gate ends in no-go
- **THEN** the reason is recorded in `notes/log/` and the C generator and its spec are unchanged

## ADDED Requirements

### Requirement: Python generator keeps the recorded timing
If the port goes ahead, the generator SHALL meet the pulse and carrier results recorded on 2026-10-06: pulse widths 100 / 200 ms and spacing 1000 ms within 2 ms, a minute gap of 2000 ms within 2 ms, the 77.5 kHz carrier from a hardware PWM, and the 15% dip within one carrier period of each `PA0`-equivalent edge, or the deviation SHALL be recorded and accepted by the author.

#### Scenario: Analyzer check
- **WHEN** the output is captured at 1 MS/s for 190 s
- **THEN** pulse widths and spacing are within the limits above and 3 decoded frames match the console log bit for bit

#### Scenario: DK decode
- **WHEN** the DK bring-up DCF test reads the generator for 160 s
- **THEN** it reports valid pulses, three minute markers 60 s apart, and decoded frames equal to the generator log
