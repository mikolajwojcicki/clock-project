# Handoff: breadboard bring-up hardware verification

Stopped: 2026-10-04 ~19:45. Session verified tasks on the nRF52 DK with a
Saleae Logic (8 ch) through the `Saleae Logic 2` MCP server, plus the UART log.

## Where we are

- 14 of 16 tasks in `tasks.md` are done. Open: **4.3** (DCF pulses on hardware)
  and **6.1** (full integration run). Then `/opsx-archive` this change.
- Firmware on the DK: bring-up app with the 250 kHz SPI clock and the LIS3DH
  boot reset (latest `main`, flashed through VS Code task `firmware: bringup flash`).
- Commits are local only; `main` is ahead of `origin`. Do not push without
  asking the user.

## Why 4.3 stopped

The DCF-77 module (Drhomeam DCF-1060N-800, pads `PON`, `OUT`, `GND`, `VDD`) has
no wires soldered yet. The user must solder leads before the next session.

## Next steps

1. **4.3, stage A.** USB off. `VDD` to DK `3V3` (never `5V`), `GND` to `GND`,
   `PON` to `P0.24` (firmware holds it high = receiver off), `OUT` to Saleae
   **CH7 only** (move CH7 off `P0.29`). Antenna away from the laptop and USB.
   Start a capture, press button 3, expect ~1 pulse/s on CH7 (100 ms = 0,
   200 ms = 1, a missing pulse at the minute mark). The Saleae model cannot
   measure voltage; `OUT` is safe because the module runs from 3.3 V.
2. **4.3, stage B.** USB off, connect `OUT` to DK `P0.25` as well. Run button 3
   for several minutes; the UART log shows per-pulse lines and the final
   `valid / invalid / minute_markers` counts. Zero valid pulses is an
   acceptable recorded observation (location, antenna orientation, time).
3. **6.1.** With everything wired (guide section 13 step 6), press RESET, then
   run buttons 1, 2, 3, 4 in order. Each must end with its result line, and
   CH0 to CH3 must show the safe states between tests.
4. Tick the tasks, commit (thesis-style bodies, see `.cursor/rules/`), then archive.

## Current wiring on the breadboard

| Module | Connections |
| --- | --- |
| E-paper (Waveshare 2.13" V4) | `VCC` 3V3, `GND`, `DIN` P0.04, `CLK` P0.03, `CS` P0.11, `DC` P0.12, `RST` P0.28, `BUSY` P0.29 |
| LIS3DH (Adafruit) | `VIN` 3V3, `GND`, `SCL` P0.03, `SDA` P0.04, `SDO` P0.02 (header spot labeled `AREF` on some DKs), `CS` P0.30, `INT1` P0.23 |
| Buzzer (5 V active) | 2N3904 low side: E to GND, B through 1 kΩ to P0.31, C to buzzer −; buzzer + to DK **5V**; 1N4148 band on buzzer + |
| DCF-77 | Not connected |

Saleae: GND on DK GND; CH0 P0.24, CH1 P0.11, CH2 P0.30, CH3 P0.31, CH4 P0.03,
CH5 P0.04, CH6 P0.02, CH7 P0.29 (move to DCF `OUT` for 4.3).

## Findings from this session (all in commit bodies)

| Finding | Fix | Commit |
| --- | --- | --- |
| LIS3DH loop counted ~4 events per INT1 pulse (level check while INT1 stays high ~1 ODR period) | Count GPIOTE edges only | `974a7bf` |
| 1 MHz SPI over jumpers lost clock edges, garbled e-paper | SPI at 250 kHz | `4c5d370` |
| First refresh after reset garbled; LIS3DH runs I2C while CS high and disturbs SCK/MOSI after a reset | Read `WHO_AM_I` once at boot | `f13e003` |
| GPIOs float during reset (`BUZZER_EN` high 98 µs) | PCB pull resistors `R6`–`R10` in `hardware/bom/pcb-bom.md` | `2081120` |
| Buzzer silent on 3V3 (5 V active type, 4–7 V) | Buzzer + on DK 5V, guide updated | `c739bfa` |

## Tool notes for the next agent

- Saleae: device `830EF3B3CBB01FDB`, no configurable threshold (omit
  `digitalThresholdVolts`), 12 MS/s works. Raw CSV export can reach GBs when a
  line is noisy: export only the channels you need (`logicChannels`). Export to
  the home directory; the `context-mode` sandbox cannot see the host `/tmp`.
- The SPI decoder (`add_analyzer` "SPI", CPOL 0 / CPHA 0, enable CH1) plus
  `export_data_table_csv` lets you compare the e-paper frame byte by byte.
- The agent Shell tool stopped responding late in the session; the `git` MCP
  server and the user's VS Code tasks covered commits and flashing.
- Do not `pkill -f "cat /dev/ttyACM0"`: it matches the calling shell. Kill by PID.
- Before editing `tasks.md`, re-read it: an earlier stale-buffer edit undid six
  checkboxes (restored in the commit that adds this note).
