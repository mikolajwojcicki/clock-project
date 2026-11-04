# Prototyping BOM (breadboard / nRF52 DK)

Bring-up kit for firmware and peripheral integration before the custom PCB.
Status reflects current project records and user-provided hardware identities.

Production / PCB BOM: [`pcb-bom.md`](pcb-bom.md).
Detailed tool and ownership record:
[`hardware/inventory/prototype-tools.md`](../inventory/prototype-tools.md).

| # | Item | Exact identity | Role | Status | Evidence |
| --- | --- | --- | --- | --- | --- |
| 1 | Nordic nRF52 DK | `PCA10040` / nRF52832 | MCU, USB power, and on-board J-Link | Owned | Project record and user statement |
| 2 | Waveshare e-Paper HAT | 2.13inch, Rev 2.1, 2x20 RPi female header + 1x8 cable | Display with module-side power conversion | Owned | User-provided identity and prototype guide |
| 3 | LIS3DH breakout | Adafruit product 2809 | Motion and alarm-dismiss interrupt | Owned | User-provided identity and prototype guide |
| 4 | DCF-77 receiver and antenna | Drhomeam DCF-1060N-800 | Periodic time synchronization | Owned | Local manual and user-provided hardware |
| 5 | Electromagnetic buzzer | `AP-1205V-P1`, THT | Alarm sound | Owned | Existing project record |
| 6 | NPN transistor | `2N3904` or `BC547`, TO-92 | Buzzer driver | Owned | Existing project record |
| 7 | Flyback diode | `1N4148` | Buzzer coil protection | Owned | Existing project record |
| 8 | Base resistor | Approximately 1 kOhm, 1/4 W | NPN base current limit | Owned | Existing project record |
| 9 | Breadboard and jumpers | Solderless board, M-M and F-M wires | Temporary wiring | Owned | Existing project record |
| 10 | Charger and battery | Pimoroni LiPo Amigo `PIM611` (plain, off-board charger on `J5`) and Akyga `LP503035` 3.7 V 500 mAh protected Li-Po with JST-PH | Battery mode: cell to Amigo `BAT`/`VBAT` pads to `J5` | To buy | `temp-resources/shit-to-buy.md`; schematic `J5` `Amigo_VBAT` |
| 10a | TP4056 USB-C breakout | TP4056 | Unused spare; replaced by the LiPo Amigo | Owned | Existing project record |
| 11 | ST Nucleo-F411RE | `MB1136 C-04` | Bench DCF-77 signal generator, not product hardware | Owned | User statement |
| 12 | SCK series resistor | Approximately 100 Ohm (47 to 220 Ohm), 1/4 W | Damps ringing on `SPI_SCK` at `P0.03`; without it the display hangs | To buy | `notes/log/2026-10-22_epd-sck-ringing.md` |
| 13 | Perfboard | Approx. 7x9 cm, double-sided, 2.54 mm pitch, plated holes | Shield for the nRF52 DK; carries all peripherals | To buy | Decision 2026-10-24: perfboard is a DK shield |
| 14 | Female header, long pin | 1x10 and 1x8, 2.54 mm, for DK connectors | Plugs the shield onto the DK GPIO and power headers | To buy | Shield decision |
| 15 | Male pin header 2x20 | 2.54 mm, RPi GPIO layout; built from two 1x20 halves of a 1x40 strip in adjacent perfboard rows | Waveshare HAT plugs onto the shield (decision 2026-11-03, replaces the 1x8 socket) | Owned (1x40 strips) | `hardware/docs/waveshare-hat-40pin.md` |
| 16 | Female headers | 2.54 mm, sized to LIS3DH and DCF-1060N-800 pin rows | Keeps the breakouts removable | To buy | Shield decision |
| 17 | Male pin header strip | 1x40, 2.54 mm | Breakout pins and test points | To buy | Shield decision |
| 18 | Pull-up resistors | 10 kOhm, 1/4 W | Spares only: the shield fits no button pull-ups (DK internal pull-ups) | To buy | `hardware/docs/perfboard-shield.md` |
| 19 | Tactile buttons | 6x6 mm THT, 3 to 4 pcs | MODE, UP, DOWN, SNOOZE | To buy | Shield decision |
| 20 | Decoupling capacitors | 100 nF ceramic and 10 uF | Local decoupling at DCF and LIS3DH | To buy | Shield decision |
| 21 | Battery connector | JST-PH 2-pin, THT | Li-Po connection on the shield | To buy | Shield decision |
| 22 | Slide switch | `MSS-2245`, SPDT, 2.54 mm (one pole used) | `SW5`: battery `VBAT` to `5V` | To buy | Shield decision; `hardware/kicad/dk-perfboard-shield.kicad_sch` |
| 23 | Standoffs | M3 with screws | Mount the shield | To buy | Shield decision |

## Warnings and alternatives

- Use the nRF52 DK for early firmware. The Raytac `MDBT42Q` belongs to the
  custom PCB.
- The perfboard is a shield for the nRF52 DK, not a standalone device. The
  MDBT42Q module stays on the custom PCB. Pin map and wiring diagram go under
  `hardware/docs/`.
- Keep display SPI away from P0.22 through P0.30. These pins have the module
  drive and frequency restrictions described in `AGENTS.md`.
- The AP-1205V-P1 voltage and drive requirements are not confirmed in this
  repository. Do not drive it from an nRF52 GPIO. Use the transistor and
  flyback diode circuit in the breadboard guide.
- A low-power passive piezo buzzer is a comparison candidate from the supplied
  research conversation. It does not replace the AP-1205V-P1 architecture.
- Do not connect the battery path (Amigo, Akyga, SW5) during first USB-powered
  bring-up, and never feed the DK from USB and the battery at the same time.
- The TP4056 breakout is replaced by the LiPo Amigo and is an unused spare. See
  [`perfboard-shield.md`](../docs/perfboard-shield.md) and the schematic
  [`dk-perfboard-shield`](../kicad/dk-perfboard-shield.kicad_sch).

## Sources

- [Prototype tool and ownership inventory](../inventory/prototype-tools.md)
- [Adafruit LIS3DH product 2809](https://adafru.it/2809)
- [Waveshare 2.13inch e-Paper HAT](https://www.waveshare.com/2.13inch-e-paper-hat.htm)
- [Local DCF-77 manual](../../temp-resources/docs/dcf77/dcf77-manual-dump.html)
- [Supplied purchase and component research](../../temp-resources/shit-to-buy.md)

Durable pin maps and wiring diagrams belong under `hardware/docs/`.
