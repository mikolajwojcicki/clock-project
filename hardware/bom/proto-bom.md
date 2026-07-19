# Prototyping BOM (breadboard / nRF52 DK)

Bring-up kit for firmware and peripheral integration **before** the custom PCB. Status reflects on-hand parts (2026-07).

Production / PCB BOM: [`pcb-bom.md`](pcb-bom.md).

| # | Item | Role | Status |
| --- | --- | --- | --- |
| 1 | Nordic nRF52 DK (`PCA10040`) | nRF52832 + on-board J-Link, header breakout | Owned |
| 2 | Waveshare 2.13" e-Paper module (1×8 2.54 mm) | Display (HV boost on module) | Owned |
| 3 | LIS3DH breakout (Adafruit or equiv., 2.54 mm) | Motion / alarm-dismiss wake via INT | Owned |
| 4 | DCF-77 receiver module + ferrite antenna | Periodic time sync; power-gate when idle | Owned |
| 5 | Electromagnetic buzzer `AP-1205V-P1` (THT) | Alarm sound | Owned |
| 6 | NPN `2N3904` or `BC547` (TO-92) | Buzzer driver | Owned |
| 7 | Diode `1N4148` | Flyback across buzzer | Owned |
| 8 | Base resistor ~1 kΩ (¼ W) | NPN base | Owned |
| 9 | Breadboard + M-M / F-M jumpers | Wiring | Owned |
| 10 | TP4056 USB-C charger breakout + Li-Po ~400–500 mAh (JST-PH) | Optional power experiments | Owned |

## Notes

- Prefer the DK for all early firmware; Raytac `MDBT42Q` lands on the custom PCB.
- Keep SPI (e-paper) off `P0.22`–`P0.30` (low-drive / low-frequency only) — see `AGENTS.md`.
- Durable pin maps and wiring diagrams go under `hardware/docs/` as bring-up progresses.
