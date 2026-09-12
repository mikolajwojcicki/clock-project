## Why

The project needs a safe first hardware build before custom PCB work. A beginner-friendly guide will reduce wiring errors and provide repeatable checks for the nRF52 DK, display, motion sensor, DCF-77 receiver, and alarm buzzer.

## What Changes

- Create an extremely detailed Markdown assembly guide for the breadboard prototype.
- Define the parts, tools, power limits, safety warnings, and preparation steps.
- Define a beginner-safe wiring plan with named rails, pin tables, wire-by-wire instructions, and text diagrams.
- Explain how to build and flash the existing bare-metal blinky firmware before connecting peripherals.
- Explain staged tests for power, display, motion interrupt, DCF-77 signal, buzzer driver, and full prototype integration.
- Document expected results, common mistakes, fault isolation, cleanup, and safe teardown.
- Record assumptions where module pin labels or board revisions can differ.

## Capabilities

### New Capabilities

- `breadboard-hardware-prototype`: Assemble and validate the first alarm-clock hardware prototype on a breadboard using the nRF52 DK and owned peripheral modules.

### Modified Capabilities

No existing capabilities require modification.

## Impact

- Adds a detailed hardware guide under `hardware/docs/`.
- Uses the existing `hardware/bom/proto-bom.md` parts list and `firmware/apps/blinky/` build and flash workflow.
- Establishes a documented prototype pin map that future firmware drivers and PCB decisions can reference.
- Does not change production PCB files, firmware behavior, dependencies, or the locked product architecture.
