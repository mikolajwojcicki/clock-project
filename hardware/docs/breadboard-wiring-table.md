# Breadboard wiring table

One row per wire of the breadboard prototype, with both ends named.
Use it as a checklist when you rebuild the board.
The explanation for each stage is in
[`breadboard-prototype-guide.md`](breadboard-prototype-guide.md).
Pin names match `firmware/boards/dk_breadboard_pins.h`.
Pin labels are the labels of the owned parts (Waveshare 2.13" HAT Rev 2.1,
Adafruit LIS3DH breakout, DCF-1060N-800). A different breakout can use other
labels. If a label does not match, stop and read the guide.

Classes:

- `power`: supply and ground.
- `bus`: one DK pin goes to more than one device.
- `signal`: one DK pin goes to one device pin.

Pins `P0.22` to `P0.31` are low-drive. They carry only signals up to 10 kHz
(marked "low-drive" below).

## 1. Power (class `power`)

| Net | From | To | Note |
| --- | --- | --- | --- |
| `3V3` | DK `P1 VDD` | `3V3` rail; display `VCC`, LIS3DH `VIN`, DCF `VDD` | Single source. Never to a `5V` pin of a module. The DK `VDD` pin is the 3.3 V output |
| `GND` | DK `P1 GND` | `GND` rail; display `GND`, LIS3DH `GND`, DCF `GND`, transistor `E` | Join split rail halves with a jumper |
| `5V` (buzzer only) | DK `P1 5V` | buzzer `+`, diode banded end | The owned buzzer is a 5 V active type and is silent on `3V3` (guide 12.2). Never to a GPIO or a 3.3 V input. Leave unwired until the buzzer stage |

LIS3DH `3Vo` and `INT2` stay unconnected.

## 2. Shared SPI bus (class `bus`)

The DK is the only source of the bus signals.

| Signal | DK pin (header) | Devices on the line | Note |
| --- | --- | --- | --- |
| `SPI_SCK` | `P0.03` (`P2`) | display `CLK`, LIS3DH `SCL` (`SCK`) | Fast signal |
| `SPI_MOSI` | `P0.04` (`P2`) | display `DIN`, LIS3DH `SDA` (`SDI`) | Never to `SPI_MISO` |
| `SPI_MISO` | `P0.02` (`P4`) | LIS3DH `SDO` | The display has no `MISO` |

## 3. Point-to-point signals (class `signal`)

| Signal | DK pin (header) | Device pin | Note |
| --- | --- | --- | --- |
| `EPD_CS` | `P0.11` (`P3`) | display `CS` | Idle high |
| `EPD_DC` | `P0.12` (`P3`) | display `DC` | |
| `EPD_RST` | `P0.28` (`P2`) | display `RST` | Low-drive, slow signal |
| `EPD_BUSY` | `P0.29` (`P2`) | display `BUSY` | Low-drive, input. Never to `3V3` |
| `SENSOR_CS` | `P0.30` (`P2`) | LIS3DH `CS` | Idle high. Low-drive |
| `SENSOR_INT1` | `P0.23` (`P4`) | LIS3DH `INT1` | Low-drive. `INT2` stays open |
| `DCF_PON` | `P0.24` (`P4`) | DCF `PON` | Active low, idle high. Low-drive |
| `DCF_OUT` | `P0.25` (`P4`) | DCF `OUT` | Push-pull, no pull. Low-drive |

The display `BS1` solder bridge must be at `0` (four-wire SPI).
`P0.22` (`P4`) is a free spare. No wire goes to a reserved pin (guide 6.1).

## 4. Buzzer stage (class `signal` plus local parts)

| Net | From | To | Note |
| --- | --- | --- | --- |
| `BUZZER_EN` | DK `P0.31` (`P2`) | 1 kOhm resistor, then transistor `B` | Idle low. Low-drive |
| collector | transistor `C` | buzzer `-`, diode anode | |
| flyback | diode banded end | buzzer `+` (on `5V`) | `1N4148` across the buzzer |
| emitter | transistor `E` | `GND` | Check the pin order against the datasheet first (`2N3904` and `BC547` differ) |

No buzzer terminal connects directly to `P0.31`.

## 5. Debug probes (optional, not product wiring)

| Probe | To | Note |
| --- | --- | --- |
| Saleae `CH0` | Nucleo generator `PA0` | Test stimulus only |
| Saleae `CH1` | DK `P0.25` | Watches `DCF_OUT` |
| Saleae `CH2` | DK `P0.24` | Watches `DCF_PON` |
| Saleae `GND` | DK `GND` | |

## 6. Rebuild order and checks

Do every step with USB power removed. After each group, run its check
before you go on.

1. **`GND` and `3V3`.** Wire section 1 without the `5V` row.
   - Check: continuity from DK `P1 VDD` to every `3V3` rail section.
   - Check: continuity from DK `P1 GND` to every `GND` rail section.
   - Check: no continuity between `3V3` and `GND`.
   - Check: nothing on `5V`, `External supply`, `nRF current measurement` or `VDD_nRF`.
2. **SPI bus.** Wire section 2.
   - Check: each of `SPI_SCK`, `SPI_MOSI`, `SPI_MISO` reaches only the devices listed.
   - Check: no continuity between any two bus lines, and none to `3V3` or `GND`.
3. **Signals.** Wire section 3.
   - Check: each DK pin reaches only its one device pin.
   - Check: no signal pin touches `3V3` or `GND` (except `DCF_PON` in the isolated DCF test, guide 11.2).
   - Check: `3V3` to `GND` still open.
4. **Buzzer stage.** Wire section 4 and the `5V` row.
   - Check: transistor pin order matches the datasheet.
   - Check: diode banded end is on the buzzer `+`.
   - Check: no continuity from `P0.31` to the buzzer or to `5V`.
   - Check: `3V3` to `GND` open, and `5V` to `GND` open.

The finished wiring matches this table with no extra wires.
