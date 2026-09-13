# Hardware and Tool Inventory

This inventory records what the project can use for prototype work.
It separates physical ownership from planned BOM decisions.

Source for the purchase claims:
[`temp-resources/shit-to-buy.md`](../../temp-resources/shit-to-buy.md).

The inventory does not replace a product datasheet.
Check the physical label before connecting power.

## Status meanings

| Status | Meaning |
| --- | --- |
| `Owned` | The user or current project record confirms possession. |
| `User-reported purchased` | The user reports buying the item, but the exact model or physical presence is not verified in this repository. |
| `Planned` | The project intends to use the item, but purchase is not confirmed. |
| `Optional` | The item is useful for later work and is not required for the current prototype. |
| `Unconfirmed` | A source mentions the item, but ownership or compatibility is not established. |
| `Rejected` | The project has a reason not to use the item. |

## Tools and measurement equipment

### User-reported tools already available

The user stated that these tools are available.
Exact models are not recorded.

| Tool | Status | Project use | Evidence |
| --- | --- | --- | --- |
| 3D printers | `Owned` | Enclosure prototypes | User statement in supplied conversation |
| Digital calipers | `Owned` | PCB and enclosure measurements | User statement in supplied conversation |
| Computer | `Owned` | Firmware, documentation, and design work | User statement in supplied conversation |
| Logic analyzer | `Owned` | SPI, I2C, and DCF-77 digital signal checks | User statement in supplied conversation |
| Digital multimeter | `Owned` | Voltage, continuity, resistance, and basic current checks | User statement in supplied conversation |
| Nordic nRF52 DK | `Owned` | nRF52832 prototype, SWD programmer, and debugger | User statement and project records |

### Recommended tools reported as purchased

The user later stated that the suggested hardware and tools were bought.
Exact models remain unverified unless listed in the table.

| Tool | Status | Project use | Suggested source or model | Verification note |
| --- | --- | --- | --- | --- |
| Nordic Power Profiler Kit II | `User-reported purchased` | Sleep, BLE, display, DCF-77, and alarm current measurements | PPK2 | Confirm model and physical presence |
| Adjustable bench power supply | `User-reported purchased` | Current-limited module and PCB power | LongWei PS-305D was suggested | Confirm voltage, current limit, and model |
| Temperature-controlled soldering iron | `User-reported purchased` | Headers, wires, and PCB assembly | Yihua 898BD station was suggested | Confirm station and tip set |
| Hot-air station or hot plate | `User-reported purchased` | SMD rework and later PCB assembly | Yihua 898BD or WEP 853A was suggested | Confirm tool and safe operating procedure |
| Li-Po safety bag | `User-reported purchased` | Battery charging and storage | 30 x 23 cm bag was suggested | Confirm bag is rated for Li-Po cells |
| USB-C power meter | `User-reported purchased` | USB charging and input-current checks | FNIRSI FNAC-28 was suggested | Confirm meter model and connector support |
| ESD silicone mat | `User-reported purchased` | Assembly work surface | Yihua ESD mat was suggested | Confirm grounded or static-safe setup |
| ESD tweezers | `User-reported purchased` | SMD handling | Six-piece ESD set was suggested | Confirm tips are clean and undamaged |
| PCB holder or magnifier | `User-reported purchased` | Board handling and inspection | ZD-126-2 was suggested | Confirm holder is stable |
| Solder, flux, and solder paste | `User-reported purchased` | Assembly and rework consumables | Cynel solder and Botland categories were suggested | Confirm type, alloy, and storage |
| Desoldering braid | `User-reported purchased` | Bridge and excess-solder removal | 2.5 mm braid was suggested | Confirm width and flux condition |

### Optional or not confirmed

| Tool | Status | Project use | Note |
| --- | --- | --- | --- |
| Oscilloscope | `Optional` | Supply ripple, buzzer drive, and DCF-77 analog checks | Logic analyzer and PPK2 cover earlier digital and current checks |
| PCB preheater | `Optional` | Later custom PCB assembly | Not needed for breadboard bring-up |
| Electronic load or battery tester | `Planned` | Battery capacity and runtime tests | Add after the power path is defined |

The optional oscilloscope and preheater are not marked purchased.
The user did not explicitly confirm those delayed purchases.

## Prototype hardware

These parts are already listed as owned in
[`hardware/bom/proto-bom.md`](../bom/proto-bom.md).
The exact identities below match current project records.

| Item | Status | Exact identity | Project use | Evidence |
| --- | --- | --- | --- | --- |
| Development board | `Owned` | Nordic nRF52 DK, `PCA10040` | nRF52832 controller and J-Link | Prototype BOM and project architecture |
| E-paper display | `Owned` | Waveshare 2.13inch e-Paper HAT, Rev 2.1 | Display | User-provided identity and prototype guide |
| Motion sensor | `Owned` | Adafruit LIS3DH breakout, product 2809 | Motion and alarm dismissal interrupt | User-provided identity and prototype guide |
| Time receiver | `Owned` | Drhomeam DCF-1060N-800 with ferrite antenna | DCF-77 synchronization | Local manual and prototype BOM |
| Buzzer | `Owned` | AP-1205V-P1 electromagnetic buzzer | Alarm sound | Prototype BOM |
| Buzzer transistor | `Owned` | `2N3904` or `BC547`, TO-92 | Low-side buzzer driver | Prototype BOM |
| Flyback diode | `Owned` | `1N4148` | Buzzer coil protection | Prototype BOM |
| Base resistor | `Owned` | Approximately 1 kOhm, 1/4 W | Transistor base current limit | Prototype BOM |
| Breadboard and jumpers | `Owned` | Solderless breadboard, M-M and F-M jumpers | Temporary wiring | Prototype BOM |
| Charger and battery | `Owned` | TP4056 USB-C breakout and 400 to 500 mAh Li-Po | Optional power experiments | Prototype BOM, but not used in first USB-powered bring-up |

### Prototype alternatives and warnings

The AP-1205V-P1 voltage and drive requirements remain unverified in project
documentation. Do not connect it directly to an nRF52 GPIO.
Use the transistor driver and flyback diode from the prototype guide.

The supplied research conversation listed a low-power passive piezo buzzer as
an alternative. It is a comparison candidate, not a replacement for the
locked AP-1205V-P1 choice.

## Final PCB candidates

Final PCB items are design candidates.
They are not purchase records.

| Candidate | Status | Role | Source or note |
| --- | --- | --- | --- |
| Raytac `MDBT42Q-512KV2` | `Planned` | BLE MCU module | Locked project architecture and PCB BOM |
| ST `LIS3DHTR` | `Planned` | Production accelerometer | Locked project architecture and PCB BOM |
| `AP2112K-3.3TRG1` | `Planned` | Current locked 3.3 V regulator | Locked project architecture |
| `TPS7A0533PDBVR` | `Unconfirmed` | Proposed lower-quiescent-current regulator alternative | Supplied research conversation; requires architecture approval |
| `MCP73831T-2ACI/OT` | `Planned` | Single-cell Li-Po charger | Locked project architecture |
| `TPS22917DBVR` | `Unconfirmed` | Proposed DCF power switch | Supplied research conversation; verify receiver power behavior |
| `GCT USB4105-GF-A` | `Planned` | USB-C power receptacle | PCB BOM candidate |
| JST `B2B-PH-K-S` | `Planned` | Battery connector | PCB BOM candidate |
| `MMBT3904` and `1N4148W` | `Planned` | Production buzzer driver and diode | PCB BOM candidate |
| Tactile buttons | `Planned` | User controls | PCB BOM candidate |
| 32.768 kHz crystal | `Planned` | Low-drift RTC clock | PCB BOM candidate |
| USB-C resistors, charger resistor, LED resistor, and capacitors | `Planned` | Power and decoupling support | PCB BOM candidate; values require electrical review |

The research conversation proposed `RPROG` for approximately 200 mA.
The selected battery rating must be checked before this value becomes final.

## Source links

- [Supplied purchase and component research](../../temp-resources/shit-to-buy.md)
- [Prototype BOM](../bom/proto-bom.md)
- [Production PCB BOM](../bom/pcb-bom.md)
- [Adafruit LIS3DH product 2809](https://adafru.it/2809)
- [Waveshare 2.13inch e-Paper HAT](https://www.waveshare.com/2.13inch-e-paper-hat.htm)
- [Local DCF-77 manual](../../temp-resources/docs/dcf77/dcf77-manual-dump.html)

