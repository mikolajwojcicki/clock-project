# Spec Delta

## Purpose
Define which events wake the MCU, what is powered in each state, and how current is measured and recorded, so the ultra-low-power claim of the thesis rests on evidence.

## ADDED Requirements

### Requirement: Idle state is System ON sleep with no polling
When no event is pending, the MCU SHALL sleep in System ON low-power mode. The firmware SHALL NOT run a busy-wait or polling loop for time, buttons, accelerometer or DCF input while idle.

#### Scenario: Idle clock
- **WHEN** the clock runs with no alarm, no refresh and no DCF window
- **THEN** the MCU is asleep between wake events and the main loop does not spin

### Requirement: Wake sources are limited to defined interrupts
The MCU SHALL wake only from: the 1 s time tick from RTC1, button GPIO events, accelerometer INT1 during an alarm, the DCF output edge during a DCF window, and timers for the alarm timeout and for e-paper busy waits. The clock SHALL keep correct time and seconds across sleep.

#### Scenario: Time keeps running
- **WHEN** the clock sleeps for 10 minutes with no button press
- **THEN** the displayed time and the UART second log advance by 600 s within the RTC tolerance

#### Scenario: Button wake
- **WHEN** a button is pressed during sleep
- **THEN** the MCU wakes, handles the press and returns to sleep

### Requirement: Peripherals are powered only while used
The e-paper panel SHALL be powered and active only during a refresh and SHALL be put into its deep-sleep state afterwards. The DCF receiver SHALL be held in power-down (`DCF_PON` inactive) outside a sync window. The buzzer driver and the accelerometer SHALL be off outside an alarm. The UART SHALL NOT keep the MCU awake while idle.

#### Scenario: Between DCF windows
- **WHEN** no DCF sync window is open
- **THEN** `DCF_PON` is inactive and the DCF output causes no wake

#### Scenario: After a display refresh
- **WHEN** the e-paper refresh has finished
- **THEN** the panel is in deep sleep and `EPD_BUSY` is not being polled

### Requirement: Existing behavior is unchanged by sleep
Alarm timing, motion dismiss, DCF sync and the display content SHALL behave as before the change.

#### Scenario: Alarm from sleep
- **WHEN** the alarm time is reached while the MCU sleeps
- **THEN** the buzzer starts within 1 s and motion or a button still silences it

### Requirement: Current is measured in four states
The measurement SHALL cover: idle deep sleep, e-paper refresh, DCF window, and alarm with the buzzer on. Each state SHALL have its average current and, where practical, its duration, so that a daily energy estimate can be computed.

#### Scenario: All states measured
- **WHEN** the measurement session ends
- **THEN** a current value exists for each of the four states

#### Scenario: Idle current plausibility
- **WHEN** the idle state is measured
- **THEN** the result is compared with the nRF52832 System ON sleep figure with RTC running, and any large gap is explained in the note

#### Scenario: Supply paths separated
- **WHEN** the states are measured
- **THEN** idle is measured with the nRF alone (P22 only, no external line on its pins), and the four states are measured both on the peripheral rail alone and on one shared rail feeding the nRF and the peripherals, and the note compares the shared result with the sum of the separate results

### Requirement: Measurements are stored with conditions
Each measurement SHALL be stored as raw CSV under `notes/measurements/` with instrument, temperature, supply voltage and firmware commit. A dated note in `notes/log/` SHALL describe the setup, results and failures. The measurement SHALL state what else is on the supply path, such as DK regulators or debug interface, that could add current.

#### Scenario: Record complete
- **WHEN** a measurement is saved
- **THEN** its CSV and a record of instrument, temperature, supply and commit exist, and the log note links them

### Requirement: Software flowchart matches the sleep structure
If the implemented sleep and wake structure differs from `firmware/docs/software-flowchart.*`, the flowchart SHALL be updated in the same change.

#### Scenario: Flowchart differs
- **WHEN** the final firmware structure differs from the flowchart
- **THEN** the flowchart is updated and committed
