## MODIFIED Requirements

### Requirement: Guide identifies required materials and safe preparation
The guide SHALL list every required component, tool, cable, and software prerequisite. It SHALL identify optional parts, owned parts, user-reported purchased items, unconfirmed items, parts that can differ by module revision, and electrical hazards before assembly begins. It SHALL use the reconciled inventory as its source for availability and SHALL never present an unverified item as owned. Before any powered measurement, it SHALL explain correct laboratory-supply voltage and current-limit setup, multimeter current-mode wiring, the prohibition against placing a current meter across a power source, and a pre-power short check.

#### Scenario: Beginner prepares the work area
- **WHEN** a beginner follows the preparation section
- **THEN** the beginner can identify the nRF52 DK, each peripheral, the breadboard rails, the required USB connection, the laboratory supply and meter limits, and the parts that must remain disconnected during initial checks

#### Scenario: User lacks an optional power part
- **WHEN** the user does not have the TP4056 board or Li-Po battery
- **THEN** the guide clearly states that USB power from the nRF52 DK is sufficient for the first prototype and identifies which power experiments must be skipped

#### Scenario: Inventory marks a tool as user-reported
- **WHEN** a tool is listed as user-reported purchased but its exact model or presence is not verified
- **THEN** the guide labels it as user-reported and provides a safe alternative or states which test must wait for verification

#### Scenario: User measures a low-current module
- **WHEN** the user measures DCF receiver current from a laboratory supply
- **THEN** the guide requires a current limit of no more than 100 mA, a series-connected meter with the correct sockets and mode, and a power-off check before enabling output

### Requirement: Guide defines complete and unambiguous wiring
The guide SHALL provide a complete pin map for power, ground, SPI display signals, display control signals, LIS3DH signals, DCF-77 signals, and buzzer-driver signals. It SHALL use nRF52 DK pin names and module labels, identify unused pins, and warn against unsafe power connections. It SHALL require physical verification of board labels and revision-specific settings before wiring. For the Waveshare Rev2.1 HAT, it SHALL identify the `BS1 = 0` four-wire SPI requirement. For the DCF receiver, it SHALL identify `PON` as active-low and define the powered and unpowered states before connecting it to an MCU GPIO.

#### Scenario: Beginner wires one peripheral
- **WHEN** the beginner follows the pin table and wire-by-wire instructions for one peripheral
- **THEN** every wire has one source pin, one destination pin, and a stated signal or power purpose, and every control pin has a defined safe state

#### Scenario: Module labels differ
- **WHEN** a module has a different label or connector order than the guide
- **THEN** the guide instructs the user to stop, identify the module documentation and voltage pins, and avoid guessing a connection

#### Scenario: User connects the e-paper module
- **WHEN** the user connects the e-paper module
- **THEN** the guide assigns SPI signals to safe nRF52 GPIO pins in the P0.11 to P0.20 range, does not assign SPI or PWM to P0.22 through P0.30, and confirms the HAT is configured for four-wire SPI

#### Scenario: User connects the DCF receiver
- **WHEN** the user connects the verified DCF receiver
- **THEN** the guide connects `VDD` to 3.3 V, `GND` to common ground, keeps `PON` low to enable reception or high to disable it, and leaves `OUT` disconnected until its logic-level compatibility is evidenced

### Requirement: Guide uses staged bring-up and observable checks
The guide SHALL require staged testing in an order that limits fault scope. Each stage SHALL state the power state, action, expected result, and stop condition before the next stage is connected. Stages SHALL include an isolated DCF power-control test and SHALL distinguish a multimeter indication of changing `OUT` voltage from a verified digital logic level.

#### Scenario: User verifies the controller first
- **WHEN** the user powers the nRF52 DK with no external peripheral connected and flashes the existing blinky firmware
- **THEN** the DK LED changes state at the documented interval and the guide allows the user to continue only after this check passes

#### Scenario: User tests the display
- **WHEN** the user connects the e-paper module and runs the planned display test
- **THEN** the display shows the documented test pattern or message, and the guide gives checks for power, ground, SPI wiring, reset, busy, chip-select, and four-wire mode when it does not

#### Scenario: User tests motion and radio inputs
- **WHEN** the user tests the LIS3DH interrupt and DCF-77 receiver
- **THEN** the guide defines an observable interrupt or signal result, explains how DCF-77 reception conditions affect the result, records DCF enabled and disabled current, and does not treat missing radio reception as proof of a wiring fault

#### Scenario: User observes DCF output
- **WHEN** the user sees a changing multimeter reading on DCF `OUT`
- **THEN** the guide records that pulses are present but requires a logic analyzer or oscilloscope measurement before treating the signal as an nRF52-compatible digital input

#### Scenario: User tests the buzzer
- **WHEN** the user tests the buzzer circuit
- **THEN** the buzzer is driven through the NPN transistor, the base resistor is present, the flyback diode is installed with stated polarity, the exact buzzer rating and transistor pinout are checked, and the guide warns that the buzzer must not be driven directly from a GPIO pin

### Requirement: Guide supports fault isolation and safe recovery
The guide SHALL provide a fault-isolation procedure for no power, unexpected heat, a failed firmware flash, a blank display, false or missing motion interrupts, missing DCF-77 data, an invalid DCF output level, unsafe multimeter readings, and a silent buzzer. It SHALL define when to remove USB or laboratory-supply power and how to return to the last known-good stage. It SHALL require stopping before reconnection after any reversed-power, short-circuit, overcurrent, or instrument-mode error.

#### Scenario: User sees unexpected heat or smell
- **WHEN** any board, wire, or component becomes hot or smells unusual
- **THEN** the user is instructed to disconnect USB power and laboratory-supply output immediately and inspect for reversed power, a short circuit, or an incorrect rail before reconnecting

#### Scenario: A later stage fails
- **WHEN** a test fails after an earlier stage passed
- **THEN** the guide tells the user to disconnect the newest peripheral, restore the last known-good wiring, and retest before changing firmware or adding more wires

#### Scenario: User makes a current-measurement mistake
- **WHEN** the meter is in current mode with incorrect sockets, is placed across supply terminals, or reports an unexpected high current
- **THEN** the guide tells the user to disable the supply, remove power, correct the meter setup, inspect the meter fuse and wiring, and repeat the pre-power checks before reconnecting

### Requirement: Guide records a reproducible prototype result
The guide SHALL provide a completion checklist and a test record template. The record SHALL capture module identities, pin assignments, firmware revision or commit, test conditions, observed results, unresolved faults, and changes from the documented wiring. It SHALL also capture supply voltage and current limit, meter method, DCF `PON` polarity, enabled and disabled DCF current, display `BS1` setting, and evidence status for DCF `OUT` logic-level compatibility and buzzer part ratings.

#### Scenario: Prototype passes the planned tests
- **WHEN** the user completes the checklist
- **THEN** the user can record which subsystems passed, under what conditions they passed, which safety checks were completed, and which prototype limitations remain

#### Scenario: Prototype cannot receive DCF-77
- **WHEN** the prototype cannot receive a valid DCF-77 frame during the test period
- **THEN** the record can separate environmental or timing limitations from confirmed hardware failures and still document the other completed tests

#### Scenario: DCF electrical behavior is characterized
- **WHEN** the user completes the DCF power test
- **THEN** the record contains the measured enabled and disabled current, `PON` state, supply voltage, and whether `OUT` logic levels were measured with suitable equipment
