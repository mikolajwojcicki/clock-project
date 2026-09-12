## Purpose

This capability provides a safe, repeatable way for a beginner to assemble and test the first alarm-clock hardware prototype on solderless breadboards.

## Requirements

### Requirement: Guide identifies required materials and safe preparation

The guide SHALL list every required component, tool, cable, and software prerequisite. It SHALL identify optional parts, owned parts, parts that can differ by module revision, and electrical hazards before assembly begins.

#### Scenario: Beginner prepares the work area

- **WHEN** a beginner follows the preparation section
- **THEN** the beginner can identify the nRF52 DK, each peripheral, the breadboard rails, the required USB connection, and the parts that must remain disconnected during initial checks

#### Scenario: User lacks an optional power part

- **WHEN** the user does not have the TP4056 board or Li-Po battery
- **THEN** the guide clearly states that USB power from the nRF52 DK is sufficient for the first prototype and identifies which power experiments must be skipped

### Requirement: Guide defines complete and unambiguous wiring

The guide SHALL provide a complete pin map for power, ground, SPI display signals, display control signals, LIS3DH signals, DCF-77 signals, and buzzer-driver signals. It SHALL use nRF52 DK pin names and module labels, identify unused pins, and warn against unsafe power connections.

#### Scenario: Beginner wires one peripheral

- **WHEN** the beginner follows the pin table and wire-by-wire instructions for one peripheral
- **THEN** every wire has one source pin, one destination pin, and a stated signal or power purpose

#### Scenario: Module labels differ

- **WHEN** a module has a different label or connector order than the guide
- **THEN** the guide instructs the user to stop, identify the module documentation and voltage pins, and avoid guessing a connection

#### Scenario: User connects the e-paper module

- **WHEN** the user connects the e-paper module
- **THEN** the guide assigns SPI signals to safe nRF52 GPIO pins in the P0.11 to P0.20 range and does not assign SPI or PWM to P0.22 through P0.30

### Requirement: Guide uses staged bring-up and observable checks

The guide SHALL require staged testing in an order that limits fault scope. Each stage SHALL state the power state, action, expected result, and stop condition before the next stage is connected.

#### Scenario: User verifies the controller first

- **WHEN** the user powers the nRF52 DK with no external peripheral connected and flashes the existing blinky firmware
- **THEN** the DK LED changes state at the documented interval and the guide allows the user to continue only after this check passes

#### Scenario: User tests the display

- **WHEN** the user connects the e-paper module and runs the planned display test
- **THEN** the display shows the documented test pattern or message, and the guide gives checks for power, ground, SPI wiring, reset, busy, and chip-select signals when it does not

#### Scenario: User tests motion and radio inputs

- **WHEN** the user tests the LIS3DH interrupt and DCF-77 receiver
- **THEN** the guide defines an observable interrupt or signal result, explains how DCF-77 reception conditions affect the result, and does not treat missing radio reception as proof of a wiring fault

#### Scenario: User tests the buzzer

- **WHEN** the user tests the buzzer circuit
- **THEN** the buzzer is driven through the NPN transistor, the base resistor is present, the flyback diode is installed with stated polarity, and the guide warns that the buzzer must not be driven directly from a GPIO pin

### Requirement: Guide supports fault isolation and safe recovery

The guide SHALL provide a fault-isolation procedure for no power, unexpected heat, a failed firmware flash, a blank display, false or missing motion interrupts, missing DCF-77 data, and a silent buzzer. It SHALL define when to remove USB power and how to return to the last known-good stage.

#### Scenario: User sees unexpected heat or smell

- **WHEN** any board, wire, or component becomes hot or smells unusual
- **THEN** the user is instructed to disconnect USB power immediately and inspect for reversed power, a short circuit, or an incorrect rail before reconnecting

#### Scenario: A later stage fails

- **WHEN** a test fails after an earlier stage passed
- **THEN** the guide tells the user to disconnect the newest peripheral, restore the last known-good wiring, and retest before changing firmware or adding more wires

### Requirement: Guide records a reproducible prototype result

The guide SHALL provide a completion checklist and a test record template. The record SHALL capture module identities, pin assignments, firmware revision or commit, test conditions, observed results, unresolved faults, and changes from the documented wiring.

#### Scenario: Prototype passes the planned tests

- **WHEN** the user completes the checklist
- **THEN** the user can record which subsystems passed, under what conditions they passed, and which prototype limitations remain

#### Scenario: Prototype cannot receive DCF-77

- **WHEN** the prototype cannot receive a valid DCF-77 frame during the test period
- **THEN** the record can separate environmental or timing limitations from confirmed hardware failures and still document the other completed tests
