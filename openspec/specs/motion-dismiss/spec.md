# motion-dismiss Specification

## Purpose
Silence a sounding alarm only after real physical activity, detected by the accelerometer interrupt without polling while the MCU sleeps.

## Requirements

### Requirement: Motion wakes the MCU by interrupt during an alarm
While an alarm sounds, the accelerometer SHALL signal activity on INT1 as a threshold event. The MCU SHALL wake from sleep on that signal and SHALL NOT poll the accelerometer. INT2 SHALL stay unused. Outside an alarm the accelerometer SHALL be powered down and SHALL cause no wake.

#### Scenario: Board is moved during an alarm
- **WHEN** an alarm sounds and the board is shaken above the activity threshold
- **THEN** the MCU wakes and logs an activity event for each INT1 pulse that is counted

#### Scenario: Board is still during an alarm
- **WHEN** an alarm sounds and the board rests on a table for 30 s
- **THEN** the MCU logs no wake caused by INT1

#### Scenario: No alarm
- **WHEN** no alarm sounds and the board is shaken
- **THEN** no INT1 wake occurs

### Requirement: Alarm needs N activity events in a time window
An activity event SHALL be an INT1 pulse that comes at least 300 ms after the previous counted event. While an alarm sounds, the firmware SHALL silence the alarm when N = 20 counted events fall inside a sliding window of T = 30 s. Events older than T SHALL NOT count. N and T SHALL be fixed at build time. Events from before the alarm started SHALL NOT count.

#### Scenario: Enough shaking
- **WHEN** an alarm sounds and 20 counted events occur within 30 s
- **THEN** the buzzer stops and the event count resets

#### Scenario: Too slow
- **WHEN** an alarm sounds and events arrive so slowly that fewer than 20 fall inside any 30 s window
- **THEN** the alarm keeps sounding until the normal alarm timeout

#### Scenario: One hard shake
- **WHEN** a single shake causes several INT1 pulses within 300 ms
- **THEN** they count as one event

#### Scenario: Motion before the alarm
- **WHEN** the board is shaken while no alarm sounds, then an alarm starts
- **THEN** the count at alarm start is zero

### Requirement: Buttons remain the emergency silence
A button press SHALL silence a sounding alarm at any time, regardless of the activity count.

#### Scenario: Button with no motion
- **WHEN** an alarm sounds and a button is pressed with zero activity events
- **THEN** the alarm stops

### Requirement: Dismissal behaviour is verified without volunteers
The verification SHALL use instrument measurement or a self-test only. It SHALL record a UART log and a logic-analyzer capture of INT1 and `BUZZER_EN` in `notes/log/` and `notes/measurements/`.

#### Scenario: Evidence recorded
- **WHEN** the verification run finishes
- **THEN** a dated note shows the UART log and capture for a still period, a shaking period that silences the alarm, and a button silence with no motion
