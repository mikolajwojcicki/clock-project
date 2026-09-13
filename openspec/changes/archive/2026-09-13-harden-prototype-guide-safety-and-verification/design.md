## Context

The existing guide already uses staged bring-up, shared 3.3 V and ground rails,
safe nRF52 GPIO choices, and a transistor buzzer driver. Recent evidence narrows
the physical hardware: the display is a Waveshare 2.13inch HAT Rev2.1, the
accelerometer is the photographed Adafruit LIS3DH breakout, and the DCF board
has labeled `PON`, `OUT`, `GND`, and `VDD` pads. A controlled test with the
KORAD KKG305D measured 68.3 µA with `PON = GND` and 0.0 µA with `PON = VDD`.

## Goals / Non-Goals

**Goals:**

- Make the guide safe to follow without guessing pin order, board revision,
  supply state, meter mode, or control-pin polarity.
- Separate confirmed facts, provisional facts, and facts that require suitable
  measurement equipment.
- Prevent accidental damage during power-up, current measurement, DCF
  connection, display SPI setup, and buzzer testing.
- Preserve the existing nRF5 SDK bare-metal firmware and locked hardware
  architecture.

**Non-Goals:**

- Do not create peripheral firmware drivers.
- Do not change the PCB architecture or production BOM.
- Do not require a logic analyzer or oscilloscope for initial power testing.
- Do not claim successful DCF time decoding from pulse activity alone.

## Decisions

### Use evidence gates before integration

The guide will separate identity, power, and signal checks. A peripheral can
only advance when its supply and ground are verified, its control pins have
known safe states, and its signal voltage is within the receiving device's
limits. This is safer than treating a matching product name as proof that every
board revision is wired identically.

### Treat the DCF receiver as active-low power control

The verified DCF test becomes the guide's known behavior: `PON = GND` enables
the receiver and `PON = VDD` disables it. The measured current values are
recorded as evidence, not as a universal specification for every unbranded
module. `OUT` remains gated until a suitable instrument measures its logic
levels.

### Add a dedicated instrument-safety gate

The KORAD supply provides a current limit and the multimeter provides the
low-current reading. The guide will state their separate roles and show the
series-current path. It will explicitly forbid placing a current-mode meter
across the supply. This prevents the most likely beginner measurement fault.

### Require explicit e-paper SPI mode verification

The HAT photograph shows a revision-specific `BS1` selection. The guide will
require the four-wire SPI setting before the shared SPI wiring is connected.
This avoids a silent display failure caused by assuming the connector alone
selects the protocol.

### Keep unknowns as stop conditions

The exact buzzer electrical rating, actual transistor package pinout, and DCF
`OUT` high-level waveform remain evidence-dependent. The guide will not invent
values. It will require the corresponding part marking or measurement before
the relevant powered test.

## Risks / Trade-offs

- [Risk] A multimeter may not resolve an 85 µA-class current accurately in every
  range. → Use the KORAD current limit for protection, record the meter range,
  and treat the reading as a prototype observation.
- [Risk] DCF pulse activity can be mistaken for a safe MCU logic level. →
  Require oscilloscope or logic-analyzer evidence before MCU connection.
- [Risk] HAT or breakout revisions may differ from the photographed boards. →
  Require label and revision checks at the workbench.
- [Risk] Added checks make first bring-up slower. → Keep the checklist staged
  and limit each gate to one observable result.
- [Risk] The buzzer may draw more current than expected. → Keep it transistor
  driven, use short tests, verify the diode, and require the exact rating.

## Migration Plan

1. Update the guide's preparation, pin maps, DCF, display, buzzer, fault
   isolation, checklist, and test-record sections.
2. Validate the guide and delta spec against the photographed hardware and
   measured DCF behavior.
3. Run a documentation-only review. Do not connect additional hardware as part
   of this change.
4. If a later logic-level measurement changes the DCF conclusion, update the
   guide's evidence status before wiring `OUT` to the nRF52.

