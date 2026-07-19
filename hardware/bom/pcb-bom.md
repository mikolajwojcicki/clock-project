# Production PCB BOM

Master bill of materials for the custom alarm-clock PCB. Prefer **TME.pl** for Poland next-day sourcing; MPNs also usable on Mouser/DigiKey.

Locked product choices: repo-root [`AGENTS.md`](../../AGENTS.md). Proto / breadboard kit: [`proto-bom.md`](proto-bom.md).

## Assembly warnings

1. **`LIS3DHTR` (`U1`)** — LGA-16 (3×3 mm), pads under the package only. Needs solder paste + stencil + hot air / reflow plate; iron-only assembly is impractical.
2. **Raytac `MDBT42Q` (`M1`)** — castellated SMT pads; hand-solderable with fine tip, flux, and care.
3. **Passives** — prefer **0603** (and **0805** where noted) for a balance of density and student hand-assembly.
4. **Datasheets** — verify KiCad footprints and pin maps against manufacturer drawings before fab.

## Section 1 — Active ICs & modules

| Item | Description | Ref | Mfr | MPN | Package | TME |
| --- | --- | --- | --- | --- | --- | --- |
| 1.1 | nRF52832 BLE module (chip antenna) | `M1` | Raytac | `MDBT42Q-512KV2` | castellated SMT | `MDBT42Q-512KV2` |
| 1.2 | 3-axis accelerometer, ULP | `U1` | ST | `LIS3DHTR` | LGA-16 3×3 mm | `LIS3DHTR` |
| 1.3 | 3.3 V LDO, 600 mA, low Iq | `U2` | Diodes | `AP2112K-3.3TRG1` | SOT-23-5 | `AP2112K-3.3TRG1` |
| 1.4 | Single-cell Li-Po charger (4.2 V) | `U3` | Microchip | `MCP73831T-2ACI/OT` | SOT-23-5 | `MCP73831T-2ACI/OT` |
| 1.5 | NPN buzzer driver | `Q1` | ON / ST | `MMBT3904` (or S8050) | SOT-23 | `MMBT3904-ON` |

## Section 2 — External modules & electromechanical UI

| Item | Description | Ref | MPN / source | Interface | Notes |
| --- | --- | --- | --- | --- | --- |
| 2.1 | Waveshare **2.13"** e-Paper module | `DISP1` | Waveshare 2.13inch e-Paper Module | SPI: VCC, GND, DIN, CLK, CS, D/C, RST, BUSY | **Locked Option 2:** 1×8 2.54 mm header (not raw FPC). Do not substitute 1.54" without updating AGENTS.md. |
| 2.2 | DCF-77 77.5 kHz receiver + antenna | `DCF1` | Generic (Allegro / Conrad) | VCC, GND, PON, DATA | 1×4 2.54 mm header; power-gate / PD when idle |
| 2.3 | Electromagnetic buzzer ~2400 Hz | `BZ1` | `AP-1205V-P1` | THT 12 mm, 6.5 mm pitch | Option A (loud); ~50 mA while sounding |
| 2.4 | SMT tactile buttons | `SW1`–`SW4` | Diptronics `DTSM-61N-V-T/R` | SMD 6×6 mm | MODE, UP, DOWN, SNOOZE (TME) |

## Section 3 — Resistors, diode, LED

Resistors: **0603**, 1%, 0.1 W unless noted.

| Item | Value | Ref | Purpose | Package |
| --- | --- | --- | --- | --- |
| 3.1 | 5.1 kΩ | `R1`, `R2` | USB-C CC1/CC2 pull-downs (5 V request) | 0603 |
| 3.2 | 5.0 kΩ | `R3` | MCP73831 `RPROG` (~200 mA charge) | 0603 |
| 3.3 | 1.0 kΩ | `R4` | `Q1` base limiter | 0603 |
| 3.4 | 470 Ω | `R5` | Charge-status LED limiter | 0603 |
| 3.5 | — | `D1` | `1N4148W` flyback across buzzer | SOD-123 |
| 3.6 | Red LED | `LED1` | MCP73831 STAT | 0603 |

## Section 4 — Capacitors & inductors

Voltage ratings: input caps ≥ 10 V; VCC/output caps ≥ 6.3 V.

| Item | Value / type | Ref | Purpose | Package | Notes |
| --- | --- | --- | --- | --- | --- |
| 4.1 | 4.7 µF ceramic | `C1`, `C2` | MCP73831 VDD & VBAT bypass | 0805 or 0603 | X5R/X7R, ≥ 10 V |
| 4.2 | 1.0 µF ceramic | `C3`–`C5` | AP2112 in/out + nRF DCC (`C14` in Raytac notes) | 0603 | X5R/X7R, ≥ 6.3 V |
| 4.3 | 0.1 µF ceramic | `C6`–`C9` | Decoupling MCU / `U1` | 0603 | X7R, ≥ 10 V |
| 4.4 | 12 pF ceramic | `C10`, `C11` | Load caps for `X1` (32.768 kHz) | 0603 | C0G/NP0, 50 V |
| 4.5 | 10 µH | `L1` | nRF DC-DC (`L2` in Raytac schematic) | 0805 | e.g. `LQM21PN100MGRD` |
| 4.6 | 15 nH | `L2` | nRF DC-DC (`L3` in Raytac schematic) | 0402 | e.g. `LQG15HS15NJ02D` |

## Section 5 — Clocks & connectors

| Item | Description | Ref | MPN / type | Form | Notes |
| --- | --- | --- | --- | --- | --- |
| 5.1 | 32.768 kHz crystal (~9 pF CL) | `X1` | e.g. Citizen `MC-306 32.7680K-A0` or Epson `ST3215SB32768B0HPWBB` | 3.2×1.5 mm SMD | Internal RTC clock |
| 5.2 | USB-C 6-pin receptacle (power only) | `J1` | GCT `USB4105-GF-A` | SMT + shield legs | Charge input |
| 5.3 | JST-PH 2.0 mm 2-pin battery | `J2` | JST `B2B-PH-K-S(LF)(SN)` | THT vertical | Single-cell Li-Po |
| 5.4 | 1×8 2.54 mm header | `J3` | generic | THT | E-paper module |
| 5.5 | 1×4 2.54 mm header | `J4` | generic | THT | DCF-77 module |
| 5.6 | 1×4 2.54 mm header | `J5` | generic | THT | SWD: VDD, GND, SWDIO, SWDCLK |

## Review notes (vs locked architecture)

- Matches AGENTS.md actives: MDBT42Q, LIS3DH, AP2112K-3.3, MCP73831, AP-1205V-P1, Waveshare 2.13" header module, DCF-77, SWD.
- Original LLM text allowed “2.13 or 1.54” — **this file locks 2.13"** only.
- Refdes `L1`/`L2` here map to Raytac’s `L2`/`L3` naming (noted in section 4).
- Exact GPIO netlist / pin map is still TBD (schematic phase).
