# Spec Delta

## ADDED Requirements

### Requirement: Generator emits a modulated 77.5 kHz carrier
The generator SHALL output a 77.5 kHz square-wave carrier on `PA6` (Arduino `D12`) for an external coupling loop. Its fundamental amplitude SHALL drop to 15% ± 2% of the selected level for exactly the time that `PA0` is high, and return to the selected level when `PA0` goes low.

#### Scenario: Carrier frequency
- **WHEN** the generator runs from the external 8 MHz clock and a logic analyzer measures `PA6` over at least one second
- **THEN** the carrier frequency is 77 500 Hz within 0.01%

#### Scenario: Modulation follows the logic output
- **WHEN** a logic analyzer records `PA0` and `PA6` together during a frame
- **THEN** the `PA6` duty cycle switches to its pulse value within one carrier period of each `PA0` edge, and stays at its idle value while `PA0` is low

### Requirement: Carrier runs only from the external clock
The generator SHALL enable the carrier only when its timebase runs from the external 8 MHz clock. With the internal oscillator, it SHALL hold `PA6` low, keep the `PA0` logic output working, and report in the startup banner that the carrier is disabled and why.

#### Scenario: External clock missing
- **WHEN** the generator starts and the external clock does not become ready
- **THEN** the banner reports the internal oscillator and a disabled carrier, `PA6` stays low, and `PA0` keeps pulsing

### Requirement: Operator controls carrier level and on/off over UART
The generator SHALL accept a key that cycles the carrier level through 0, -6, -12, and -20 dB relative to full drive, and a key that turns the carrier on and off. A change SHALL take effect within one carrier period and SHALL be confirmed over UART. The banner SHALL show the carrier state and level. After reset, the carrier SHALL be on at -20 dB.

#### Scenario: Level change
- **WHEN** the operator presses the level key
- **THEN** the generator reports the new level, and both the idle and pulse duty cycles on `PA6` change to the values for that level

#### Scenario: Carrier off
- **WHEN** the operator turns the carrier off
- **THEN** `PA6` stays low until the carrier is turned on again, and `PA0` keeps pulsing

### Requirement: Faults act on the carrier like the transmitter would
The existing faults SHALL apply to the carrier through the same timing as `PA0`. A missing pulse SHALL leave the carrier at its idle level for that second, a glitch SHALL reduce it for the glitch duration, and silence SHALL leave an unmodulated carrier at the selected level.

#### Scenario: Silence on the carrier
- **WHEN** the operator selects silence with the carrier on
- **THEN** `PA6` keeps the idle duty cycle without reductions until silence ends at a frame start
