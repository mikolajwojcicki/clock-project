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
| P1 | `SIP8` | 4 `+3V3`, 5 `5V`, 6 `GND` | J1 split into 4 DK header strips |
| P2 | `SIP6` | 1–6 | |
| P3 | `SIP8` | 3–8 | |
| P4 | `SIP10` | 3–8 | |
| J4 (DCF), J5 (Amigo VBAT) | `SIPn` | per netlist | socket strips |
| J2A (HAT header, odd RPi pins 1..39) | `SIP20` | 6 `EPD_RST` (pin 11), 10 `SPI_MOSI` (19), 12 `SPI_SCK` (23) | block pin i = RPi pin 2i−1 |
| J2B (HAT header, even RPi pins 2..40) | `SIP20` | 1 `+3V3` (pin 2), 3 `GND` (6), 9 `EPD_BUSY` (18), 11 `EPD_DC` (22), 12 `EPD_CS` (24) | block pin i = RPi pin 2i; place 1 hole from J2A, same columns |
| J3A (LIS3DH bottom row) | `SIP6` | 1 `+3V3` (VIN), 3 `GND`, 4 `SPI_SCK`, 5 `SPI_MOSI`, 6 `SENSOR_INT1` | row VIN 3Vo GND SCL SDA INT; 3Vo open |
| J3B (LIS3DH top row) | `SIP6` | 5 `SPI_MISO` (SDO), 6 `SENSOR_CS` | row I2 A3 A2 A1 SDO CS; place 6 holes (15.24 mm) from J3A, same columns, silkscreen upright |
| BZ1 | `SIP4` | 1 `5V`, 4 `BZ1-` | pins 2, 3 unused; 8.21 mm pitch fits the holes |
| SW5 | `SIP3` | 2 `5V`, 3 `VBAT` | single pole as 3-pin block |
| SW1–SW4 | `SWITCH_BUTTON_4PIN` | 1, 2 | |
| Q1 | `TO92` | 1 `GND`, 2 base, 3 `BZ1-` | |
| D1 | `DIODE` | 1 `5V`, 2 `BZ1-` | |
| R1, R_SCK | `RESISTOR4` | 1, 2 | |
| C1, C3 | `CAP_CERAMIC` | 1, 2 | |
| C2 | `CAP_ELECTRO_200` | 1 `+3V3`, 2 `GND` | |

## Fallbacks used

- `SWITCH_ST` and `SWITCH_DT` fail import ("Part has no pins") → SW5 is a
  3-pin `SIP3`; its unused second row (one hole, 2.54 mm away) is
  reserved in layout (task 4.3), no net.
- KiCad netlist unusable → derived layout netlist (see above).

## Checked against sources (2026-11-03)

| Item | Source | Result |
|---|---|---|
| DK P1–P4 pin order | PCA10040 v3.0.0 `PCA10040_Schematic_And_PCB.pdf` sheet 5 + `400150_Pick_And_Place.csv` (P1/P2 at 0°, P3/P4 at 180°) | Matches `HEADERS` in the generator. Block pin 1 = low-x end; on P3/P4 that is Nordic pin 8/10 (D7, SCL) |
| J4 DCF order PON OUT GND VDD | `temp-resources/docs/dcf77/` vendor pinout photo | Matches J4 pins 1–4 |
| J2 Waveshare order VCC GND DIN CLK CS DC RST BUSY | `temp-resources/IMG_4276/4277.jpeg`: 8-wire Waveshare cable (grey, brown, blue, yellow, orange, green, white, purple), no 9th PWR wire | Matches J2 pins 1–8 |
| Q1 2N3904 TO-92 | `temp-resources/2N3904_datasheet.pdf` p. 1: `EBC` | Matches symbol 1 = E, 2 = B, 3 = C |
| J3 LIS3DH rows | author read the board silkscreen | Split into J3A/J3B (see table) |

## Still to confirm in the GUI (task 4.3, before routing)

1. `SWITCH_BUTTON_4PIN`: pins 1 and 2 must be the two *unshorted* contacts, else
   the button reads always-on.
2. `DIODE`: pin 1 = anode? D1 cathode must be on `5V`. Check the placed symbol's
   stripe; swap netlist pins if reversed.
3. `TO92`: the VeroRoute footprint's pin 1 must sit where the part's E lead goes
   (flat face toward you, E on the left).
4. One DK silkscreen glance before soldering: P0.03 must be the header pin at the
   USB-side end of P2 (the GPIO labels are printed next to the headers).

## Revision 2026-11-03 (later)

`BLOCK_100MILx` imports but draws each pin as a pill 3 holes tall, so a header
blocks 3 rows (seen in the VeroRoute screenshot). Connectors and sockets now use
`PADSx` (plain pads, 1 hole each; string found in the 2.40 binary). Not yet
re-imported; if `PADSx` is rejected, try `SIP` or fall back to `BLOCK_100MILx`.

`PADSx` imported as individual loose pads (no block to move). The binary has
`AddSIP` and `BreakSIPSintoPADS`, so PADS are broken-up SIPs: connectors are now
`SIPx`. Not yet re-imported.

Confirmed 2026-11-03: `SIPx` imports as one 1-row block per header, one hole per
pin. This is the working mapping (BLOCK_100MILx and PADSx are superseded).
