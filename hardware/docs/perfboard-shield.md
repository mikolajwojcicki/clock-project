# Perfboard shield for the nRF52 DK

Pin map for the soldered prototype. The perfboard plugs onto the nRF52 DK
(`PCA10040`) and carries every peripheral. The nets are the same as in
[`breadboard-wiring-table.md`](breadboard-wiring-table.md). Pin names match
`firmware/boards/dk_breadboard_pins.h`, so the firmware does not change.
Source schematic: [`hardware/kicad/dk-perfboard-shield.kicad_sch`](../kicad/dk-perfboard-shield.kicad_sch)
(PDF and netlist beside it).

Bench results from the breadboard build that the layout MUST respect:

- Power the display from a short, direct `3V3`/`GND` path. Long jumpers made
  it fail on 2026-10-18.
- Keep `SPI_SCK` and `SPI_MOSI` short and away from `EPD_RST` and `EPD_BUSY`.
- Fit a 100 Ω series resistor on `SPI_SCK` at the DK pin (`P0.03`). Without it,
  every refresh hangs (`notes/log/2026-10-22_epd-sck-ringing.md`).

## Shield connector to DK header

| DK header | Shield pins used | Nets |
| --- | --- | --- |
| `P1` (power) | `VDD`, `GND`, `5V` | `3V3`, `GND`, `5V` (battery mode: switched `5V` feeds the DK and the buzzer) |
| `P2` | `P0.03`, `P0.04`, `P0.28`, `P0.29`, `P0.30`, `P0.31` | `SPI_SCK`, `SPI_MOSI`, `EPD_RST`, `EPD_BUSY`, `SENSOR_CS`, `BUZZER_EN` |
| `P3` | `P0.11`, `P0.12`, `P0.13` to `P0.16` | `EPD_CS`, `EPD_DC`, `SW1` to `SW4` |
| `P4` | `P0.02`, `P0.23`, `P0.24`, `P0.25` | `SPI_MISO`, `SENSOR_INT1`, `DCF_PON`, `DCF_OUT` |

`P0.22` stays free (the clock app uses it as a timing marker; leave a test
pad). Do not use `P0.05` to `P0.08`; they belong to the DK virtual COM port.
Header assignment checked against Figure 10 of the nRF52 DK User Guide v3.x
(`temp-resources/docs/nrf/`): `P3` carries `P0.11` to `P0.18`, `P4` carries
`P0.02` and `P0.22` to `P0.27`, `P2` carries `P0.03`, `P0.04`, `P0.28` to
`P0.31`. `P0.17` to `P0.20` are the DK LEDs; leave them unused.

## Buttons

`SW1` to `SW4` (MODE, UP, DOWN, SNOOZE) go from `P0.13` to `P0.16` to `GND`
through tactile switches. The DK already has its own buttons on these pins
with the pull-ups, so the firmware keeps the internal pull-up and the shield
adds no resistor. Press = low. The 10 kΩ resistors from the BOM are spares
for this reason.

## On-shield parts

| Part | Connects to | Note |
| --- | --- | --- |
| Waveshare 2.13" HAT Rev2.1 on a 2x20 male header (RPi layout) | `3V3` → pin 2, `GND` → 6, `SPI_MOSI` → 19, `SPI_SCK` → 23, `EPD_CS` → 24, `EPD_DC` → 22, `EPD_RST` → 11, `EPD_BUSY` → 18 | Pins 2/4 are the HAT's power input (`RPI_5V` net) and get `3V3`, never `5V`. Pins 1/17 power nothing on the HAT. Leave every other pin open. Only parts lower than the header stack under the HAT outline (65 × 30.2 mm). Source: `waveshare-hat-40pin.md` |
| LIS3DH breakout socket | `3V3`, `GND`, `SPI_SCK`, `SPI_MOSI`, `SPI_MISO`, `SENSOR_CS`, `SENSOR_INT1` | `INT2`, `3Vo` open |
| DCF-1060N-800 socket | `3V3`, `GND`, `DCF_PON`, `DCF_OUT` | `PON` active low |
| `R_SCK` 100 Ω | in series on `SPI_SCK`, next to `P0.03` | 47 to 220 Ω ok |
| Buzzer stage | `P0.31` to 1 kΩ to `2N3904` base; emitter `GND`; buzzer `+` on `5V`, `-` on collector; `1N4148` across the buzzer | Check transistor pin order |
| `C1` 100 nF + `C2` 10 µF | `3V3` to `GND`, next to the display and DCF sockets | Local decoupling |
| `C3` 100 nF | `3V3` to `GND`, next to the LIS3DH socket | |
| `SW5` slide switch `MSS-2245` | `VBAT` (from `J5`) to the switched `5V` net: DK `P1` `5V` and buzzer `+` | One pole used. Never on the DK `VDD` pin or the peripheral `3V3` rail |
| `J5` 2-pin (`Amigo_VBAT`) | Amigo `BAT`/`VBAT` and `GND` to `SW5` and shield `GND` | Battery mode only; charger stays off-board on the Amigo |
| Four tactile buttons | see Buttons | |

## Layout rules

1. Put the display socket next to the `P2` header end so `3V3`, `GND` and SPI
   are short.
2. Route the SPI wires as one bundle on one side. Keep `EPD_RST` and
   `EPD_BUSY` on the other side, spaced apart.
3. Keep the buzzer `5V` and collector loop away from the SPI wires.
4. Mount with M3 standoffs to the DK holes. Leave the DK `nRF current
   measurement` header and the PPK2 connection accessible.

## Check before first power-up

Same order and checks as section 6 of the wiring table: `GND`/`3V3`, then SPI,
then signals, then the buzzer stage. In USB mode power the shield only from the
DK `VDD` pin (battery path unconnected). Use the PPK2 on the DK measurement
header exactly as in the baseline measurement.

## Power modes

1. **USB bring-up:** DK USB powers `VDD`. `J5` empty, `SW5` off. Do not attach
   the battery to `P1` `5V`.
2. **Battery mode:** USB unplugged. Akyga cell to Amigo `PIM611` JST, Amigo
   `BAT`/`VBAT` pads to `J5`, then `SW5` (`VBAT` to `5V`) to DK `P1` `5V` and
   the buzzer `+`. The Amigo `5V` pads are USB VBUS only and are not a battery
   rail. Leave the Amigo Fast Charge pads open.

Never dual-feed: DK USB and the battery path must not supply the DK at the same
time. Unplug USB before closing `SW5`.
