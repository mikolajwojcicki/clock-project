## Why

The thesis prototype is a soldered perfboard with no jumpers; the breadboard is bring-up only. A custom PCB is not required. Stage of `openspec/roadmap.md`. Needs: stages 3, 4 (hardware frozen). Unblocks: stage 11.

## What Changes

- Draw a KiCad schematic of the locked circuit: MDBT42Q module, e-paper header, LIS3DH, DCF receiver, MCP73831, AP2112K, buzzer stage, SWD. Run ERC.
- Solder it on perfboard and bring it up with the stage 3 and 4 firmware.
- Export print-safe schematic blocks for the thesis.

## Capabilities

### New Capabilities
- `perfboard-prototype`: circuit contents, ERC cleanliness and bring-up criteria.

### Modified Capabilities
<!-- none -->

## Impact

- `hardware/kicad/`, `hardware/docs/`, `hardware/bom/`, `notes/log/`, `notes/media/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 5.1 of the archived `plan-thesis-roadmap` change.
