## Why

The prototype guide is broadly correct, but several instructions still require
the beginner to infer electrical details from incomplete evidence. That creates
avoidable risks: reversed power, an incorrectly configured e-paper SPI mode,
unsafe multimeter use, an undefined DCF power-control state, and buzzer
overcurrent or flyback damage. The recent board photos and DCF measurements
provide enough evidence to replace those assumptions with explicit checks and
stop conditions before more parts are connected.

## What Changes

- Add a pre-power safety procedure for the KORAD KKG305D, multimeter current
  measurement, current limiting, rail checks, and current-meter wiring.
- Record the verified DCF facts: labeled pin identity, active-low `PON`,
  `68.3 µA` enabled current, and `0.0 µA` disabled current.
- Require the user to verify the Waveshare Rev2.1 `BS1` setting for four-wire
  SPI before connecting the display.
- Make all module-revision and pin-label checks explicit, with a hard stop when
  a physical board does not match the documented board.
- Add safe default states for DCF `PON`, display control signals, sensor chip
  select, and buzzer control before firmware drives them.
- Add a measured-output requirement before connecting DCF `OUT` to an nRF52
  GPIO, and distinguish a multimeter pulse indication from a logic-level
  measurement.
- Tighten buzzer checks for the exact buzzer rating, transistor package
  pinout, diode direction, supply voltage, and short-duration testing.
- Update the completion checklist and test record so safety checks and
  evidence are recorded, not assumed.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `breadboard-hardware-prototype`: require evidence-based safety checks,
  unambiguous power and signal wiring, safe instrument use, module-revision
  verification, and explicit stop conditions before prototype integration.

## Impact

- `hardware/docs/breadboard-prototype-guide.md`
- `openspec/specs/breadboard-hardware-prototype/spec.md`
- Possibly `hardware/bom/proto-bom.md` if safety evidence or item status needs
  clarification.
- No firmware, PCB, architecture, or external dependency changes.
