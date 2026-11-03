## Why

The thesis prototype is a soldered perfboard shield on the nRF52 DK, with no jumper wires; the breadboard is bring-up only. The project moved from a custom PCB to this shield; the custom PCB (MDBT42Q, discrete MCP73831, AP2112K) stays optional in `hardware/bom/pcb-bom.md`. Stage of `openspec/roadmap.md`. Needs: stages 3, 4 (hardware frozen). Unblocks: stage 11.

## What Changes

- Promote the shield schematic from `scratch/perfboard-routing/kicad-shield/` (v0.3, ERC 0 errors) into `hardware/kicad/`: DK `P1`–`P4` headers, sockets for e-paper, LIS3DH and DCF modules, `R_SCK` 100 Ω, SW1–SW4, buzzer stage on switched `5V`, Akyga → LiPo Amigo `BAT` → MSS-2245 → DK `P1` `5V`. Keep the breadboard schematic as an archive.
- Lay out the shield in VeroRoute from the exported OrcadPCB2 netlist, check connectivity, export a build sheet.
- Solder it and bring it up with the stage 3 and 4 firmware in USB and battery mode.
- Export print-safe schematic blocks for the thesis.
- Fix stale docs that still name TP4056 or a switch on peripheral `3V3` (`hardware/bom/proto-bom.md`, `hardware/docs/perfboard-shield.md`).

## Capabilities

### New Capabilities
- `perfboard-prototype`: DK shield circuit, ERC cleanliness, netlist-checked layout, and bring-up criteria.

### Modified Capabilities
<!-- none -->

## Impact

- `hardware/kicad/`, `hardware/docs/`, `hardware/bom/`, `notes/log/`, `notes/media/`.
- Locked architecture unchanged (bare-metal nRF52832, nRF5 SDK 17.1.0, no Zephyr or RTOS).
- Source: Roadmap task 5.1 of the archived `plan-thesis-roadmap` change.
