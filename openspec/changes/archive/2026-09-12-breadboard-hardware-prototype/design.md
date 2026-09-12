## Context

The repository already has an owned prototype kit listed in `hardware/bom/proto-bom.md`. The kit uses an nRF52 DK (`PCA10040`) as the controller, not the production Raytac module. The existing first firmware app is a SoftDevice-free blinky built with nRF5 SDK 17.1.0 and flashed with OpenOCD.

The guide must support a solderless breadboard build with modules that can have different connector labels or pin orders. The selected hardware is the Waveshare 2.13inch e-Paper HAT, Rev 2.1, the Adafruit LIS3DH breakout from product 2809, and the DCF-1060N-800 DCF77 receiver described by the local manual dump in `temp-resources/docs/dcf77/`. It must preserve the locked architecture: e-paper display, LIS3DH motion sensor, DCF-77 receiver, NPN-driven electromagnetic buzzer, and bare-metal nRF52832 firmware.

## Goals / Non-Goals

**Goals:**

- Give a person with no electronics experience one safe path from parts check to subsystem tests.
- Separate firmware, power, wiring, and peripheral faults through staged bring-up.
- Record a stable prototype pin map and assumptions for later firmware and PCB work.
- Explain electrical safety in plain English, with stop conditions before risky actions.
- Keep the guide usable with the installed NixOS tools and existing repository workflow.

**Non-Goals:**

- Build the production PCB or replace the KiCad design.
- Implement display, LIS3DH, DCF-77, alarm, or BLE firmware in this change.
- Guarantee DCF-77 reception in every location, room, time, or orientation.
- Use the Li-Po battery as the default first power source.
- Introduce Zephyr, nRF Connect SDK Bare Metal, an RTOS, or new system packages.

## Decisions

### Use a staged assembly sequence

The guide will begin with a visual parts check, then validate the DK and blinky firmware, then add one peripheral at a time. The display, LIS3DH, DCF-77 receiver, and buzzer will each have a separate check before full integration.

This sequence is safer than wiring the complete circuit first because one failed test has a small search area. It also creates useful evidence for the thesis. A single all-at-once wiring diagram is not sufficient for beginners.

### Use USB power from the nRF52 DK first

The first build will use the DK USB connection and its 3.3 V rail for low-risk digital bring-up. The TP4056 and Li-Po path will be optional and documented as a later power experiment.

This avoids charging and battery polarity risks during first assembly. The guide will state the current and voltage limits that must be checked against the actual modules before external power is connected.

### Define a project-owned pin map

The guide will select conservative nRF52 DK GPIO assignments for the prototype and will reserve safe pins for display SPI. It will keep P0.22 through P0.30 away from display SPI and PWM, in line with the module constraints.

The exact pin map will appear in one table and in each relevant wiring step. The guide will mark any signal that depends on a module revision as an assumption that the user must confirm from the module label or documentation.

### Provide text diagrams and physical orientation rules

The guide will use text diagrams, pin tables, color suggestions for wires, breadboard row references, and connector orientation checks. It will describe how to identify power rails and how to test rail continuity without assuming prior breadboard experience.

Photos are not required for the first planning artifact. The implementation guide will use the confirmed product identities and instruct the builder to compare physical labels before applying power.

### Test observable behavior, not only continuity

Each stage will have a visible, audible, or measurable expected result. DCF-77 testing will include environmental limitations such as antenna orientation, distance from interference, and wait time. A missing radio frame will remain a conditional result until power, wiring, and receiver output are checked.

## Risks / Trade-offs

- [Module pin labels differ] → Require a label and voltage check before wiring. Mark uncertain connector mappings as assumptions and stop the user from guessing.
- [DK 3.3 V rail cannot supply all loads] → Start with USB-powered digital tests, state rail limits, and isolate the buzzer and battery experiments.
- [Buzzer causes resets or noise] → Use the transistor driver and flyback diode, test it last, and document separate power or grounding checks.
- [E-paper module remains blank] → Check power, reset, busy, chip select, and SPI wiring separately. Explain that e-paper can retain an image after power removal.
- [DCF-77 signal is unavailable] → Document reception conditions and allow the rest of the prototype to pass without a successful radio frame.
- [Breadboard wiring is changed without recording it] → Include a wiring-difference field and a final pin-map record in the guide.

## Migration Plan

1. Create the detailed guide under `hardware/docs/`.
2. Validate every part and pin reference against the BOM, DK documentation, module labels, and the chosen firmware test plan.
3. Assemble and test the prototype in stages.
4. Record measured results and any wiring changes in the guide or its test record.
5. Use the confirmed pin map for later firmware drivers and PCB review.

Rollback requires removing the new guide and prototype-specific records. The change does not alter production hardware, firmware, or system configuration.

## Confirmed Hardware References

- The display is the Waveshare 2.13inch e-Paper HAT, Rev 2.1. Its documented signals are VCC, GND, DIN, CLK, CS, DC, RST, and BUSY. The display uses SPI and accepts 3.3 V or 5 V input through its onboard voltage translator.
- The motion board is the Adafruit LIS3DH breakout from product 2809. It includes a 3.3 V regulator, level shifting, I2C and SPI support, interrupt outputs, and standard header pins.
- The DCF-77 board is the Drhomeam DCF-1060N-800 module described by the local manual dump. Its documented pins are PON, OUT, GND, and VDD. The manual states a 1.1 V to 3.3 V supply range. The guide will treat PON polarity and the actual output voltage as values to confirm from the board label and measured signal before connecting the nRF52 GPIO.
