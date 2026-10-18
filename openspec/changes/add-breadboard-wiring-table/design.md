## Context

Pin map: guide section 6 and `firmware/boards/dk_breadboard_pins.h`. Module pin labels come from guide sections 9 onward (display `VCC GND DIN CLK CS DC RST BUSY`; LIS3DH breakout `VIN GND SCL SDA SDO CS INT1 INT2`; DCF-1060N `VDD GND PON OUT`). Supply is DK `P1 VDD` as the `3V3` rail only. Reserved DK pins stay unused.

## Goals / Non-Goals

**Goals:**
- One table, one row per wire, both ends named.
- Bus and signal classes visible for perfboard routing.

**Non-Goals:**
- No pin changes, no firmware edits, no KiCad work.
- Battery and charger are not part of this table (guide forbids them on the `3V3` rail).

## Decisions

- New file `hardware/docs/breadboard-wiring-table.md`, not a rewrite of the guide: the guide stays the explanation, the table is the checklist. Guide section 6 gets one link line.
- Draft content of the table (the apply step verifies it against the guide and the header, then writes it):

**Power (class `power`)**

| Net | From | To | Note |
| --- | --- | --- | --- |
| `3V3` | DK `P1 VDD` | rail; display `VCC`, LIS3DH `VIN`, DCF `VDD`, buzzer `+`, diode cathode | Single source. Never to a `5V` pin |
| `GND` | DK `P1 GND` | rail; display `GND`, LIS3DH `GND`, DCF `GND`, transistor emitter | Join split rail halves with a jumper |

**Shared SPI bus (class `bus`; one DK pin fans out to several devices)**

| Signal | DK pin | Device pins on the line | Note |
| --- | --- | --- | --- |
| `SPI_SCK` | `P0.03` | display `CLK`, LIS3DH `SCL` | Fast; not on `P0.22`-`P0.31` |
| `SPI_MOSI` | `P0.04` | display `DIN`, LIS3DH `SDA`/`SDI` | Never to `MISO` |
| `SPI_MISO` | `P0.02` | LIS3DH `SDO` | Display has no `MISO` |

**Point-to-point signals (class `signal`)**

| Signal | DK pin | Device pin | Note |
| --- | --- | --- | --- |
| `EPD_CS` | `P0.11` | display `CS` | Idle high |
| `EPD_DC` | `P0.12` | display `DC` | |
| `EPD_RST` | `P0.28` | display `RST` | Low-drive pin, slow signal |
| `EPD_BUSY` | `P0.29` | display `BUSY` | Low-drive pin, input; not to `3V3` |
| `SENSOR_CS` | `P0.30` | LIS3DH `CS` | Idle high; low-drive pin |
| `SENSOR_INT1` | `P0.23` | LIS3DH `INT1` | Low-drive pin; `INT2` left open |
| `DCF_PON` | `P0.24` | DCF `PON` | Active low; idle high |
| `DCF_OUT` | `P0.25` | DCF `OUT` | Push-pull, no pull |

**Buzzer stage (class `signal` plus local parts)**

| Net | From | To | Note |
| --- | --- | --- | --- |
| `BUZZER_EN` | DK `P0.31` | 1 kOhm to transistor base | Idle low; optional 100 kOhm base to `GND` |
| collector | transistor collector | buzzer `-` and diode anode | |
| flyback | diode cathode | `3V3` (with buzzer `+`) | `1N4148` across the buzzer |
| emitter | transistor emitter | `GND` | |

**Debug probes (optional, not part of the product wiring):** Saleae `CH0` generator `PA0`, `CH1` `P0.25`, `CH2` `P0.24`, Saleae `GND` to DK `GND`.

## Risks / Trade-offs

- Module pin labels differ between breakouts; the table uses the labels of the owned parts in the guide and says so.
- The table duplicates guide section 6. Risk of drift; mitigated by the spec scenario that compares it with the header, and by linking instead of copying prose.
