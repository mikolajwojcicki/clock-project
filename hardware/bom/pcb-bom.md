# Production PCB BOM

Master bill of materials for the custom alarm-clock PCB. Prefer **TME.pl** for Poland next-day sourcing; MPNs also usable on Mouser/DigiKey.

Locked product choices: repo-root [`AGENTS.md`](../../AGENTS.md). Proto / breadboard kit: [`proto-bom.md`](proto-bom.md).

## Assembly warnings

1. **`LIS3DHTR` (`U1`):** LGA-16 (3×3 mm), pads under the package only. Needs solder paste + stencil + hot air or reflow plate. Iron-only assembly is impractical.
2. **Raytac `MDBT42Q` (`M1`):** castellated SMT pads. Hand soldering is possible with a fine tip and flux.
3. **Passives:** Prefer **0603** and **0805** where noted for student hand assembly.
4. **Datasheets:** Verify KiCad footprints and pin maps against manufacturer drawings before fabrication.

## Section 1: Active ICs and modules

| Item | Description | Ref | Mfr | MPN | Package | Status | Source |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1.1 | nRF52832 BLE module (chip antenna) | `M1` | Raytac | `MDBT42Q-512KV2` | castellated SMT | Locked | `AGENTS.md`, project architecture |
| 1.2 | 3-axis accelerometer, ULP | `U1` | ST | `LIS3DHTR` | LGA-16 3×3 mm | Locked | `AGENTS.md`, project architecture |
| 1.3 | 3.3 V LDO, 600 mA, low Iq | `U2` | Diodes | `AP2112K-3.3TRG1` | SOT-23-5 | Locked | `AGENTS.md`, project architecture |
| 1.4 | Single-cell Li-Po charger (4.2 V) | `U3` | Microchip | `MCP73831T-2ACI/OT` | SOT-23-5 | Locked | `AGENTS.md`, project architecture |
| 1.5 | NPN buzzer driver | `Q1` | ON / ST | `MMBT3904` (or S8050) | SOT-23 | Locked | `AGENTS.md`, project architecture |
| 1.6 | Lower-quiescent-current LDO alternative | `U2A` | Texas Instruments | `TPS7A0533PDBVR` | SOT-23-5 | Proposed | `temp-resources/shit-to-buy.md` |
| 1.7 | DCF receiver power switch alternative | `U4` | Texas Instruments | `TPS22917DBVR` | SOT-23-5 | Proposed | `temp-resources/shit-to-buy.md` |

## Section 2: External modules and electromechanical UI

| Item | Description | Ref | MPN / source | Interface | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 2.1 | Waveshare **2.13"** e-Paper module | `DISP1` | Waveshare 2.13inch e-Paper Module | SPI: VCC, GND, DIN, CLK, CS, D/C, RST, BUSY | Locked | **Option 2:** 1×8 2.54 mm header (not raw FPC). Do not substitute 1.54" without updating `AGENTS.md`. |
| 2.2 | DCF-77 77.5 kHz receiver + antenna | `DCF1` | Generic (Allegro / Conrad) | VCC, GND, PON, DATA | Pending verification | 1×4 2.54 mm header. Confirm exact module output and power-down behavior. |
| 2.3 | Electromagnetic buzzer ~2400 Hz | `BZ1` | `AP-1205V-P1` | THT 12 mm, 6.5 mm pitch | Locked | Option A. Verify voltage and drive requirement before final PCB release. |
| 2.4 | SMT tactile buttons | `SW1`–`SW4` | Diptronics `DTSM-61N-V-T/R` | SMD 6×6 mm | Proposed | MODE, UP, DOWN, SNOOZE. Verify footprint and availability. |

## Section 3: Resistors, diode, and LED

Resistors: **0603**, 1%, 0.1 W unless noted.

| Item | Value | Ref | Purpose | Package | Status | Source |
| --- | --- | --- | --- | --- | --- | --- |
| 3.1 | 5.1 kΩ | `R1`, `R2` | USB-C CC1/CC2 pull-downs (5 V request) | 0603 | Pending verification | USB-C design candidate |
| 3.2 | TBD, 100 mA candidate | `R3` | MCP73831 `RPROG` | 0603 | Pending verification | Battery charge-rate review |
| 3.3 | 1.0 kΩ | `R4` | `Q1` base limiter | 0603 | Pending verification | Buzzer driver design |
| 3.4 | 470 Ω | `R5` | Charge-status LED limiter | 0603 | Pending verification | Charger indicator design |
| 3.5 | Not applicable | `D1` | `1N4148W` flyback across buzzer | SOD-123 | Pending verification | Buzzer driver design |
| 3.6 | Red LED | `LED1` | MCP73831 STAT | 0603 | Pending verification | Charger indicator design |
| 3.7 | 100 kΩ | `R6` | `SPI_SCK` pull-down: no clock edges while the MCU is in reset | 0603 | Pending verification | DK bring-up, see review notes |
| 3.8 | 100 kΩ | `R7`, `R8` | `EPD_CS` and `SENSOR_CS` pull-ups: both devices deselected during reset | 0603 | Pending verification | DK bring-up, see review notes |
| 3.9 | 100 kΩ | `R9` | `Q1` base pull-down: buzzer off while `BUZZER_EN` floats | 0603 | Pending verification | DK bring-up, see review notes |
| 3.10 | 10 kΩ | `R10` | DCF `PON` pull-up: receiver off during reset | 0603 | Pending verification | Prototype BOM R2; confirm PON polarity of the chosen module |

## Section 4: Capacitors and inductors

Voltage ratings: input caps ≥ 10 V; VCC/output caps ≥ 6.3 V.

| Item | Value / type | Ref | Purpose | Package | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 4.1 | 4.7 µF ceramic | `C1`, `C2` | MCP73831 VDD and VBAT bypass | 0805 or 0603 | Pending verification | X5R/X7R, ≥ 10 V |
| 4.2 | 1.0 µF ceramic | `C3`–`C5` | AP2112 in/out and nRF DCC (`C14` in Raytac notes) | 0603 | Pending verification | X5R/X7R, ≥ 6.3 V |
| 4.3 | 0.1 µF ceramic | `C6`–`C9` | Decoupling MCU and `U1` | 0603 | Pending verification | X7R, ≥ 10 V |
| 4.4 | 12 pF ceramic | `C10`, `C11` | Load caps for `X1` (32.768 kHz) | 0603 | Pending verification | C0G/NP0, 50 V |
| 4.5 | 10 µH | `L1` | nRF DC-DC (`L2` in Raytac schematic) | 0805 | Pending verification | Example only: `LQM21PN100MGRD` |
| 4.6 | 15 nH | `L2` | nRF DC-DC (`L3` in Raytac schematic) | 0402 | Pending verification | Example only: `LQG15HS15NJ02D` |

## Section 5: Clocks and connectors

| Item | Description | Ref | MPN / type | Form | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 5.1 | 32.768 kHz crystal (~9 pF CL) | `X1` | Citizen `MC-306 32.7680K-A0` or Epson `ST3215SB32768B0HPWBB` | 3.2×1.5 mm SMD | Pending verification | Internal RTC clock. Confirm load capacitance. |
| 5.2 | USB-C 6-pin receptacle (power only) | `J1` | GCT `USB4105-GF-A` | SMT + shield legs | Pending verification | Charge input. Verify footprint. |
| 5.3 | JST-PH 2.0 mm 2-pin battery | `J2` | JST `B2B-PH-K-S(LF)(SN)` | THT vertical | Pending verification | Single-cell Li-Po. |
| 5.4 | 1×8 2.54 mm header | `J3` | Generic | THT | Locked | E-paper module interface. |
| 5.5 | 1×4 2.54 mm header | `J4` | Generic | THT | Pending verification | DCF-77 module interface. |
| 5.6 | 1×4 2.54 mm header | `J5` | Generic | THT | Locked | SWD: VDD, GND, SWDIO, SWDCLK. |

## Review notes (vs locked architecture)

- Matches AGENTS.md actives: MDBT42Q, LIS3DH, AP2112K-3.3, MCP73831, AP-1205V-P1, Waveshare 2.13" header module, DCF-77, SWD.
- Original LLM text allowed “2.13 or 1.54”. **This file locks 2.13"** only.
- Refdes `L1`/`L2` here map to Raytac’s `L2`/`L3` naming (noted in section 4).
- Exact GPIO netlist / pin map is still TBD (schematic phase).
- `TPS7A0533PDBVR` is a proposed AP2112K alternative, not an approved replacement.
- `TPS22917DBVR` is a proposed DCF power switch. Confirm the receiver's power-down
  behavior before selecting it.
- The AP-1205V-P1 voltage and drive requirement remain pending verification.
- `R3` is not final. Check the MCP73831 charge current against the selected
  one-cell battery before choosing `RPROG`.
- `R6`–`R10` cover the window when firmware cannot drive pins (reset, SWD
  flashing, before `main()`); all nRF52 GPIOs float then. Evidence from DK
  bring-up (2026-10-04, Saleae): `BUZZER_EN` read high for 98 µs during
  reset, and after a reset the LIS3DH disturbed the shared SPI lines until its
  first SPI access, garbling the first e-paper refresh. The LIS3DH enables its
  I2C interface whenever `CS` is high (datasheet p. 9, 21, 24) on the pins we
  share as `SCK`/`MOSI`; `R6` keeps `SCK` low so it cannot see an I2C START.
  Firmware also reads `WHO_AM_I` once at boot (commit `f13e003`). Each pull
  sits at its line's idle level, so sleep leakage is near zero.

## Sources

- [Hardware and tool inventory](../inventory/prototype-tools.md)
- [Supplied purchase and component research](../../temp-resources/shit-to-buy.md)
- [Raytac MDBT42Q listing](https://www.tme.eu/pl/details/mdbt42q-512kv2/moduly-iot-wifi-bluetooth/raytac/)
- [MCP73831 listing](https://www.tme.eu/pl/details/mcp73831t-2aci_ot/kontrolery-baterii-i-akumulatorow-uklady/microchip-technology/)
- [Proposed TPS7A0533 listing](https://www.tme.eu/pl/details/tps7a0533pdbvr/stabilizatory-napiecia-nieregulowane-ldo/texas-instruments/)
- [Proposed TPS22917 listing](https://www.tme.eu/pl/details/tps22917dbvr/power-switches-uklady-scalone/texas-instruments/)
