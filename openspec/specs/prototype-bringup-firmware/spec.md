## Purpose

Provides firmware for the nRF52 DK breadboard prototype that holds every
peripheral in a safe state and runs one operator-selected peripheral test at a
time, so the breadboard guide's display, motion, DCF-77, and buzzer checks can
be executed and recorded.

## Requirements

### Requirement: Firmware builds and flashes with the project toolchain
The bring-up firmware SHALL build for the nRF52 DK (`PCA10040`, nRF52832) with nRF5 SDK 17.1.0 from `NRF5_SDK_DIR` and `arm-none-eabi-gcc` from `GNU_INSTALL_ROOT`, without a SoftDevice, Zephyr, or an RTOS. It SHALL provide build, flash, and erase commands that match the existing blinky app and flash through OpenOCD.

#### Scenario: Operator builds and flashes
- **WHEN** the operator runs `make` and then `make flash` in the bring-up app directory with the DK connected over USB
- **THEN** the build produces `armgcc/_build/nrf52832_xxaa.hex` without errors and OpenOCD programs and verifies it

#### Scenario: SDK variable missing
- **WHEN** `NRF5_SDK_DIR` is not set
- **THEN** the build stops with an error that names `NRF5_SDK_DIR`

### Requirement: Firmware drives safe control states at reset
Before any other peripheral setup or test, the firmware SHALL configure `DCF_PON` (`P0.24`) as output high, `EPD_CS` (`P0.11`) high, `SENSOR_CS` (`P0.30`) high, and `BUZZER_EN` (`P0.31`) low, matching guide section 6.2. These states SHALL hold whenever no test is running and after every test ends, including a test that fails or times out.

#### Scenario: Board resets with peripherals wired
- **WHEN** the DK resets or powers up with the firmware flashed
- **THEN** the receiver is disabled, neither SPI device is selected, and the buzzer is silent before the firmware prints its start banner

#### Scenario: Test ends with an error
- **WHEN** any test stops because of an error or timeout
- **THEN** the four control signals return to their safe states before the firmware reports the result

### Requirement: Firmware uses only the guide pin map
The firmware SHALL use only the project pins in guide section 6 for peripherals, plus DK buttons 1 to 4, DK LEDs, and the DK UART for operator interaction. It SHALL NOT drive any other reserved DK pin. Signals on `P0.22` to `P0.31` SHALL stay at or below 10 kHz.

#### Scenario: Pin assignments are reviewed
- **WHEN** a reviewer compares the firmware pin definitions with guide section 6
- **THEN** every peripheral signal matches the guide table and no SPI clock or data signal uses `P0.22` to `P0.31`

### Requirement: Operator selects one test at a time
The firmware SHALL map DK button 1 to the display test, button 2 to the LIS3DH test, button 3 to the DCF-77 test, and button 4 to the buzzer test. Only one test SHALL run at a time. A button press during a running test SHALL be reported and ignored, except the button that started a continuous test, which SHALL stop it.

#### Scenario: Operator starts a test
- **WHEN** the firmware is idle and the operator presses a test button
- **THEN** the firmware reports the test name over UART and runs only that test

#### Scenario: Operator presses another button during a test
- **WHEN** a test is running and the operator presses a different test button
- **THEN** the firmware reports that a test is already running and the running test continues

### Requirement: Firmware reports progress and results over UART
The firmware SHALL print human-readable text on the DK virtual COM port at 115200 baud, 8N1, readable with `picocom`. It SHALL print a start banner with the firmware name, build identifier, and button map. Each test SHALL print its start, each observation, and a final `PASS`, `FAIL`, or `STOPPED` line with a reason.

#### Scenario: Operator opens the serial console
- **WHEN** the operator opens the DK serial port with `picocom -b 115200` and resets the DK
- **THEN** the banner and button map appear

#### Scenario: Test finishes
- **WHEN** any test finishes
- **THEN** the last line for that test starts with `PASS`, `FAIL`, or `STOPPED` and names the test

### Requirement: Display test shows a documented pattern
The display test SHALL reset the Waveshare 2.13" panel, run a full refresh, and draw a documented pattern: a black border with a black-and-white checkerboard inside. It SHALL wait on `EPD_BUSY` with a timeout and SHALL put the panel into deep sleep after the refresh.

#### Scenario: Display is wired correctly
- **WHEN** the operator runs the display test with the display wired per guide section 9
- **THEN** the panel shows the border and checkerboard, the firmware reports `PASS`, and the image stays after USB power is removed

#### Scenario: Busy never clears
- **WHEN** `EPD_BUSY` does not clear within the timeout
- **THEN** the firmware reports `FAIL` with a busy-timeout reason and returns all signals to safe states

### Requirement: LIS3DH test identifies the sensor and reports motion interrupts
The LIS3DH test SHALL read `WHO_AM_I` over SPI and compare it with `0x33`. On a match, it SHALL configure a motion threshold on `INT1` and report each `INT1` event with its source axes until the operator stops the test or a timeout expires.

#### Scenario: Sensor responds
- **WHEN** the sensor is wired per guide section 10 and the operator starts the test
- **THEN** the firmware reports the `WHO_AM_I` value and that identification passed

#### Scenario: Sensor does not respond
- **WHEN** `WHO_AM_I` is not `0x33`
- **THEN** the firmware reports `FAIL` with the read value and does not enable the interrupt test

#### Scenario: Board is moved
- **WHEN** identification passed and the operator moves the board
- **THEN** the firmware reports an `INT1` event, and the events stop when the board is still

### Requirement: DCF-77 test enables the receiver only during the test window
The DCF-77 test SHALL drive `DCF_PON` low only while the test runs, and SHALL drive it high when the operator stops the test or a timeout expires. It SHALL log each `DCF_OUT` pulse width in milliseconds, its classification as a `0` bit, a `1` bit, or invalid, and each detected minute marker.

#### Scenario: Receiver produces pulses
- **WHEN** the receiver is enabled and `DCF_OUT` is connected after its logic level was checked
- **THEN** the firmware logs about one pulse per second with its width and classification

#### Scenario: Operator stops the test
- **WHEN** the operator presses button 3 again or the timeout expires
- **THEN** `DCF_PON` returns high and the firmware reports `STOPPED` with counts of valid, invalid, and minute-marker pulses

#### Scenario: No signal is received
- **WHEN** no valid pulse arrives during the window
- **THEN** the firmware reports zero valid pulses as an observation, not as a wiring failure

### Requirement: Buzzer test is short and bounded
The buzzer test SHALL drive `BUZZER_EN` for a bounded time of at most one second per step: first a steady high level, then a square-wave tone below 10 kHz, with a report before each step. `BUZZER_EN` SHALL be low between steps and after the test.

#### Scenario: Buzzer circuit works
- **WHEN** the buzzer driver is wired per guide section 12 and the operator runs the test
- **THEN** the firmware reports each step, the buzzer sounds during at least one step, and `BUZZER_EN` is low at the end

#### Scenario: DK resets during the buzzer test
- **WHEN** the DK resets while the buzzer sounds
- **THEN** the firmware reports the reset reason in the next start banner so the operator can record it
