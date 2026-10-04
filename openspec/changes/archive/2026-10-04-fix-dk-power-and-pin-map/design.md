# Design

## Context

Sources read for this design (see proposal.md for motivation):

- `temp-resources/docs/nrf/nRF52_DK_User_Guide_v3.x.x.pdf`: section 4.3 (power
  supply), 4.4 (connector interface, Figure 10), 4.5 (buttons and LEDs),
  4.5.1 (I/O expander), 4.10 (solder bridges), 5 (current measurement).
- `temp-resources/docs/nrf/nRF52832_PS_v1.9.pdf`: section 4.3.1, Table 5
  (`P0.22`–`P0.31` are "low drive, low frequency I/O only", up to 10 kHz).
- `temp-resources/docs/nrf/nRF52_PDK_User_Guide_v1.2.1.pdf`: preview DK
  (`PCA10036`) only; used as a revision-mismatch fallback, not as a source.
- `firmware/apps/blinky/armgcc/Makefile`: defines `CONFIG_GPIO_AS_PINRESET`.

Facts from the DK guide that drive the design:

| DK item | Fact |
| --- | --- |
| `P1` header | Labels `VDD`, `VDD`, `RESET`, `VDD`, `5V`, `GND`, `GND` (Arduino: IOREF, RESET, 3.3V, 5V, GND, GND) |
| `VDD` rail | USB 5 V, then AP7333 3.3 V LDO (300 mA), then diode D1, then `VDD`; SB35 feeds `VDD` to the connectors |
| `External supply` (`P21`) | Power **input**, 1.7–3.6 V, not regulated, through diode D7 |
| `nRF current measurement` (`P22`) | Series point for nRF52832 current (SB9 cut); not a supply pin |
| `P0.13`–`P0.16` | Buttons 1–4, active low to GND, no external pull-up |
| `P0.17`–`P0.20` | LEDs 1–4, 220 Ω to `VDD`, active low; `P0.17` also I/O-expander `/INT` (SB30) |
| `P0.18` | Also routed to the interface MCU (SB27) |
| `P0.21` | nRF52832 reset when `CONFIG_GPIO_AS_PINRESET` is set |
| `P0.26`, `P0.27` | I/O-expander SDA and SCL |
| `P0.00`/`P0.01`, `P0.05`–`P0.08`, `P0.09`/`P0.10` | Crystal, UART to interface MCU, NFC |

## Goals / Non-Goals

**Goals:**

- One unambiguous answer to "which pin is 3.3 V and which is ground".
- A DK pin map with no on-board electrical load on any project signal.
- Guide, schematic symbol, and schematic PDF use the same pin names.

**Non-Goals:**

- No firmware drivers or pin defines; none exist yet.
- No change to the custom MDBT42Q PCB pin map in `AGENTS.md`.
- No solder-bridge modification on the DK (no cuts, no shorts).
- No change to the KORAD isolated DCF measurement procedure (section 4.1).

## Decisions

### Supply: `P1 VDD` and `P1 GND`

Use the `P1` header pin labeled `VDD` for the breadboard `3V3` rail and a `P1`
pin labeled `GND` for the ground rail. Any `VDD` pin on `P1` is acceptable
because they are one net; the guide names one so wiring photos match.

The breadboard rail keeps the name `3V3` so module tables stay unchanged, but
every connection step says "DK `P1` pin `VDD`". The guide states once: "The DK
calls this rail `VDD`. This guide calls the breadboard rail `3V3`."

Alternatives rejected:

- `External supply` (`P21`): it is an input. Driving it while USB is
  connected puts two sources on one rail.
- `5V`: wrong voltage for every module input.
- `VDD_nRF`: downstream of the current-measurement point; loads would be
  counted as nRF52832 current and break the later measurement chapter.

### Rail voltage is measured, not assumed

The diode D1 drop puts `VDD` below 3.3 V on USB power. The DK guide gives no
exact value, so the guide does not invent one. It requires a measurement,
records it, and stops on 0 V, unstable readings, or more than 3.6 V (the
External supply maximum and nRF52832 VDD limit).

### New DK pin map

| Signal | Old pin | New pin | DK header | Why |
| --- | --- | --- | --- | --- |
| `SPI_SCK` | `P0.13` | `P0.03` | `P2` | Fast signal; unrestricted pin; no DK load |
| `SPI_MOSI` | `P0.15` | `P0.04` | `P2` | Fast signal; unrestricted pin |
| `SPI_MISO` | `P0.14` | `P0.02` | `P4` | Fast signal; unrestricted pin |
| `EPD_CS` | `P0.17` | `P0.11` | `P3` | Can toggle per byte in Waveshare-style drivers |
| `EPD_DC` | `P0.18` | `P0.12` | `P3` | Can toggle per command byte |
| `EPD_RST` | `P0.19` | `P0.28` | `P2` | Slow reset pulse |
| `EPD_BUSY` | `P0.20` | `P0.29` | `P2` | Slow status input |
| `SENSOR_CS` | `P0.21` | `P0.30` | `P2` | Toggles per transaction, well under 10 kHz |
| `SENSOR_INT1` | `P0.23` | `P0.23` | `P4` | Unchanged |
| `DCF_PON` | `P0.24` | `P0.24` | `P4` | Unchanged |
| `DCF_OUT` | `P0.25` | `P0.25` | `P4` | Unchanged |
| `BUZZER_EN` | `P0.12` | `P0.31` | `P2` | Drives 1 kΩ base resistor; on/off or audio-rate drive stays under 10 kHz |
| spare | | `P0.22` | `P4` | Reserve for a second LIS3DH interrupt or later use |

Only five free pins (`P0.02`–`P0.04`, `P0.11`, `P0.12`) are both
unrestricted and free of DK hardware. The three SPI lines and the two display
lines that can toggle per byte get those five. All other signals are slow
and go to `P0.22`–`P0.31`, avoiding `P0.26`/`P0.27`.

Alternatives rejected:

- Keep `P0.13`–`P0.20` and cut SB5–SB8: removes LEDs only, not buttons, and
  modifies the DK. Blinky would also stop working.
- Enable the I/O expander to free `P0.13`–`P0.20`: needs `SHIELD DETECT`
  grounded or SB18 shorted, and then uses `P0.26`/`P0.27` and `P0.17`.
- Change `CONFIG_GPIO_AS_PINRESET` to free `P0.21`: loses the reset button
  and touches firmware outside this change.

### Reserved-pin table in the guide

The guide gets a table of reserved DK pins with their owner (crystal, UART,
NFC, button, LED, I/O expander, reset). This replaces the current sentence
that only lists unused pins.

### Schematic follows the guide

Rename DK symbol pins in `breadboard-prototype-symbols.kicad_sym`:
`3V3` → `P1_VDD`, `GND` → `P1_GND`, and each GPIO pin to `P0.xx_SIGNAL` with
the new number. Net labels keep signal names, so only the symbol and its
embedded copy in the schematic change. Re-export the PDF with `kicad-cli`.

## Risks / Trade-offs

- [User already wired the old map] → Guide marks the pin-map change at the top
  of section 6 and adds a checklist item "old map wiring removed".
- [Buzzer current from `VDD`] → LDO is 300 mA and shared with the DK; the
  guide keeps short buzzer tests and the existing current check.
- [Low-drive pins for `EPD_RST`, `EPD_BUSY`, `SENSOR_CS`, `BUZZER_EN`] → All
  loads are logic inputs or a 1 kΩ base resistor; standard-drive current
  (a few mA) is enough. Documented as a DK-only choice.
- [DK revision differs] → Revision check stops the user before wiring.
- [Old pin map remains in archived OpenSpec changes] → Archives are history;
  only the main spec and current docs change.

## Migration Plan

1. Remove USB power.
2. Remove any wires on the old pins (`P0.13`–`P0.21`).
3. Rewire using the new table.
4. Re-run section 7 (blinky) and section 8 (power check) before peripherals.

Rollback: revert the guide and schematic commits; no hardware is modified.
