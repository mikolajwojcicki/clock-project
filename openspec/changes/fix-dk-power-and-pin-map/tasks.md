# Tasks

## 1. DK power and ground identification

- [x] 1.1 In guide section 2 "Power rules", replace "DK 3.3 V pin" wording with "DK `P1` pin `VDD`" and add the sentence mapping DK `VDD` to the breadboard `3V3` rail; verify `rg -n "DK 3.3 V pin|DK \`3V3\`" hardware/docs/breadboard-prototype-guide.md` returns no matches
- [x] 1.2 Add a "DK power pins" subsection with two tables (pins to use: `P1 VDD`, `P1 GND`; pins never to use: `5V`, `External supply` P21, `nRF current measurement` P22, `VDD_nRF`, coin cell, Debug out) citing DK User Guide v3.x.x sections 4.3–4.4; verify every row has a label, location, and reason
- [x] 1.3 Add the DK revision check (`PCA10040`, sticker 3.x.x, stop on mismatch) to the stop conditions; verify it appears before section 5
- [x] 1.4 Update section 5.2 rail table and section 8 steps to "DK `P1` pin `VDD`" / "DK `P1` pin `GND`", add the measured-rail step with stop on 0 V, unstable, or above 3.6 V, and explain the D1 diode drop; verify section 8 has the measurement step and stop condition

## 2. DK pin map

- [x] 2.1 Replace the section 6 pin-map table with the new map from design.md (pin, DK header, project name, device, purpose) and add a note that it supersedes the old map; verify every signal in the table appears exactly once and no row uses `P0.00`, `P0.01`, `P0.05`–`P0.10`, `P0.13`–`P0.21`, `P0.26`, or `P0.27`
- [x] 2.2 Add a "Reserved DK pins" table (pin, DK function, source section) and replace the old "unused pins" sentence; state the `P0.22`–`P0.31` low-frequency rule from nRF52832 PS v1.9 section 4.3.1; verify the table lists all reserved pins from design.md
- [x] 2.3 Update the display (section 9), LIS3DH (section 10), DCF (section 11), and buzzer (section 12) pin tables and numbered wiring steps to the new pins; verify `rg -n "P0\.(1[3-9]|2[01])" hardware/docs/breadboard-prototype-guide.md` returns only lines in the reserved-pin table and the old-map migration note
- [x] 2.4 Update section 6.1 safe control states and section 13 integration order to use new pin numbers where pins are named; verify by reading both sections

## 3. Checklist, record, and references

- [x] 3.1 Add checklist items "DK revision checked", "breadboard rail fed from `P1 VDD`", "old-map wires removed", and record fields "DK version sticker", "measured `P1 VDD` voltage"; verify items appear in sections 16 and 17
- [x] 3.2 Add the nRF52 DK User Guide v3.x.x and nRF52832 Product Specification v1.9 (paths under `temp-resources/docs/nrf/`) to section 18 references; verify both paths exist with `ls`
- [x] 3.3 Commit the guide changes as `hardware: clarify DK power pins and remap prototype GPIOs`; verify `git show --stat HEAD` lists only the guide

## 4. KiCad schematic

- [x] 4.1 Rename DK symbol pins in `hardware/kicad/breadboard-prototype-symbols.kicad_sym` to `P1_VDD`, `P1_GND`, and `P0.xx_SIGNAL` with the new numbers; verify with `rg -n "name \"P" hardware/kicad/breadboard-prototype-symbols.kicad_sym`
- [x] 4.2 Apply the same names to the embedded DK symbol in `hardware/kicad/breadboard-prototype.kicad_sch` and update the schematic safety text that names `P0.25`/DK `3V3`; verify `kicad-cli sch export pdf` succeeds
- [x] 4.3 Run `kicad-cli sch erc` on the schematic and confirm no new connectivity errors compared with before the change; record the result
- [x] 4.4 Export `hardware/kicad/breadboard-prototype.pdf`, view it, and confirm DK pin names match the guide table; commit as `hardware: match schematic DK pins to updated guide`

## 5. Integration check

- [x] 5.1 Cross-check the guide pin map, wiring steps, and schematic PDF signal by signal and confirm all three agree; run `openspec validate fix-dk-power-and-pin-map --strict` and confirm it passes
