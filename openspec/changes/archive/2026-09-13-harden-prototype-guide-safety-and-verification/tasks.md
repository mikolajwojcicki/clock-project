## 1. Update safety and preparation instructions

- [x] 1.1 Add KORAD KKG305D setup, 100 mA maximum current limit, multimeter socket and mode checks, series-current wiring, and pre-power short checks; verify the section forbids placing a current-mode meter across a supply.
- [x] 1.2 Add stop conditions for reversed power, unexpected current, hot parts, damaged meter fuse, loose rails, and incorrect instrument setup; verify every condition directs power removal before inspection.
- [x] 1.3 Keep USB-only first bring-up and Li-Po/charger isolation explicit; verify the guide does not introduce a second power source into the first prototype.

## 2. Correct board-specific wiring

- [x] 2.1 Update the Waveshare Rev2.1 section with the photographed board identity and a required `BS1 = 0` four-wire SPI check; verify the display pin table remains consistent with the existing nRF52 pin map.
- [x] 2.2 Update the LIS3DH section with photographed breakout labels, correct power-path guidance, and the datasheet decoupling requirement; verify `VDD`, `VDD_IO`, ground, SPI, and interrupt constraints are unambiguous.
- [x] 2.3 Update the DCF section with labeled pin identity, active-low `PON`, verified `68.3 µA` enabled current, verified `0.0 µA` disabled current, and safe control states; verify no step asks the beginner to guess connector order.
- [x] 2.4 Keep the buzzer section conditional on exact buzzer rating and transistor package pinout; verify it retains the base resistor, flyback diode polarity, transistor drive, and short-duration test limit.

## 3. Gate signal connection and evidence

- [x] 3.1 Distinguish changing DCF `OUT` voltage on a multimeter from verified digital logic levels; verify the guide requires an oscilloscope or logic analyzer before MCU connection when voltage compatibility is unproven.
- [x] 3.2 Add explicit safe defaults for DCF `PON`, display control pins, sensor chip select, and buzzer control; verify each default prevents unintended activation during reset or wiring.
- [x] 3.3 Expand the completion checklist and test record with supply voltage, current limit, meter method, DCF states and currents, HAT SPI mode, signal-level evidence, and buzzer identity; verify every new field can be filled from an observable result.

## 4. Validate documentation

- [x] 4.1 Review the updated guide against the delta spec and all supplied board photos and datasheets; verify no instruction contradicts a confirmed label, measurement, or locked architecture decision.
- [x] 4.2 Run OpenSpec validation for `harden-prototype-guide-safety-and-verification`; verify proposal, spec, design, and tasks are complete and the modified capability path is accepted.
- [x] 4.3 Perform a final safety-only read-through from a beginner's perspective; verify each powered action has a setup, expected result, and stop condition before the next connection.
