# Tasks

## 1. Promote shield schematic

- [x] 1.1 Copy `dk-perfboard-shield.{kicad_pro,kicad_sch,kicad_pcb,kicad_prl}`, `dk-perfboard-shield-symbols.kicad_sym` and `sym-lib-table` from `scratch/perfboard-routing/kicad-shield/` into `hardware/kicad/`, then fix library paths so the project opens from its new location; leave `breadboard-prototype.*` untouched
- [x] 1.2 Set the title block to `v1.0-shield` with the current date; make the sheet note name USB bring-up mode and battery mode separately and state the DK USB + battery dual-feed ban
- [x] 1.3 Audit the netlist against `firmware/boards/dk_breadboard_pins.h`: every signal on its spec pin, no net on `P0.05`–`P0.08` or `P0.17`–`P0.20`, no MDBT42Q/AP2112K/MCP73831/USB-C parts
- [x] 1.4 Audit the power and passive rules: `R_SCK` 100 Ω between `P0.03` and the bus, no button pull-ups, BZ1+ and D1 cathode on `5V` (not `3V3`), SW5 on `VBAT`→`5V`, C1/C2/C3 fitted, R2/R3 DNP
- [x] 1.5 Run `kicad-cli sch erc` → `hardware/kicad/erc-dk-perfboard-shield.rpt` with 0 errors; list each remaining warning with its reason in `hardware/kicad/README.md` (or the sheet notes)
- [x] 1.6 Export the OrcadPCB2 `.net` and the PDF with `kicad-cli` into `hardware/kicad/`; commit the schematic, exports and ERC report as one `hardware:` commit

## 2. Fix stale hardware docs

- [x] 2.1 Update `hardware/docs/perfboard-shield.md`: Amigo PIM611 replaces TP4056, slide switch on BAT→`5V` (not peripheral `3V3`), no fitted button pull-ups, link to the new schematic
- [x] 2.2 Update `hardware/bom/proto-bom.md`: Amigo PIM611, Akyga cell, MSS-2245, `R_SCK`; TP4056 marked as an unused spare; 10k resistors marked as spares; commit the docs separately from the schematic

## 3. Header test-fit

- [ ] 3.1 Push unsoldered long-pin female (stacking) headers through the perfboard so their tails go into DK `P1`–`P4`. P1/P2/P3 share the perfboard grid; P4 pins are 1.016 mm off it, so pre-bend only the P4 tails 1.016 mm sideways just below the board underside (toward P3) and confirm all four mate; if the bend fails, use a separate P4 strip or two boards. Also check whether the DK `P20` 1×8 pin list (4 holes below P4) touches the perfboard underside: if it blocks seating, leave the tails a few mm short of full insertion or cut a notch over the keep-out. Record the result and a photo in the dated bring-up log

## 4. VeroRoute layout

- [ ] 4.1 In `nix-shell -p veroroute`, create a pad-per-hole board of about 28×36 holes and import `hardware/kicad/dk-perfboard-shield.net` (OrcadPCB2)
- [ ] 4.2 Map Part Aliases (J1 split into P1–P4 PADSn, sockets, TO-92, diode polarity, SW5 single pole); record every mapping, the VeroRoute version and any fallback in `hardware/docs/perfboard-veroroute-aliases.md`
- [ ] 4.3 Place parts in the design D5 order (headers at the DK geometry from 3.1, module sockets, `R_SCK` next to `P0.03`, buzzer cluster at the edge, then passives and switches); leave the DK `P20` keep-out empty (holes cols 7–15, rows 14–16, numbering as in design.md, no parts, no joints, no links)
- [ ] 4.4 Autoroute, then fix by hand using solder traces plus short insulated links; keep SPI away from `EPD_RST`/`EPD_BUSY`, the Q1 collector and the `5V` loop
- [ ] 4.5 Run the VeroRoute connectivity check until it shows 0 opens and 0 shorts; save `hardware/veroroute/dk-perfboard-shield.vrt` and commit

## 5. Build sheet

- [ ] 5.1 Export component-side and solder-side PDF/PNG from VeroRoute into `hardware/veroroute/`
- [ ] 5.2 Write `hardware/docs/perfboard-build-sheet.md`: a continuity checklist by KiCad net name in the order power, SPI, signals, buzzer; the dual-feed warning; and the "USB unplugged before SW5 on" step; commit

## 6. Solder and bare-board checks

- [ ] 6.1 Solder the shield from the build sheet (no plug-in wires); photograph both sides into `notes/media/YYYY-MM-DD_perfboard_*.png`
- [ ] 6.2 With no modules and no DK fitted, measure `3V3`–`GND`, `5V`–`GND` and `VBAT`–`GND` for shorts, then walk the continuity checklist; log the values in `notes/log/YYYY-MM-DD_perfboard-bringup.md`

## 7. USB-mode bring-up

- [ ] 7.1 Plug the shield onto the DK with no modules, power over USB, measure and log DK `VDD`
- [ ] 7.2 Fit the e-paper; run repeated full refreshes with stage 3/4 firmware; confirm there are no `EPD_BUSY` hangs; log
- [ ] 7.3 Fit the LIS3DH; confirm the ID read and `INT1` on motion; log
- [ ] 7.4 Fit the DCF module; confirm `DCF_OUT` pulses with `DCF_PON` enabled; log
- [ ] 7.5 Confirm SW1–SW4 read low when pressed and high when released, and the buzzer sounds on `BUZZER_EN`; log

## 8. Battery-mode bring-up

- [ ] 8.1 Unplug USB, connect Akyga → Amigo `BAT` → J5; with SW5 off, confirm 0 V on `P1` `5V` and BZ1+
- [ ] 8.2 Switch SW5 on; log the cell voltage, `P1` `5V` and DK `VDD`; repeat checks 7.2–7.5 and record `VDD` during the buzzer alarm (brown-out risk); commit the bring-up log and photos as a separate commit

## 9. Thesis schematic blocks

- [x] 9.1 Export the schematic as black-and-white SVG/PDF and crop the power path, SPI + e-paper, buzzer stage and sensors + DCF blocks into `notes/media/YYYY-MM-DD_perfboard_sch-<block>.svg`
- [x] 9.2 Print or preview each block in grayscale at thesis width; confirm refs, net names and pin numbers are readable; commit
