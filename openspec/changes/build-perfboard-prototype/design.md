# Design

## Context

See proposal.md for why; requirements are in `specs/perfboard-prototype/spec.md`.

Current state:

- `hardware/kicad/breadboard-prototype.*` (v1.1, 2026-09-13) is a breadboard sketch with the buzzer on `+3V3` and no `R_SCK`, buttons or battery path.
- `scratch/perfboard-routing/kicad-shield/dk-perfboard-shield.*` (v0.3, 2026-11-03) fixes those gaps: `R_SCK`, SW1–SW5, `5V` buzzer rail, J5 `Amigo_VBAT`, R2/R3 DNP. ERC reports 0 errors and 2 warnings (isolated stubs, 2N3904 library drift). OrcadPCB2 `.net` and PDF are exported. The tree is a nested scratch repo, so it does not count as tracked monorepo evidence.
- `hardware/docs/perfboard-shield.md` and `hardware/bom/proto-bom.md` still say TP4056, a switch on peripheral `3V3`, and 10k button pull-ups.
- DK socket geometry is known from the Nordic pick-and-place file: P1→P2 Δx 20.32 mm, bottom→top Δy 48.26 mm, P4→P3 uses the Arduino 0.06" stagger.
- VeroRoute 2.40 runs from nixpkgs (`nix-shell -p veroroute`). It is GUI-only, and nets become numbers after import.

## Goals / Non-Goals

**Goals:**
- Make one schematic the source of truth for the shield, with the netlist exported from it so layout never copies nets by hand.
- Make one tracked layout file with a connectivity check before any solder touches the board.
- Keep evidence the thesis can cite: ERC report, PDFs, build sheets, bring-up log, photos.

**Non-Goals:**
- Custom PCB copper, DRC, or fab exports.
- Adding the Python maze router in `scratch/perfboard-routing/router/` to the repo or using it as the layout source.
- Firmware changes. Stage 3 and 4 firmware is used as it is. A pin-map mismatch counts as a schematic bug.
- Drawing the Amigo charger circuit. The Amigo stays an off-board module behind J5.

## Decisions

### D1. Promote the scratch v0.3 shield as a new KiCad project
Copy `dk-perfboard-shield.{kicad_pro,kicad_sch,kicad_pcb,kicad_prl}`, `dk-perfboard-shield-symbols.kicad_sym` and `sym-lib-table` into `hardware/kicad/`. Keep the breadboard files there untouched as the archive. Bump the title block to `v1.0-shield` and today's date.
- *Alternative:* edit `breadboard-prototype.*` in place. Rejected because it loses the breadboard-era record the chronology chapter cites, and the spec requires that record to stay unchanged.
- *Alternative:* leave the schematic in `scratch/`. Rejected because scratch is a separate ignored repo, so the thesis could not cite tracked history.

### D2. Keep the DK as one schematic symbol; split it into P1–P4 only in layout
The KiCad `DK_PCA10040` symbol stays one block, since that is the readable form for thesis figures. In VeroRoute, J1 is mapped to four header parts (PADS8 / PADS6 / PADS10 / PADS8) placed at the Nordic socket coordinates.
- *Alternative:* four header symbols in KiCad. Rejected because it makes the schematic harder to read, with no electrical gain. The refs in the OrcadPCB2 netlist are enough for aliasing.

### D3. VeroRoute is the layout source of truth, in pad-per-hole mode
Import the netlist with `File → Import Netlist → OrcadPCB2`. Record each Part Alias in `hardware/docs/perfboard-veroroute-aliases.md` (promoted from `scratch/.../aliases.md` and filled in after the first import), so the import can be repeated.
- *Alternatives:* LochCAD, boardwright, perfstudio. Rejected as too young for a graded prototype (see `scratch/perfboard-routing/RESEARCH.md`).
- *Alternative:* KiCad PCB with freerouting. Rejected because its copper model is wrong for this board and it has no notion of a hole-to-hole link.
- *Alternative:* the custom A* maze. Rejected because the demo run left many nets unrouted without rip-up.

### D4. Wiring style: solder traces plus short insulated links
The user chose this on 2026-11-03. Pad-to-pad solder or bare-wire traces go on the solder side. Soldered insulated wire links are allowed where a trace cannot cross. Plug-in or removable wires are banned. This fits the spec ("all connections soldered, no removable jumper wires") and gives the router room on a 28×36-hole board.
- *Alternative:* solder bridges only. Rejected because the board is too dense and would likely need a larger board.

### D5. Placement order and SI rules are human-enforced
Placement order:
1. Headers at the fixed DK geometry.
2. Module sockets.
3. `R_SCK` on the hole next to `P0.03`.
4. Buzzer cluster (Q1, D1, R1, BZ1) at the board edge, away from P2/P4 SPI pins.
5. Passives, then switches.

Then run the autorouter, then hand-fix routes so SPI does not run beside `EPD_RST`/`EPD_BUSY`, the Q1 collector, or the `5V` loop. VeroRoute's autorouter has no cost map, so the review is a manual checklist step.

### D6. Artefact locations

| Artefact | Path |
| --- | --- |
| Schematic, symbols, OrcadPCB2 `.net`, PDF, ERC report | `hardware/kicad/dk-perfboard-shield.*`, `hardware/kicad/erc-dk-perfboard-shield.rpt` |
| VeroRoute layout | `hardware/veroroute/dk-perfboard-shield.vrt` |
| Build sheets (component side, solder side) + continuity checklist | `hardware/veroroute/` PDF/PNG + `hardware/docs/perfboard-build-sheet.md` |
| Alias table | `hardware/docs/perfboard-veroroute-aliases.md` |
| Bring-up log | `notes/log/YYYY-MM-DD_perfboard-bringup.md` |
| Photos, thesis schematic blocks | `notes/media/YYYY-MM-DD_perfboard_<detail>.png` (light theme, grayscale-checked) |

Every export is regenerated with `kicad-cli` (`sch erc`, `sch export netlist --format orcadpcb2`, `sch export pdf`/`svg`), so the outputs can be rebuilt from the schematic.

### D7. Thesis schematic blocks are exported from the same schematic
Export the full sheet as SVG and PDF in black-and-white mode. Crop blocks (power path, SPI + e-paper, buzzer stage, sensors + DCF) as vector images. There are no separate drawings for the thesis, so figures cannot drift from the built circuit.

### D8. Bring-up is staged, and each stage gates the next
1. Bare board, no modules: short checks on `3V3`, `5V` and `VBAT` to `GND`, plus a continuity checklist.
2. USB mode: plug onto the DK with no modules, measure `VDD`.
3. Fit modules one at a time: e-paper, LIS3DH, DCF, buzzer.
4. Battery mode with USB unplugged, cell voltage logged.

Each stage is logged before the next starts. A failure stops the bring-up and goes back to schematic or layout, not to firmware.

### D9. Stale docs are fixed in the same change, in separate commits
Update `perfboard-shield.md` and `proto-bom.md`:
- Amigo PIM611 replaces TP4056.
- The switch sits on BAT→`5V`, not on peripheral `3V3`.
- There are no fitted button pull-ups; the 10k resistors are spares.

Point both docs at the new schematic. Split commits so that schematic, layout, docs and log land separately.

## Risks / Trade-offs

- **The shield does not mate because P4 is off the perfboard grid.** From the Nordic pick-and-place centers, P1, P2 and P3 pin rows sit on one 2.54 mm grid. P4 (1×10) pins sit 0.4 hole (1.016 mm) off it, which is the Arduino 0.06" stagger. → Decision 2026-11-03: bend the P4 male-header tails 1.016 mm so the upper pins stay on the DK grid and the tails land in the perfboard holes (tails on hole 7, upper pins on 7.4, counting from the P3 origin; P3 starts at hole 18). Test-fit unsoldered headers on the DK before any routing is committed. In VeroRoute, P4 is a normal on-grid PADS10 at the tail position. Fallbacks if the bend fails: separate P4 strip with insulated links, or two boards.
- **DK `P20` pins touch the perfboard underside.** `P20` is a 1×8 SMD pin list on the DK top (Nordic pick-and-place: x 18.8–36.6 mm, y 45.71 mm), four holes below the P4 row and under its first 8 pins, with the same 0.4-hole offset as P4. User reported on 2026-11-03 that it interferes. → Keep holes cols 7–15, rows 14–16 (hole numbers: columns from x = 0 mm of the DK, so P3 starts at col 18; rows counted up from the P1/P2 row = 0, P3/P4 row = 19) empty so nothing sits on the pins. If the board still does not seat, use taller male headers or cut a notch over that zone. Decision between those two waits for the test-fit.
- **Part Aliases fail on import (custom footprints).** → Use `PADSn` for every connector and socket and record each mapping. If import is still blocked, place generic parts by hand and link pins with the connectivity view, and record that fallback in the alias doc.
- **Nets show as numbers in VeroRoute, so a wrong pin is easy to miss.** → Check the continuity checklist against the KiCad netlist by net name, not by VeroRoute number.
- **SPI ringing comes back on longer shield traces.** → Keep `R_SCK` at the pin, and bring a scope or logic analyser to the first e-paper refresh. The `EPD_BUSY` hang is the known symptom.
- **Dual-feed damage (USB plus battery).** → The schematic sheet note, the build sheet warning, and the log step "USB unplugged" are all required before closing SW5.
- **Buzzer current on the cell sags `VDD` during an alarm.** → Accept this for the prototype. Measure it during battery bring-up and record it as thesis evidence. Mitigate only if `VDD` drops below the nRF brown-out.
- **Two ERC warnings remain.** → List them with their reasons beside the report. They are not errors, and silencing them would hide library drift.

## Migration Plan

Hardware has no deploy step. Rollback means going back to the breadboard. The breadboard schematic and wiring table stay valid, and the shield is socketed, so modules move back without desoldering. The breadboard files are retired only when the shield passes battery-mode bring-up.

## Open Questions

- Exact VeroRoute alias names for the LIS3DH 9-pin and Waveshare 8-pin sockets. These are found during the first import and do not change the approach.
- Whether the P13–P17 through-hole duplicates give an easier mating option than the P1–P4 sockets. This is decided at the header test-fit and does not change the specs.
