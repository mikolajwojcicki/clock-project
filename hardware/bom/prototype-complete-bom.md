# Complete first prototype BOM

This is the unfiltered list for the first breadboard prototype.
It includes parts, tools, cables, and optional later power parts.

`Status` is intentionally not filled as owned. Filter it with the actual
inventory before buying.

## Main prototype parts

| # | Category | Item | Exact specification | Qty | Needed for | Status |
| ---: | --- | --- | ---: | ---: | --- | --- |
| 1 | Controller | nRF52 development kit | Nordic nRF52 DK, PCA10040, nRF52832 | 1 | MCU and programmer | To verify |
| 2 | Display | E-paper HAT | Waveshare 2.13inch e-Paper HAT, Rev2.1 | 1 | Time display | To verify |
| 3 | Display connector | Header pins | 1x8, 2.54 mm male header, if the HAT header is not installed | 1 | HAT connection | Check board |
| 4 | Motion | Accelerometer breakout | Adafruit LIS3DH, product 2809 | 1 | Motion detection | To verify |
| 5 | Motion connector | Header pins | 2x7, 2.54 mm male header, or 14-pin breakaway header | 1 | LIS3DH breadboard connection | Check board |
| 6 | Radio | DCF receiver | Drhomeam DCF-1060N-800 or the photographed equivalent, with antenna | 1 | DCF-77 time sync | To verify |
| 7 | Radio connector | Header pins or wire | 1x4, 2.54 mm header, or wire suitable for the DCF edge pads | 1 | DCF breadboard connection | Check board |
| 8 | Alarm | Electromagnetic buzzer | AP-1205V-P1, exact voltage and current still to verify | 1 | Audible alarm | To verify |
| 9 | Alarm driver | NPN transistor | 2N3904, TO-92, or BC547, but use one part type | 1 | Buzzer low-side switch | To verify |
| 10 | Alarm protection | Flyback diode | 1N4148, through-hole | 1 | Buzzer coil protection | To verify |
| 11 | Wiring | Solderless breadboard | Full-size board with power rails | 1 | Temporary assembly | To verify |
| 12 | Wiring | Jumper wires | Male-to-male, 2.54 mm | 1 set | Breadboard connections | To verify |
| 13 | Wiring | Jumper wires | Female-to-male, 2.54 mm | 1 set | DK and module connections | To verify |
| 14 | Wiring | Short hookup wire | Flexible wire for DCF and buzzer connections | 1 set | Board-edge connections | To verify |
| 15 | Power | USB data cable | Cable compatible with the nRF52 DK | 1 | First prototype power | To verify |

## Resistors

Buy a resistor assortment if possible. It prevents a second order when one
value is missing.

| # | Value | Type | Qty | Use | Status |
| ---: | ---: | --- | ---: | --- | --- |
| R1 | 1 kOhm | 1/4 W through-hole | 5 | NPN base current limit | Missing, buy |
| R2 | 10 kOhm | 1/4 W through-hole | 5 | Optional DCF `PON` pull-up, receiver disabled by default | Missing, buy |
| R3 | 100 kOhm | 1/4 W through-hole | 5 | Optional buzzer transistor base pull-down | Missing, buy |

The guide requires R1 for the buzzer test. R2 and R3 improve safe startup
behavior. Do not substitute resistor values until the wiring plan is checked.

## Capacitors and small support parts

These parts may already exist on the breakouts. Check the boards before buying
or adding them.

| # | Item | Exact specification | Qty | Use | Status |
| ---: | --- | --- | ---: | --- | --- |
| C1 | Ceramic capacitor | 100 nF, X7R or similar, at least 6.3 V | 2 | Local sensor and receiver bypass | Check boards first |
| C2 | Bulk capacitor | 10 uF, ceramic or electrolytic, at least 6.3 V | 2 | Local sensor and receiver bypass | Check boards first |
| D2 | Spare diode | 1N4148, through-hole | 2 | Spare buzzer protection parts | Optional |

The LIS3DH datasheet calls for 100 nF and 10 uF near the sensor supply.
Do not remove capacitors already fitted to a breakout. The DCF board already
shows local capacitors in the supplied photographs.

## Measurement and assembly tools

| # | Tool | Exact specification | Qty | Use | Status |
| ---: | --- | --- | ---: | --- | --- |
| T1 | Bench power supply | KORAD KKG305D, 0 to 30 V, 0 to 5 A | 1 | Isolated DCF test, set to 3.30 V and 100 mA limit | To verify |
| T2 | Digital multimeter | DC voltage, continuity, resistance, and current ranges | 1 | Rail, short, and current checks | To verify |
| T3 | Logic analyzer | 3.3 V-compatible digital inputs | 1 | DCF `OUT` and SPI signal checks | To verify |
| T4 | Soldering iron | Temperature-controlled, fine tip | 1 | Headers and wires | To verify |
| T5 | Solder | Electronics solder, suitable for through-hole work | 1 | Soldering headers and wires | To verify |
| T6 | Flux | Electronics flux | 1 | Clean solder joints | Optional |
| T7 | Wire cutters | Small electronics cutters | 1 | Cut wires and headers | Optional |
| T8 | Wire strippers | Small electronics strippers | 1 | Prepare hookup wire | Optional |
| T9 | Magnifier | Desk magnifier or inspection lamp | 1 | Inspect solder joints and board labels | Optional |

## Optional power experiment parts

Do not connect these during first USB-powered bring-up.

| # | Item | Exact specification | Qty | Use | Status |
| ---: | --- | --- | ---: | --- | --- |
| P1 | Li-Po battery | Single-cell 3.7 V, 400 to 500 mAh, JST-PH if used | 1 | Later battery tests | Optional |
| P2 | Charger board | TP4056 USB-C breakout | 1 | Later battery charging tests | Optional |
| P3 | Li-Po safety bag | Suitable for single-cell Li-Po storage and charging | 1 | Battery safety | Optional |

## Do not buy for first prototype

| Item | Reason |
| --- | --- |
| 5 V power adapter for the breadboard | First integrated test uses the DK 3.3 V rail |
| Extra DCF receiver | The photographed module is already available for testing |
| External DS3231 RTC | Not part of the locked architecture |
| OLED display | Not part of the locked architecture |
| STM32 board | Not part of the locked architecture |
| Zephyr or RTOS tooling | Firmware uses bare-metal nRF5 SDK |

## Sources

- [Prototype guide](../docs/breadboard-prototype-guide.md)
- [Existing prototype BOM](proto-bom.md)
- [Hardware and tool inventory](../inventory/prototype-tools.md)
- [Waveshare 2.13inch e-Paper HAT](https://www.waveshare.com/2.13inch-e-paper-hat.htm)
- [Adafruit LIS3DH product 2809](https://adafru.it/2809)
- [Local DCF-77 manual](../../temp-resources/docs/dcf77/dcf77-manual-dump.html)
