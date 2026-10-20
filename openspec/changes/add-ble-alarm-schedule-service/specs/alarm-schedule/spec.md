# Spec Delta

## MODIFIED Requirements

### Requirement: Per-weekday alarm table
The firmware SHALL hold eight alarm slots per weekday (Monday to Sunday) in RAM, 56 slots in total. Each slot SHALL have an HH:MM time and an enable flag. After reset with no valid saved table, all slots SHALL be disabled. Two enabled slots on the same weekday SHALL NOT have the same HH:MM. The table SHALL be a plain struct array with set/get functions addressed by (weekday, slot) so that the BLE service can fill it without changing the alarm logic.

#### Scenario: Slots start disabled
- **WHEN** the firmware starts with no valid saved table
- **THEN** all 56 slots are disabled and no alarm fires

#### Scenario: Slots are independent
- **WHEN** Monday slot 1 is set to 07:00 on and Monday slot 2 to 07:30 on
- **THEN** each slot keeps its own time and flag, and the other weekdays are unchanged

#### Scenario: Duplicate time on one weekday
- **WHEN** Monday slot 1 is 07:00 on and the host sets Monday slot 2 to 07:00 on
- **THEN** the table is unchanged and the firmware prints a usage or error line

### Requirement: UART alarm commands
The clock app SHALL accept `alarm <1-7> <1-8> HH:MM on|off`, where the first number is the weekday (1 is Monday, 7 is Sunday) and the second is the slot. It SHALL also accept the short form `alarm <1-7> HH:MM on|off` as an alias for slot 1. It SHALL accept `alarm?` and print every slot that is enabled or has a non-zero time, one line each. Invalid input SHALL leave the table unchanged and print a usage line. Every accepted change SHALL be saved to flash.

#### Scenario: Set an alarm
- **WHEN** the host sends `alarm 3 2 06:45 on`
- **THEN** Wednesday slot 2 holds 06:45 enabled and the firmware replies with that slot

#### Scenario: Short form
- **WHEN** the host sends `alarm 3 06:45 on`
- **THEN** Wednesday slot 1 holds 06:45 enabled

#### Scenario: Invalid command
- **WHEN** the host sends `alarm 8 9 25:61 on`
- **THEN** the table is unchanged and the firmware prints a usage line

#### Scenario: List
- **WHEN** the host sends `alarm?` and three slots are in use
- **THEN** the firmware prints three lines with weekday, slot, time and on/off

### Requirement: Trigger rule
The firmware SHALL check the table once per minute change, using the weekday and HH:MM of the clock. An enabled slot whose time equals the current HH:MM on the current weekday SHALL start the alarm once for that minute. Setting the clock into a matching minute SHALL also start the alarm. Writing the table (UART or BLE) SHALL NOT by itself start the alarm for the current minute. A disabled slot SHALL never start it.

#### Scenario: Match
- **WHEN** the clock reaches 06:45:00 on Wednesday and any Wednesday slot is 06:45 enabled
- **THEN** the alarm starts and the console prints an `ALARM` line

#### Scenario: Wrong weekday
- **WHEN** the clock reaches 06:45 on Thursday and only Wednesday has a 06:45 slot
- **THEN** the alarm does not start

#### Scenario: Once per minute
- **WHEN** the alarm has started at 06:45:00 and been silenced at 06:45:10
- **THEN** it does not start again in the same minute

#### Scenario: Table write in the matching minute
- **WHEN** a schedule write adds a 06:45 slot at 06:45:20 on that weekday
- **THEN** the alarm does not start for that minute

## ADDED Requirements

### Requirement: Alarm table persistence
The firmware SHALL save the alarm table to non-volatile flash after every accepted change and SHALL restore it at start-up. The saved record SHALL carry a format version and a checksum. If the record is missing, has an unknown version or fails its checksum, the firmware SHALL start with all slots disabled and SHALL NOT fail to boot. The alarm state (sounding or not) SHALL NOT be saved.

#### Scenario: Restore after reset
- **WHEN** the table is changed and the DK is reset
- **THEN** `alarm?` after start-up prints the same slots

#### Scenario: Corrupt record
- **WHEN** the saved record fails its checksum
- **THEN** all slots are disabled and the firmware runs normally

#### Scenario: Reset while sounding
- **WHEN** the DK is reset while the alarm sounds
- **THEN** the buzzer stays off after start-up and the saved table is unchanged

## REMOVED Requirements

### Requirement: Alarm state is lost on reset
**Reason**: The table is now persisted in flash. The part about the buzzer staying off after a reset moves to the new "Alarm table persistence" requirement.
**Migration**: None. No caller depends on the table being cleared at reset.
