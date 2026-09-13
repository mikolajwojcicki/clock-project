# First Breadboard Prototype Guide

This guide builds the first hardware prototype for the low-power alarm clock.
It uses an nRF52 DK and separate modules on solderless breadboards.

Follow sections in order. Do not connect all parts before testing the controller.
Remove USB power before you change any wire.

## 1. What you will build

The prototype contains five functional parts:

1. The nRF52 DK controls the prototype.
2. The Waveshare display shows information.
3. The LIS3DH detects movement.
4. The DCF-77 module receives the time signal.
5. The transistor circuit drives the alarm buzzer.

The first power source is the USB connection on the nRF52 DK.
Do not connect the Li-Po battery or TP4056 charger during first bring-up.

This guide uses these exact parts:

- Nordic nRF52 DK, board `PCA10040`, with nRF52832
- Waveshare 2.13inch e-Paper HAT, Rev 2.1
- Adafruit LIS3DH breakout, product 2809
- Drhomeam DCF-1060N-800 DCF-77 receiver module
- AP-1205V-P1 electromagnetic buzzer
- `2N3904` or `BC547` NPN transistor
- `1N4148` diode
- 1 kOhm, 1/4 W resistor
- Solderless breadboard
- Male-to-male and female-to-male jumper wires

The parts list is also in [`hardware/bom/proto-bom.md`](../bom/proto-bom.md).

## 2. Safety rules

Read this section before placing wires.

### Stop conditions

Remove the DK USB cable immediately if:

- Any part becomes hot.
- You smell burning plastic or electronics.
- A wire becomes hot.
- The DK resets repeatedly after you connect a module.
- You see smoke.

Do not reconnect power until you find the cause.

### Power rules

- Use only the DK 3.3 V pin for first tests.
- Connect every module ground to the DK ground.
- Never connect the DK 3.3 V pin to a module pin marked `5V`.
- Never connect the DK 5 V or USB pin to a 3.3 V-only input.
- Never connect a Li-Po battery to the breadboard during first bring-up.
- Never connect the TP4056 charger to the DK 3.3 V rail.
- Never connect two power sources to the same rail.
- Do not power the buzzer from a GPIO pin.
- Do not connect a signal wire before checking its voltage range.

The display accepts 3.3 V or 5 V input through its onboard voltage translator.
Use 3.3 V in this prototype.

The Adafruit LIS3DH breakout has a regulator and level shifting.
Use its `VIN` pin with 3.3 V, or use `3Vo` only when the board wiring requires it.
Do not connect both `VIN` and `3Vo` to different power sources.

The DCF-1060N-800 manual states a 1.1 V to 3.3 V supply range.
Use 3.3 V only after you identify `VDD` and `GND` on your board.

The project BOM labels the AP-1205V-P1 as a 5 V buzzer, but its exact operating
current and drive requirements are not confirmed in the available documents.
Do not assume that 3.3 V produces its rated sound. Do not add 5 V power during
first bring-up.

For the integrated prototype, use only the DK `3V3` rail. Never connect the
KORAD supply and DK `3V3` to the same breadboard rail.

### ESD and mechanical rules

Electrostatic discharge (ESD) is a small electrical spark that can damage
electronics without visible damage.

- Work on a clean, dry table.
- Touch the breadboard rails before handling modules.
- Hold boards by their edges.
- Do not press on the e-paper panel.
- Keep the DCF-77 ferrite antenna away from USB cables, monitors, speakers,
  motors, and switching power supplies.
- Do not bend the antenna wires sharply.

## 3. Glossary

**Voltage** is electrical pressure between two points. This guide uses volts,
written as `V`.

**Ground** is the shared electrical reference. It is written as `GND`.

**GPIO** means general-purpose input/output. An MCU GPIO is a pin that firmware
can read or drive.

**SPI** is a synchronous serial bus. It transfers data with a clock and separate
chip-select lines.

**I2C** is a two-wire serial bus. This prototype uses SPI for the LIS3DH instead,
so the display and sensor can share clock and data wires.

An **interrupt** is a signal that asks the MCU to stop normal work and handle an
event, such as movement.

A **breadboard rail** is a long row of connected holes used for power or ground.
Many breadboards split each rail in the middle.

A **pull-up** is a resistor that holds a digital signal at a high voltage until
another device pulls it low. This guide does not add external pull-up resistors
for the first SPI tests.

A **flyback diode** protects a transistor from the voltage pulse produced when
current stops in a coil. The buzzer is a coil load.

## 4. Tools and materials

Availability states for tools and hardware are maintained in
[`hardware/inventory/prototype-tools.md`](../inventory/prototype-tools.md).
The `Required now` and `Status` columns below describe this guide only.

### Required hardware

Mark each item before you begin:

| Item | Required now | Status | Purpose |
| --- | --- | --- | --- |
| nRF52 DK `PCA10040` | Yes | Owned | MCU, USB power, programmer |
| Waveshare 2.13inch e-Paper HAT Rev 2.1 | Yes | Owned | Display |
| Adafruit LIS3DH product 2809 | Yes | Owned | Motion sensor |
| DCF-1060N-800 module and antenna | Yes | Owned | DCF-77 time signal |
| AP-1205V-P1 buzzer | Yes | Owned | Alarm sound |
| `2N3904` or `BC547` | Yes | Owned | Buzzer switch |
| `1N4148` | Yes | Owned | Buzzer flyback protection |
| 1 kOhm resistor | Yes | Owned | Transistor base current limit |
| Solderless breadboard | Yes | Owned | Temporary wiring |
| Jumper wires | Yes | Owned | Connections |

### Optional hardware

The TP4056 USB-C board and 400 to 500 mAh Li-Po are optional.
Leave them disconnected during this guide.

Use them only in a later power experiment after the USB-powered prototype works.

### Tools

The following tools reduce wiring and fault-finding errors:

| Tool | Status | Use |
| --- | --- | --- |
| Computer running NixOS | Required | Build and flash firmware |
| USB data cable | Required | Power and program the DK |
| Digital multimeter | Required | Check voltage, continuity, shorts, and current |
| KORAD KKG305D bench supply | User-reported purchased | Isolated, current-limited DCF measurements |
| Logic analyzer | Owned | Observe SPI and DCF-77 signals |

The KORAD supply is for isolated module measurements only in this guide.
The first integrated prototype remains powered from the nRF52 DK USB
connection.

### Required software

This NixOS host already provides:

- `arm-none-eabi-gcc`
- nRF5 SDK 17.1.0 through `NRF5_SDK_DIR`
- `make`
- `openocd`

Check the tools before connecting hardware:

```bash
command -v arm-none-eabi-gcc
command -v make
command -v openocd
test -n "$NRF5_SDK_DIR" && printf '%s\n' "$NRF5_SDK_DIR"
test -n "$GNU_INSTALL_ROOT" && printf '%s\n' "$GNU_INSTALL_ROOT"
```

Do not install Zephyr or nRF Connect SDK Bare Metal for this prototype.
This project uses bare-metal nRF5 SDK firmware.

### 4.1 Safe bench-supply measurement

Use this procedure only for the isolated DCF current test. Keep the DK,
display, sensor, buzzer, battery, and charger disconnected.

1. Set the KORAD output voltage to `3.30 V`.
2. Set its current limit to `100 mA` or less.
3. Turn the KORAD output off.
4. Set the multimeter to DC current.
5. Put the red lead in the correct `mA`, `µA`, or `A` socket and the black lead
   in `COM`.
6. Connect the meter in series:

   ```text
   KORAD PS+ -> meter current input -> meter COM -> DCF VDD
   KORAD PS- -> DCF GND
   ```

7. Check that `VDD` is not connected to the ground rail.
8. Enable the output only after the wiring check passes.

Never place a current-mode multimeter across `PS+` and `PS-`. That creates a
short circuit through the meter. If the meter is in the wrong socket or mode,
the supply shows unexpected current, or a fuse opens, turn the supply output
off, remove power, and correct the setup before reconnecting anything.

## 5. Breadboard preparation

### 5.1 Identify the breadboard rows

Most solderless breadboards have five connected holes on each side of a center
gap. Holes across the center gap are not connected.

Power rails run along the long edges. A red line often marks positive voltage.
A blue or black line often marks ground.

Your breadboard can use different colors or split rails.
Test the exact board before using it:

1. Choose two holes in one suspected rail.
2. Set a multimeter to continuity mode.
3. Touch one probe to each hole.
4. Confirm that the meter reports continuity.
5. Test both halves of the rail.
6. Do not bridge rail halves until you confirm that the bridge is safe.

If you have no multimeter, treat each rail half as separate.
Use a jumper wire to connect only the rails that you can identify visually.

### 5.2 Assign rail names

Use these names in your build notes:

| Rail name | Voltage | Connect to |
| --- | --- | --- |
| `3V3` | 3.3 V | DK `3V3` |
| `GND` | 0 V reference | DK `GND` |

Do not use a rail named `5V` in first bring-up.
Do not place the Li-Po on any rail.

### 5.3 Use wire colors

Color does not change electrical behavior.
Consistent colors reduce mistakes.

| Color | Use |
| --- | --- |
| Red | `3V3` power |
| Black or blue | `GND` |
| Yellow | SPI clock |
| Green | SPI data |
| White | Control or interrupt signal |
| Orange | DCF-77 signal |
| Purple | Buzzer control |

## 6. Project pin map

This guide uses one shared SPI bus.
The display and LIS3DH have separate chip-select signals.

The pin map keeps SPI and PWM away from P0.22 through P0.30.
P0.22 through P0.30 remain unused except for low-frequency control signals.

| nRF52 DK pin | Project name | Connected device | Purpose |
| --- | --- | --- | --- |
| `P0.13` | `EPD_SCK` | Display `CLK` and LIS3DH `SCK` | SPI clock |
| `P0.15` | `EPD_MOSI` | Display `DIN` and LIS3DH `SDI` | SPI controller-to-device data |
| `P0.14` | `SENSOR_MISO` | LIS3DH `SDO` | SPI device-to-controller data |
| `P0.17` | `EPD_CS` | Display `CS` | Display chip select |
| `P0.18` | `EPD_DC` | Display `DC` | Display data or command |
| `P0.19` | `EPD_RST` | Display `RST` | Display reset |
| `P0.20` | `EPD_BUSY` | Display `BUSY` | Display busy status |
| `P0.21` | `SENSOR_CS` | LIS3DH `CS` | Sensor chip select |
| `P0.23` | `SENSOR_INT1` | LIS3DH `INT1` | Motion interrupt |
| `P0.24` | `DCF_PON` | DCF `PON` | Conditional receiver power control |
| `P0.25` | `DCF_OUT` | DCF `OUT` | Receiver digital output |
| `P0.12` | `BUZZER_EN` | Transistor base resistor | Buzzer control |

`P0.11`, `P0.16`, and P0.26 through P0.30 are unused in this guide.

### 6.1 Safe control states

Keep peripheral signal wires disconnected while firmware is being prepared.
Before connecting them, the firmware test must set these inactive states:

| Signal | Inactive state | Reason |
| --- | --- | --- |
| `DCF_PON` | High | Active-low receiver enable |
| `EPD_CS` | High | Do not select display during reset |
| `SENSOR_CS` | High | Do not select sensor during reset |
| `BUZZER_EN` | Low | Do not sound buzzer during reset |

If firmware cannot guarantee these states, leave the affected control wire
disconnected and test that peripheral separately. Do not rely on an unconfigured
MCU pin's floating input state as a safety control.

The DK header can label pins as `P0.13`, `13`, or with a board-specific name.
Use the DK pin label and the nRF52 port name together.
Do not treat Arduino-style numbers as nRF52 port numbers without checking the DK
pinout.

## 7. Controller-only test

Complete this test before connecting any peripheral.

### 7.1 Build the existing blinky app

Open a terminal in the repository root:

```bash
cd firmware/apps/blinky
make
```

The build must finish without an error.
The output must include:

```text
armgcc/_build/nrf52832_xxaa.hex
```

If the build fails, stop here.
Check `NRF5_SDK_DIR`, `GNU_INSTALL_ROOT`, and the build error.

### 7.2 Flash the DK

Connect the DK to the computer with USB.
Use the DK USB connector connected to its onboard J-Link programmer.

Flash the firmware:

```bash
make flash
```

The DK LED must change state about every 500 ms.
The existing app inverts each DK LED and waits 500 ms.

If the LED does not change:

1. Remove USB power.
2. Check that the DK USB cable carries data.
3. Reconnect the USB cable.
4. Run `make erase`.
5. Run `make flash` again.

Do not continue to peripheral wiring until this test passes.

This app does not test the display, LIS3DH, DCF-77, or buzzer.
Peripheral tests require later firmware drivers.
The wiring sections below define hardware assembly and test observations for
those drivers.

## 8. Common power connections

Perform these steps with USB power removed.

1. Connect DK `3V3` to the breadboard `3V3` rail.
2. Connect DK `GND` to the breadboard `GND` rail.
3. Connect one jumper between each split half of the `3V3` rail if needed.
4. Connect one jumper between each split half of the `GND` rail if needed.
5. Check continuity from the DK `3V3` pin to every `3V3` rail section.
6. Check continuity from the DK `GND` pin to every `GND` rail section.
7. Check that `3V3` and `GND` are not connected together.

Do not connect modules yet.
Power the DK and measure the rail voltage if you have a multimeter.
The rail must be close to 3.3 V.
Remove power if the rail is 0 V, above 3.3 V, or unstable.

## 9. Connect the e-paper display

The photographed board is the Waveshare 2.13inch e-Paper HAT Rev 2.1.
It uses these labels:

Before wiring, inspect the `BS1` solder bridge. Set it to `0` for four-wire
SPI. Do not continue if the bridge is set to `1`, because that selects
three-wire SPI and does not match this guide.

| Display pin | DK or rail | Signal |
| --- | --- | --- |
| `VCC` | `3V3` | 3.3 V power |
| `GND` | `GND` | Shared ground |
| `DIN` | `P0.15` | SPI data |
| `CLK` | `P0.13` | SPI clock |
| `CS` | `P0.17` | Display chip select |
| `DC` | `P0.18` | Data or command |
| `RST` | `P0.19` | Reset |
| `BUSY` | `P0.20` | Busy status |

### 9.1 Wire the display

With USB power removed:

1. Place the display so its connector cannot touch the breadboard power rails.
2. Connect display `VCC` to `3V3`.
3. Connect display `GND` to `GND`.
4. Connect display `DIN` to DK `P0.15`.
5. Connect display `CLK` to DK `P0.13`.
6. Connect display `CS` to DK `P0.17`.
7. Connect display `DC` to DK `P0.18`.
8. Connect display `RST` to DK `P0.19`.
9. Connect display `BUSY` to DK `P0.20`.
10. Confirm the `BS1 = 0` four-wire SPI setting again.
11. Check every wire against the table.

Do not connect display `DIN` to `MISO`.
Do not connect display `BUSY` to `3V3`.
Do not press the display panel while inserting the connector.

### 9.2 Display test

The display test needs a later e-paper driver.
Do not claim that the blinky app tests the display.

When a display test is available:

1. Remove USB power.
2. Check the eight wires again.
3. Connect USB power.
4. Run the display test.
5. Wait for the full refresh to finish.
6. Record the displayed test pattern.
7. Remove power and record whether the image remains.

The display must show the test pattern.
The image can remain after power is removed because e-paper keeps its image.

If the display stays blank:

1. Remove USB power.
2. Confirm `VCC` is 3.3 V relative to `GND`.
3. Confirm `CS`, `DC`, `RST`, and `BUSY` use the table above.
4. Confirm `DIN` and `CLK` are not swapped.
5. Confirm `BS1 = 0`.
6. Confirm no display wire crosses the breadboard center gap incorrectly.
7. Run a full refresh instead of a partial refresh.
8. Test the display by itself, without the LIS3DH or buzzer.

## 10. Connect the LIS3DH

The Adafruit LIS3DH board supports I2C and SPI.
This prototype uses SPI so the display and sensor can share the bus.

The photographed breakout has `VIN`, `3Vo`, `GND`, `SCL`, `SDA`, `SDO`,
`CS`, `INT1`, and `INT2` labels. Use those labels and not connector position.
The board can also have STEMMA QT connectors.
Do not use a STEMMA QT cable in this wiring plan.

| LIS3DH pin | DK or rail | Signal |
| --- | --- | --- |
| `VIN` | `3V3` | 3.3 V input to board regulator |
| `GND` | `GND` | Shared ground |
| `SCK` or `SCL` | `P0.13` | Shared SPI clock |
| `SDA` or `SDI` | `P0.15` | Shared SPI data into sensor |
| `SDO` | `P0.14` | SPI data out of sensor |
| `CS` | `P0.21` | Sensor chip select |
| `INT1` | `P0.23` | Motion interrupt |
| `INT2` | Not connected | Reserved |
| `3Vo` | Not connected | Board regulated output |

The exact silk labels can differ.
Use the function names in the table, not connector position.
If you cannot identify a pin, stop and use the Adafruit product documentation.

The LIS3DH datasheet requires `100 nF` and `10 µF` supply decoupling close to
the sensor's `VDD` pin. Do not remove the breakout's local capacitors. If a
different breakout lacks them, stop and add the specified capacitors before
testing. Keep `VDD` and `VDD_IO` powered together through the breakout's
documented power path.

### 10.1 Wire the LIS3DH

With USB power removed:

1. Connect `VIN` to `3V3`.
2. Connect `GND` to `GND`.
3. Connect sensor `SCK` or `SCL` to DK `P0.13`.
4. Connect sensor `SDA` or `SDI` to DK `P0.15`.
5. Connect sensor `SDO` to DK `P0.14`.
6. Connect sensor `CS` to DK `P0.21`.
7. Connect sensor `INT1` to DK `P0.23`.
8. Leave `INT2` unconnected.
9. Leave `3Vo` unconnected.
10. Check that sensor `CS` is not connected to display `CS`.

The LIS3DH and display share `SCK` and controller-to-device data.
They must have separate chip-select pins.
Only one chip-select pin can be active during an SPI transaction.

### 10.2 Motion test

The motion test needs a later LIS3DH driver.
The existing blinky app cannot read the sensor.

When a motion test is available:

1. Put the board on a stable table.
2. Connect USB power.
3. Run the sensor identification test.
4. Confirm that the sensor responds.
5. Enable the `INT1` interrupt test.
6. Move the board by hand.
7. Record the interrupt event.
8. Stop moving the board.
9. Record whether the interrupt clears or stays active.

If the sensor does not respond, test SPI wiring before testing interrupts.
If the sensor responds but `INT1` does not change, check the interrupt
configuration and the `INT1` wire.

## 11. Connect the DCF-77 receiver

The local manual is stored at
`temp-resources/docs/dcf77/dcf77-manual-dump.html`.
It describes the Drhomeam DCF-1060N-800 receiver.

The manual lists these pins:

- `PON`
- `OUT`
- `GND`
- `VDD`

The manual states a 1.1 V to 3.3 V supply range.
The photographed board has the same four labeled pads as the manual.
The measured `PON` behavior is active-low:

- `PON = GND`: receiver enabled, measured current `68.3 µA`
- `PON = VDD`: receiver disabled, measured current `0.0 µA`

| DCF pin | DK or rail | Signal |
| --- | --- | --- |
| `VDD` | `3V3` | Receiver power |
| `GND` | `GND` | Shared ground |
| `OUT` | Not connected until logic level is measured | DCF-77 digital output |
| `PON` | `P0.24` after safe-state firmware is ready | Active-low power control |

### 11.1 Confirm the DCF board

Do not connect the DCF board by connector position.

1. Compare the board labels with `PON`, `OUT`, `GND`, and `VDD`.
2. Confirm that the physical board matches the photographed board.
3. Confirm that the antenna wires are attached.
4. Use only `3.3 V` for `VDD`.
5. Use `PON = GND` to enable the receiver.
6. Use `PON = VDD` to disable the receiver.
7. Stop if any label, antenna connection, or board layout differs.

The current measurements above are prototype evidence for this physical
module. They do not replace a manufacturer datasheet for another module.

### 11.2 Wire the receiver

With USB power removed:

1. Connect `VDD` to `3V3`.
2. Connect `GND` to `GND`.
3. Connect `PON` to `GND` for the isolated enabled-state test.
4. Keep `OUT` disconnected from the DK.
5. Measure `OUT` with a multimeter and record that a changing reading indicates
   pulses, not a verified logic-high voltage.
6. Use the logic analyzer to measure the minimum and maximum `OUT` levels.
7. Connect `OUT` to DK `P0.25` only after the measured maximum is within the
   nRF52832 input range.

For later power control, configure `P0.24` as an output high before connecting
it to `PON`, so the receiver starts disabled. Drive `P0.24` low only when a
DCF reception window is intentionally enabled. If firmware cannot guarantee
that startup state, leave `PON` disconnected and operate the receiver only
with a manual jumper.

### 11.3 DCF-77 test

The DCF test needs a later receiver driver or a logic analyzer.
The existing blinky app cannot decode DCF-77.

When the test is available:

1. Place the ferrite antenna away from the computer and USB cable.
2. Rotate the antenna slowly through several orientations.
3. Keep the receiver still during each observation.
4. Record the time, location, antenna orientation, and nearby equipment.
5. Observe `OUT` for at least several minutes.
6. Continue for a longer observation window if no signal appears.
7. Record whether pulses repeat once per second.
8. Do not treat one missing frame as a wiring failure.

DCF-77 reception depends on location, time, antenna direction, interference,
and weather conditions.
A valid time frame can require repeated observations.

## 12. Connect the buzzer driver

The MCU pin controls a transistor.
The transistor carries buzzer current.
The MCU does not drive the buzzer directly.

### 12.1 Transistor pin warning

The pin order differs between `2N3904` and `BC547` packages.
Do not assume that the flat side has the same order for both parts.

Before wiring:

1. Read the marking on the transistor.
2. Find its datasheet pinout.
3. Identify `B` for base, `C` for collector, and `E` for emitter.
4. Write the pin order in the test record.
5. Stop if the marking is unreadable.

Also verify the exact buzzer marking and its voltage and current requirements.
Stop the buzzer test if those requirements are unavailable.

### 12.2 Buzzer circuit

Use this circuit:

```text
                 buzzer
3V3 or approved + ----+---- collector (C)
                       |
                       |  NPN transistor
                       +---------------- emitter (E) ---- GND

P0.12 ---- 1 kOhm ---- base (B)

Flyback diode across buzzer:
  diode cathode (marked band) ---- buzzer positive
  diode anode -------------------- transistor collector
```

With USB power removed:

1. Connect the transistor emitter to `GND`.
2. Connect the transistor collector to the buzzer negative terminal.
3. Connect the buzzer positive terminal to the approved supply.
4. Connect DK `P0.12` through the 1 kOhm resistor to the transistor base.
5. Connect the diode banded end to the buzzer positive terminal.
6. Connect the other diode end to the transistor collector.
7. Check that no buzzer terminal connects directly to `P0.12`.
8. Check that the transistor pin order matches its datasheet.

Do not power the buzzer from the nRF52 GPIO.
Do not omit the diode.
Do not add 5 V until the buzzer datasheet and power budget are checked.

### 12.3 Buzzer test

The buzzer test needs a later GPIO test firmware.
The blinky app does not configure `P0.12` as `BUZZER_EN`.

When the test is available:

1. Start with the transistor control low.
2. Connect USB power.
3. Set `BUZZER_EN` high for a short test.
4. Set `BUZZER_EN` low.
5. Confirm that the buzzer responds.
6. Confirm that the DK does not reset.
7. Stop the test if any part heats.

If the buzzer is silent, test the transistor circuit with power removed.
Check the transistor pinout, resistor value, diode direction, and ground.
Do not solve silence by connecting the buzzer directly to a GPIO.

## 13. Full integration order

Use this order after each individual stage passes:

1. DK only and blinky.
2. DK plus display power and control wiring.
3. DK plus display and LIS3DH.
4. DK plus DCF-77 receiver.
5. DK plus buzzer driver.
6. All modules together.

At each stage:

1. Remove USB power.
2. Add only the next module.
3. Compare every wire with the pin table.
4. Check `3V3` and `GND` for shorts.
5. Connect USB power.
6. Run the test for that stage.
7. Record the result.
8. Stop if the expected result does not occur.

Do not add another module to hide a failed test.
Return to the last stage that passed.

## 14. Fault isolation

| Symptom | First action | Check next |
| --- | --- | --- |
| No DK power | Remove USB and inspect cable and DK | USB data cable, DK power LED, shorted rail |
| Part becomes hot | Remove USB immediately | Reversed power, shorted rail, wrong module pin |
| Flash fails | Disconnect peripherals | DK USB, OpenOCD, SDK variables, erase and flash |
| Display stays blank | Remove USB before rewiring | `VCC`, `GND`, `CS`, `DC`, `RST`, `BUSY`, `DIN`, `CLK` |
| Display shows old image | Run a full refresh | Display reset, test firmware, panel state |
| LIS3DH gives no data | Remove USB before rewiring | SPI clock, MOSI, MISO, sensor CS, sensor power |
| LIS3DH gives data but no interrupt | Keep power off while checking | `INT1` wire, interrupt setup, motion threshold |
| DCF-77 gives no frame | Record conditions first | `VDD`, `GND`, `OUT`, antenna direction, interference |
| DCF `OUT` level is unknown | Keep `OUT` disconnected from the DK | Logic analyzer level range, receiver supply, ground |
| Meter shows unexpected current | Turn the supply output off immediately | Meter socket, current mode, series path, shorts, meter fuse |
| Buzzer is silent | Remove USB before rewiring | Transistor pinout, resistor, diode, approved supply |
| DK resets with buzzer | Remove USB immediately | Buzzer current, ground wiring, supply noise, diode |

For every later-stage fault:

1. Disconnect USB power.
2. Remove the newest module.
3. Restore the previous wiring.
4. Repeat the previous passing test.
5. Change one wire or setting at a time.

## 15. Safe teardown

1. Stop all firmware tests.
2. Set buzzer control low if firmware is running.
3. Disconnect the DK USB cable.
4. Remove the Li-Po and TP4056 if they were used for a later experiment.
5. Remove signal wires.
6. Remove power and ground wires last.
7. Store the e-paper panel face up.
8. Store the DCF antenna away from magnets and heavy objects.
9. Record any changed wiring before disassembling the breadboard.

## 16. Completion checklist

Mark a box only after the test result is recorded.

- [ ] DK powers from USB without external modules.
- [ ] Existing blinky firmware builds.
- [ ] Existing blinky firmware flashes.
- [ ] DK LED changes state about every 500 ms.
- [ ] Display wiring matches the project pin map.
- [ ] Display `BS1` is set to `0` for four-wire SPI.
- [ ] Display test firmware refreshes a known pattern.
- [ ] Display retains its image after power removal.
- [ ] LIS3DH wiring matches the project pin map.
- [ ] LIS3DH identification test passes.
- [ ] LIS3DH `INT1` event is observed during movement.
- [ ] DCF receiver supply and `PON` states are confirmed.
- [ ] DCF enabled current is recorded.
- [ ] DCF disabled current is recorded.
- [ ] DCF `OUT` logic levels are measured before MCU connection.
- [ ] DCF antenna orientation and test conditions are recorded.
- [ ] DCF output observation is recorded.
- [ ] Buzzer transistor pinout is recorded.
- [ ] Exact buzzer marking and electrical rating are recorded.
- [ ] Buzzer resistor and diode polarity are confirmed.
- [ ] Buzzer test runs without a DK reset.
- [ ] Full integration test completes.
- [ ] All wiring differences are recorded.

## 17. Test record

Copy this template into your project notes.

```text
Date:
Builder:
Firmware commit:

Controller:
  DK board:
  DK revision:
  USB port:

Display:
  Model:
  Revision:
  Pin map changes:

LIS3DH:
  Product:
  Board labels:
  Pin map changes:

DCF-77:
  Model:
  Manual:
  PON polarity confirmed:
  Supply voltage:
  Enabled state and current:
  Disabled state and current:
  OUT minimum voltage:
  OUT maximum voltage:
  OUT logic-level instrument:
  Location:
  Antenna orientation:
  Test time:
  Nearby interference:

Buzzer:
  Model:
  Exact marking and rating:
  Transistor:
  Transistor pin order:
  Resistor:
  Diode direction:
  Supply used:

Test results:
  DK blinky:
  Display:
  LIS3DH data:
  LIS3DH interrupt:
  DCF-77 output:
  Buzzer:
  Full integration:

Safety checks:
  KORAD voltage and current limit:
  Multimeter current range and sockets:
  Pre-power short check:
  BS1 four-wire SPI setting:

Unresolved faults:

Wiring differences:
  Source:
  Documented connection:
  Actual connection:
  Reason:
```

## 18. References

- [Prototype BOM](../bom/proto-bom.md)
- [Hardware and tool inventory](../inventory/prototype-tools.md)
- [Existing nRF52 DK blinky README](../../firmware/apps/blinky/README.md)
- [Adafruit LIS3DH product 2809](https://adafru.it/2809)
- [Waveshare 2.13inch e-Paper HAT](https://www.waveshare.com/2.13inch-e-paper-hat.htm)
- [Local DCF-77 manual dump](../../temp-resources/docs/dcf77/dcf77-manual-dump.html)

This guide does not replace the datasheet or manual for a physical board.
If a physical label conflicts with this guide, remove power and investigate the
conflict before continuing.
