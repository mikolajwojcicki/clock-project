# VeroRoute part mapping (perfboard shield)

VeroRoute **2.40** (`nix-shell -p veroroute --run veroroute`). Import: click
**Delete All Aliases**, then File → Import Netlist → OrcadPCB2 →
`hardware/veroroute/dk-perfboard-shield-layout.net`. Import confirmed OK on 2026-11-03.

## Why not the KiCad netlist

`hardware/kicad/dk-perfboard-shield.net` imports as all `$noname` and the DK (J1)
is one part. The layout netlist is derived from it by
`hardware/veroroute/make-layout-netlist.py` (re-run after any schematic change;
it asserts that all 21 J1 pins are used exactly once). Its Type column holds
VeroRoute import strings, so no manual alias is needed.

## Mapping

| Ref | VeroRoute type | Pins used | Note |
|---|---|---|---|
| P1 | `BLOCK_100MIL8` | 4 `+3V3`, 5 `5V`, 6 `GND` | J1 split into 4 DK header strips |
| P2 | `BLOCK_100MIL6` | 1–6 | |
| P3 | `BLOCK_100MIL8` | 3–8 | |
| P4 | `BLOCK_100MIL10` | 3–8 | |
| J2 (e-paper), J3 (LIS3DH), J4 (DCF), J5 (Amigo VBAT) | `BLOCK_100MILn` | per netlist | socket / header strips; J3 pins 2, 9 unused |
| BZ1 | `BLOCK_100MIL4` | 1 `5V`, 4 `BZ1-` | pins 2, 3 unused; 8.21 mm pitch fits the holes |
| SW5 | `BLOCK_100MIL3` | 2 `5V`, 3 `VBAT` | single pole as 3-pin block |
| SW1–SW4 | `SWITCH_BUTTON_4PIN` | 1, 2 | |
| Q1 | `TO92` | 1 `GND`, 2 base, 3 `BZ1-` | |
| D1 | `DIODE` | 1 `5V`, 2 `BZ1-` | |
| R1, R_SCK | `RESISTOR4` | 1, 2 | |
| C1, C3 | `CAP_CERAMIC` | 1, 2 | |
| C2 | `CAP_ELECTRO_200` | 1 `+3V3`, 2 `GND` | |

## Fallbacks used

- `SWITCH_ST` and `SWITCH_DT` fail import ("Part has no pins") → SW5 is a
  3-pin `BLOCK_100MIL3`; its unused second row (one hole, 2.54 mm away) is
  reserved in layout (task 4.3), no net.
- KiCad netlist unusable → derived layout netlist (see above).

## Still to confirm in the GUI (task 4.3, before routing)

1. `SWITCH_BUTTON_4PIN`: pins 1 and 2 must be the two *unshorted* contacts, else
   the button reads always-on.
2. `DIODE`: pin 1 = anode? D1 cathode must be on `5V`. Check the placed symbol's
   stripe; swap netlist pins if reversed.
3. `TO92`: confirm pin 1 = E, 2 = B, 3 = C against the placed footprint.
4. DK header pin order is inferred (Arduino R3, pin 1 at the USB end); verify
   against the DK silkscreen before soldering.
