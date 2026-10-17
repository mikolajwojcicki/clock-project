# alarm-schedule Specification

## Purpose
Per-weekday alarm table, minute-change trigger, buzzer on/off pattern with timeout, and button silence on the nRF52 clock app.

## Requirements

### Requirement: Per-weekday alarm table
The firmware SHALL hold one alarm slot per weekday (Monday to Sunday) in RAM. Each slot SHALL have an HH:MM time and an enable flag. All slots SHALL start disabled after reset. The table SHALL be a plain struct array with set/get functions so that a later BLE service can fill it without changing the alarm logic.

#### Scenario: Slots start disabled
- **WHEN** the firmware starts
- **THEN** all seven slots are disabled and no alarm fires

#### Scenario: Slots are independent
- **WHEN** Monday is set to 07:00 on and Tuesday to 07:30 on
- **THEN** each weekday keeps its own time and flag

### Requirement: UART alarm commands
The clock app SHALL accept `alarm <1-7> HH:MM on|off` on the console, where 1 is Monday and 7 is Sunday. It SHALL accept `alarm?` and print all seven slots. Invalid input SHALL leave the table unchanged and print a usage line.

#### Scenario: Set an alarm
- **WHEN** the host sends `alarm 3 06:45 on`
- **THEN** the Wednesday slot holds 06:45 enabled and the firmware replies with that slot

#### Scenario: Invalid command
- **WHEN** the host sends `alarm 8 25:61 on`
- **THEN** the table is unchanged and the firmware prints a usage line

#### Scenario: List
- **WHEN** the host sends `alarm?`
- **THEN** the firmware prints seven lines, one per weekday, with time and on/off

### Requirement: Trigger rule
The firmware SHALL check the table once per minute change, using the weekday and HH:MM of the clock. An enabled slot whose time equals the current HH:MM on the current weekday SHALL start the alarm once for that minute. Setting the clock into a matching minute SHALL also start the alarm. A disabled slot SHALL never start it.

#### Scenario: Match
- **WHEN** the clock reaches 06:45:00 on Wednesday and the Wednesday slot is 06:45 enabled
- **THEN** the alarm starts and the console prints an `ALARM` line

#### Scenario: Wrong weekday
- **WHEN** the clock reaches 06:45 on Thursday and only Wednesday is set to 06:45
- **THEN** the alarm does not start

#### Scenario: Once per minute
- **WHEN** the alarm has started at 06:45:00 and been silenced at 06:45:10
- **THEN** it does not start again in the same minute

### Requirement: Buzzer pattern and timeout
While the alarm is active, the buzzer SHALL sound 1 s on and 1 s off. The alarm SHALL stop by itself after 60 s. If a new match occurs while the alarm is sounding, the 60 s timer SHALL restart.

#### Scenario: Pattern
- **WHEN** the alarm is active
- **THEN** BUZZER_EN alternates 1 s on and 1 s off

#### Scenario: Auto-stop
- **WHEN** 60 s pass with no button press
- **THEN** the buzzer is off and the console prints a stop line

### Requirement: Button silences the alarm
Any DK button SHALL silence an active alarm. The alarm code SHALL read buttons only through one board function, `board_any_button_pressed()`, so that the final board can map it to its own pins. A button press with no active alarm SHALL do nothing.

#### Scenario: Silence
- **WHEN** the alarm is active and a button is pressed
- **THEN** the buzzer goes off within 100 ms and the console prints a stop line

#### Scenario: Idle press
- **WHEN** no alarm is active and a button is pressed
- **THEN** nothing happens

### Requirement: Alarm state is lost on reset
The firmware SHALL NOT persist the alarm table. After a reset during an active alarm, the buzzer SHALL be off and the table disabled.

#### Scenario: Reset while sounding
- **WHEN** the DK is reset while the alarm sounds
- **THEN** the buzzer stays off after start-up
