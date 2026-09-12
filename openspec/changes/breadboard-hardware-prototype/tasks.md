## 1. Confirm source material and guide structure

- [ ] 1.1 Create `hardware/docs/breadboard-prototype-guide.md` and verify that it is the only implementation document added by this change
- [ ] 1.2 Record the exact hardware identities: nRF52 DK `PCA10040`, Waveshare 2.13inch e-Paper HAT Rev 2.1, Adafruit LIS3DH product 2809, Drhomeam DCF-1060N-800, AP-1205V-P1 buzzer, transistor, diode, and resistor; verify each identity against `hardware/bom/proto-bom.md` and the supplied product documentation
- [ ] 1.3 Add a beginner glossary for voltage, ground, GPIO, SPI, I2C, interrupt, breadboard rail, pull-up, and flyback diode; verify that each term is defined before first technical use
- [ ] 1.4 Add a tools and materials checklist, ownership status, optional-part markers, and software prerequisites; verify that the checklist covers every item in the prototype BOM

## 2. Write safety and preparation instructions

- [ ] 2.1 Add USB-power-first instructions and warnings against connecting the Li-Po or TP4056 during first bring-up; verify that the guide names the safe first power source
- [ ] 2.2 Add rules for removing USB power before changing wires, checking for shorts, handling the antenna, and responding to heat or smell; verify that every hazard has an immediate stop action
- [ ] 2.3 Explain breadboard rows, split power rails, wire colors, module orientation, and how to identify VCC and GND before assembly; verify that a beginner can prepare the work area without guessing
- [ ] 2.4 Add a pre-power inspection checklist for reversed power, adjacent-row shorts, unconnected grounds, and loose jumpers; verify that the checklist appears before the first power step

## 3. Define and document the wiring plan

- [ ] 3.1 Select and document one nRF52 DK pin map for display SPI, display control, LIS3DH communication and interrupt, DCF-77 output and power control, and buzzer control; verify that display SPI uses P0.11 through P0.20 and avoids P0.22 through P0.30
- [ ] 3.2 Add complete signal tables for the Waveshare VCC, GND, DIN, CLK, CS, DC, RST, and BUSY pins; verify that each signal has one DK destination and one purpose
- [ ] 3.3 Add complete signal tables for the Adafruit LIS3DH power, communication, interrupt, and address-selection pins; verify that unused interface pins are marked rather than left ambiguous
- [ ] 3.4 Add complete signal tables for the DCF-1060N-800 PON, OUT, GND, and VDD pins; verify that the guide cites the local manual dump and requires output-voltage and PON-polarity confirmation before GPIO connection
- [ ] 3.5 Add the NPN buzzer-driver wiring with the 1 kOhm base resistor and 1N4148 flyback diode polarity; verify that the buzzer has no direct GPIO connection
- [ ] 3.6 Add text diagrams and wire-by-wire breadboard instructions for each subsystem; verify that every wire names source, destination, signal, and expected voltage
- [ ] 3.7 Add a wiring-difference record table; verify that users can record changed pins, changed module labels, and reasons without editing the guide structure

## 4. Document staged firmware and hardware tests

- [ ] 4.1 Document how to build and flash `firmware/apps/blinky/` with the existing NixOS tools; verify the commands match the existing README and Makefile
- [ ] 4.2 Define the controller-only test and its expected LED behavior; verify that the guide tells the user to stop if blinky fails
- [ ] 4.3 Define the display test, including power, SPI, reset, busy, chip-select, and retained-image checks; verify that blank-display troubleshooting distinguishes a stale e-paper image from a failed refresh
- [ ] 4.4 Define the LIS3DH communication and interrupt test; verify that the expected result identifies both sensor response and motion interrupt behavior
- [ ] 4.5 Define the DCF-77 power, output, antenna placement, orientation, interference, and wait-time test; verify that no received frame is documented as an environmental limitation until wiring and supply checks pass
- [ ] 4.6 Define the buzzer-driver test; verify that the test checks transistor operation, resistor placement, diode polarity, sound output, and unwanted DK reset behavior
- [ ] 4.7 Define the full-prototype test order and pass criteria; verify that each stage names power state, action, expected result, and stop condition

## 5. Add fault isolation and completion records

- [ ] 5.1 Add fault tables for no power, hot parts, failed flash, blank display, false or missing LIS3DH interrupt, missing DCF-77 data, and silent buzzer; verify that each fault returns to the last known-good stage
- [ ] 5.2 Add a safe teardown procedure; verify that it removes USB power before wiring changes and separates optional battery experiments from the first prototype
- [ ] 5.3 Add a completion checklist for controller, display, motion, DCF-77, buzzer, and integrated tests; verify that each capability from the specification has a checklist item
- [ ] 5.4 Add a reproducible test record with hardware revisions, pin map, firmware commit, test conditions, observed results, and unresolved faults; verify that the record can document a partial success

## 6. Review the delivered guide

- [ ] 6.1 Review every electrical value, pin label, polarity rule, and power warning against the selected product documentation and local DCF-77 manual; verify that no instruction depends on an unmarked guess
- [ ] 6.2 Perform a dry-run review in which a beginner can follow the guide in order without missing a tool, wire, check, or stop condition; record and correct each ambiguity
- [ ] 6.3 Apply the simple-English rules to the guide; verify short active sentences, defined terms, no unexplained abbreviations, and no instructions that ask the reader to guess
- [ ] 6.4 Run `openspec validate --change "breadboard-hardware-prototype" --strict` and verify that the change artifacts pass validation
