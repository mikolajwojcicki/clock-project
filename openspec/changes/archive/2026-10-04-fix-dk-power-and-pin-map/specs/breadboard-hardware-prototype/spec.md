# Spec Delta

## ADDED Requirements

### Requirement: Guide identifies the exact nRF52 DK power and ground pins
The guide SHALL name the DK connector and silkscreen label for the breadboard supply and ground, using the labels printed on the board. It SHALL list every DK power-related pin or connector that must not be used for the breadboard rail and say why. It SHALL require the user to measure and record the DK supply rail voltage before connecting any module.

#### Scenario: Beginner looks for the 3.3 V pin
- **WHEN** the beginner reads the power-connection step
- **THEN** the guide names connector `P1`, the pin labeled `VDD`, and a pin labeled `GND`, and states that every `VDD` pin on `P1` is the same rail and every `GND` pin is the same ground

#### Scenario: Beginner sees External supply or 5V labels
- **WHEN** the beginner finds pins labeled `5V`, `External supply`, `VDD_nRF`, `nRF current measurement`, or the coin-cell holder
- **THEN** the guide states that each one must not be used as the breadboard supply, and that `External supply` is a power input that must never be connected while USB powers the DK

#### Scenario: DK rail is below 3.3 V
- **WHEN** the user measures `P1 VDD` to `GND` with USB power
- **THEN** the guide explains that the reverse-protection diode lowers the rail below 3.3 V, gives a stop condition for a missing, unstable, or above-3.6 V reading, and the test record has a field for the measured value

#### Scenario: DK revision differs from the documentation
- **WHEN** the DK label is not `PCA10040` or the version sticker is not 3.x.x
- **THEN** the guide tells the user to stop and compare the connector labels with the matching DK user guide before wiring

## MODIFIED Requirements

### Requirement: Guide defines complete and unambiguous wiring
The guide SHALL provide a complete pin map for power, ground, SPI display signals, display control signals, LIS3DH signals, DCF-77 signals, and buzzer-driver signals. It SHALL use nRF52 DK pin names and module labels, identify unused pins, and warn against unsafe power connections. It SHALL require physical verification of board labels and revision-specific settings before wiring. For the Waveshare Rev2.1 HAT, it SHALL identify the `BS1 = 0` four-wire SPI requirement. For the DCF receiver, it SHALL identify `PON` as active-low and define the powered and unpowered states before connecting it to an MCU GPIO. It SHALL assign project signals only to DK GPIOs that have no on-board button, LED, UART, NFC, crystal, reset, or I/O-expander connection, and SHALL list the reserved DK GPIOs.

#### Scenario: Beginner wires one peripheral
- **WHEN** the beginner follows the pin table and wire-by-wire instructions for one peripheral
- **THEN** every wire has one source pin, one destination pin, and a stated signal or power purpose, and every control pin has a defined safe state

#### Scenario: Module labels differ
- **WHEN** a module has a different label or connector order than the guide
- **THEN** the guide instructs the user to stop, identify the module documentation and voltage pins, and avoid guessing a connection

#### Scenario: User connects the e-paper module
- **WHEN** the user connects the e-paper module
- **THEN** the guide assigns SPI clock and data and the display chip select and data/command signals to DK GPIOs outside `P0.22` through `P0.31`, does not assign SPI or PWM to `P0.22` through `P0.31`, and confirms the HAT is configured for four-wire SPI

#### Scenario: User checks for DK on-board conflicts
- **WHEN** the user reads the pin map
- **THEN** no project signal uses `P0.00`, `P0.01`, `P0.05` through `P0.10`, `P0.13` through `P0.21`, `P0.26`, or `P0.27`, and the guide states which DK button, LED, UART, NFC, crystal, reset, or I/O-expander function reserves each one

#### Scenario: User connects the DCF receiver
- **WHEN** the user connects the verified DCF receiver
- **THEN** the guide connects `VDD` to 3.3 V, `GND` to common ground, keeps `PON` low to enable reception or high to disable it, and leaves `OUT` disconnected until its logic-level compatibility is evidenced
