# Perfboard shield for the nRF52 DK

Pin map for the soldered prototype. The perfboard plugs onto the nRF52 DK
(`PCA10040`) and carries every peripheral. The nets are the same as in
[`breadboard-wiring-table.md`](breadboard-wiring-table.md). Pin names match
`firmware/boards/dk_breadboard_pins.h`, so the firmware does not change.

Bench results from the breadboard build that the layout MUST respect:

- Power the display from a short, direct `3V3`/`GND` path. Long jumpers made
  it fail on 2026-10-18.
- Keep `SPI_SCK` and `SPI_MOSI` short and away from `EPD_RST` and `EPD_BUSY`.
- Fit a 100 Ω series resistor on `SPI_SCK` at the DK pin (`P0.03`). Without it,
  every refresh hangs (`notes/log/2026-10-22_epd-sck-ringing.md`).

## Shield connector to DK header

| DK header | Shield pins used | Nets |
| --- | --- | --- |
| `P1` (power) | `VDD`, `GND`, `5V` | `3V3`, `GND`, `5V` (buzzer only) |
| `P2` | `P0.03`, `P0.04`, `P0.28`, `P0.29`, `P0.30`, `P0.31` | `SPI_SCK`, `SPI_MOSI`, `EPD_RST`, `EPD_BUSY`, `SENSOR_CS`, `BUZZER_EN` |
| `P3` | `P0.11`, `P0.12`, `P0.13` to `P0.16` | `EPD_CS`, `EPD_DC`, `SW1` to `SW4` |
| `P4` | `P0.02`, `P0.23`, `P0.24`, `P0.25` | `SPI_MISO`, `SENSOR_INT1`, `DCF_PON`, `DCF_OUT` |

`P0.22` stays free (the clock app uses it as a timing marker; leave a test
pad). Do not use `P0.05` to `P0.08`; they belong to the DK virtual COM port.
Check the header assignment of `P0.13` to `P0.16` against the DK user guide
before you cut the long header sockets. `[INFERENCE]` I took it from the
wiring table for the other pins only.

## Buttons

`SW1` to `SW4` (MODE, UP, DOWN, SNOOZE) go from `P0.13` to `P0.16` to `GND`
through tactile switches. The DK already has its own buttons on these pins
with the pull-ups, so the firmware keeps the internal pull-up and the shield
adds no resistor. Press = low. The 10 kΩ resistors from the BOM are spares
for this reason.

## On-shield parts

| Part | Connects to | Note |
| --- | --- | --- |
| Waveshare 2.13" HAT, 1x8 socket | `3V3`, `GND`, `SPI_MOSI`, `SPI_SCK`, `EPD_CS`, `EPD_DC`, `EPD_RST`, `EPD_BUSY` | `BS1` bridge at `0` |
| LIS3DH breakout socket | `3V3`, `GND`, `SPI_SCK`, `SPI_MOSI`, `SPI_MISO`, `SENSOR_CS`, `SENSOR_INT1` | `INT2`, `3Vo` open |
| DCF-1060N-800 socket | `3V3`, `GND`, `DCF_PON`, `DCF_OUT` | `PON` active low |
| `R_SCK` 100 Ω | in series on `SPI_SCK`, next to `P0.03` | 47 to 220 Ω ok |
| Buzzer stage | `P0.31` to 1 kΩ to `2N3904` base; emitter `GND`; buzzer `+` on `5V`, `-` on collector; `1N4148` across the buzzer | Check transistor pin order |
| `C1` 100 nF + `C2` 10 µF | `3V3` to `GND`, next to the display and DCF sockets | Local decoupling |
| `C3` 100 nF | `3V3` to `GND`, next to the LIS3DH socket | |
| Slide switch | in series with `3V3` to the peripherals | Do not switch the DK `VDD` pin itself |
| JST-PH 2-pin | battery to the TP4056 board, not to the DK | Not used in first bring-up |
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
then signals, then the buzzer stage. Power the shield only from the DK `VDD`
pin. Use the PPK2 on the DK measurement header exactly as in the baseline
measurement.
