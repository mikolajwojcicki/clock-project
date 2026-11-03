## Purpose

Defines the soldered perfboard shield that plugs onto the nRF52 DK and counts as the thesis prototype: its circuit, schematic quality gate, layout artefacts and bring-up acceptance.

## ADDED Requirements

### Requirement: Prototype is a DK plug-on shield
The thesis prototype SHALL be a soldered pad-per-hole perfboard shield that mates with nRF52 DK (`PCA10040`) headers `P1`–`P4`. The e-paper HAT SHALL plug onto a 2×20 Raspberry Pi-layout male header on the shield, and the LIS3DH and DCF-77 modules SHALL plug into female sockets on the shield. Production-PCB-only parts (MDBT42Q module, discrete MCP73831, AP2112K, USB-C receptacle, crystal) SHALL NOT appear in the shield schematic or netlist.

#### Scenario: Shield is fitted to the DK
- **WHEN** the shield is pressed onto the DK long-pin headers
- **THEN** every used shield net reaches its DK pin through `P1`–`P4` with no other DK connection

#### Scenario: Module is replaced
- **WHEN** a breakout module is pulled from its socket
- **THEN** it can be removed and refitted without desoldering

#### Scenario: Reviewer checks for PCB parts
- **WHEN** the shield netlist is searched for MDBT42Q, AP2112K, a discrete MCP73831 or a USB-C receptacle
- **THEN** none of them is present

### Requirement: Shield nets match firmware pin map
Each shield signal SHALL connect to the DK GPIO that the bring-up firmware pin map assigns to it. DK LED pins `P0.17`–`P0.20` and UART pins `P0.05`–`P0.08` SHALL carry no shield net.

#### Scenario: Signal pin audit
- **WHEN** the netlist is compared with the firmware pin map
- **THEN** `SPI_SCK`/`MOSI`/`MISO` are on `P0.03`/`P0.04`/`P0.02`, `EPD_CS`/`DC`/`RST`/`BUSY` on `P0.11`/`P0.12`/`P0.28`/`P0.29`, `SENSOR_CS`/`INT1` on `P0.30`/`P0.23`, `DCF_PON`/`OUT` on `P0.24`/`P0.25`, `BUZZER_EN` on `P0.31`, SW1–SW4 on `P0.13`–`P0.16`, and `CLOCK_SEC_MARK` on a `P0.22` test pad

#### Scenario: Reserved DK pins
- **WHEN** the netlist is checked for `P0.05`–`P0.08` and `P0.17`–`P0.20`
- **THEN** no shield net connects to them

### Requirement: SPI clock has a series resistor
The shield SHALL place a resistor of about 100 Ω in series on `SPI_SCK`, next to DK pin `P0.03`, between the DK pin and the SPI bus.

#### Scenario: SCK path inspection
- **WHEN** continuity is measured from DK `P0.03` to the e-paper socket `CLK` pin
- **THEN** the path reads the series resistor value, not a direct short

#### Scenario: E-paper refresh on shield
- **WHEN** the firmware does repeated full e-paper refreshes on the shield
- **THEN** no refresh hangs waiting on `EPD_BUSY`

### Requirement: Buttons use DK internal pull-ups
Four tactile buttons SHALL connect `P0.13`–`P0.16` to `GND`. The shield SHALL fit no external pull-up resistors on these lines.

#### Scenario: Button press
- **WHEN** any of SW1–SW4 is pressed with the firmware pull-ups enabled
- **THEN** its GPIO reads low, and reads high when released

### Requirement: Battery path feeds DK 5V through a switch
In battery mode, the 1S Li-Po SHALL feed the shield through an off-board LiPo Amigo `BAT`/`VBAT` output, then a slide switch, onto a single switched net that drives DK `P1` `5V` and the buzzer supply. The Amigo USB `5V` pads SHALL NOT be used as a battery rail. The switch SHALL NOT be placed on DK `VDD`.

#### Scenario: Switch off
- **WHEN** the slide switch is off and USB is unplugged
- **THEN** DK `P1` `5V` and buzzer `+` read 0 V relative to `GND`

#### Scenario: Switch on
- **WHEN** the slide switch is on with a charged cell and USB unplugged
- **THEN** DK `P1` `5V` and buzzer `+` read the cell voltage and the DK `VDD` rail comes up

### Requirement: Power sources are never dual-fed
The schematic sheet and build sheet SHALL state that DK USB and the battery path must not supply the DK at the same time, and SHALL describe the USB bring-up mode and the battery mode separately.

#### Scenario: Reader checks operating modes
- **WHEN** a reader opens the schematic PDF
- **THEN** the sheet names both modes and states the dual-feed ban

### Requirement: Buzzer coil is not on 3V3
The buzzer and its flyback diode cathode SHALL connect to the switched `5V` net, with a low-side NPN driven from `BUZZER_EN` through a base resistor. They SHALL NOT connect to the 3.3 V logic rail.

#### Scenario: Buzzer rail audit
- **WHEN** the netlist is checked for the buzzer `+` pin and the diode cathode
- **THEN** both are on the switched `5V` net and neither is on `3V3`

#### Scenario: Alarm sounds in battery mode
- **WHEN** firmware drives `BUZZER_EN` in battery mode
- **THEN** the buzzer is audible

### Requirement: Local decoupling at module sockets
The shield SHALL fit at least 100 nF plus 10 µF near the e-paper and DCF sockets and at least 100 nF at the LIS3DH socket, between `3V3` and `GND`.

#### Scenario: Decoupling audit
- **WHEN** the schematic is reviewed
- **THEN** each listed capacitor is present and fitted, not DNP

### Requirement: Schematic passes ERC
The shield schematic SHALL pass KiCad ERC with zero errors. Each remaining warning SHALL be listed with a reason. Optional parts SHALL be marked DNP.

#### Scenario: ERC run
- **WHEN** ERC is run on the frozen shield schematic
- **THEN** the report shows 0 errors and is stored beside the schematic

### Requirement: Frozen hardware lives under hardware/
The frozen shield schematic, its exported netlist, its schematic PDF and its ERC report SHALL be stored under `hardware/`, not only in `scratch/`. The earlier breadboard schematic SHALL stay unchanged as an archive.

#### Scenario: Promotion check
- **WHEN** the change is complete
- **THEN** `hardware/` holds the shield schematic, netlist, PDF and ERC report, and the breadboard schematic files are unchanged

### Requirement: Layout is checked against the netlist before soldering
The perfboard layout SHALL be made from the exported schematic netlist, not by hand-copying nets. Before soldering, the layout tool connectivity check SHALL report no opens and no shorts, and the SPI bundle SHALL be kept away from the buzzer `5V` loop and from `EPD_RST`/`EPD_BUSY`.

#### Scenario: Pre-solder check
- **WHEN** the layout is ready to solder
- **THEN** the connectivity check shows zero opens and zero shorts

#### Scenario: SI review
- **WHEN** the layout is reviewed
- **THEN** no SPI trace runs next to the buzzer collector or `5V` loop

### Requirement: Build sheet guides soldering
A component-side and solder-side build sheet SHALL be exported from the layout and stored with the hardware files, with a continuity checklist ordered power, SPI, signals, buzzer.

#### Scenario: Builder solders from the sheet
- **WHEN** a builder follows the build sheet
- **THEN** every connection has a location on the sheet and a continuity check entry

### Requirement: All prototype connections are soldered
Every connection on the shield SHALL be soldered. The board SHALL carry no removable jumper wires.

#### Scenario: Visual inspection
- **WHEN** the finished shield is inspected
- **THEN** no Dupont or other plug-in jumper wire is present

### Requirement: Shield passes staged bring-up
Before modules are fitted, the bare shield SHALL pass a short check between each supply net and `GND`. With modules fitted, the stage 3 and 4 firmware SHALL run on the shield. Results SHALL be logged in a dated note under `notes/log/` with photos under `notes/media/`.

#### Scenario: Bare-board short check
- **WHEN** the empty shield is measured
- **THEN** `3V3`–`GND`, `5V`–`GND` and `VBAT`–`GND` show no short, and the values are logged

#### Scenario: Peripheral bring-up
- **WHEN** the stage 3 and 4 firmware runs on the shield in USB mode
- **THEN** the e-paper refreshes, the LIS3DH answers its ID read and raises `INT1` on motion, `DCF_OUT` pulses with `DCF_PON` enabled, all four buttons read, and the buzzer sounds

#### Scenario: Battery bring-up
- **WHEN** the same firmware runs in battery mode with USB unplugged
- **THEN** the same checks pass and the log records cell voltage and switch state

### Requirement: Thesis gets print-safe schematic blocks
The change SHALL export schematic block images for the thesis that are readable in grayscale print, using light-theme naming `YYYY-MM-DD_topic_detail.ext`.

#### Scenario: Grayscale print
- **WHEN** an exported block is printed in grayscale at thesis width
- **THEN** reference designators, net names and pin numbers are readable
