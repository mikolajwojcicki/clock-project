# web-schedule-upload Specification

## Purpose
A browser page that reads, edits and writes the 56-slot alarm table of the ShakeClock over Web Bluetooth, using the encoding defined by the `ble-schedule-service` capability.

## Requirements

### Requirement: Connect to the clock
The page SHALL let the user connect to a device named `ShakeClock` that advertises the schedule service UUID, on user action only. It SHALL show the connection state (disconnected, connecting, connected) and SHALL tell the user to hold a button for 2 s to open the advertising window when no device is found. It SHALL report an unsupported browser instead of failing silently.

#### Scenario: Connect during window
- **WHEN** the advertising window is open and the user presses Connect and picks `ShakeClock`
- **THEN** the page shows connected and loads the current table

#### Scenario: No device
- **WHEN** no `ShakeClock` is found or the user cancels the chooser
- **THEN** the page stays disconnected and shows the hint to hold a button for 2 s

#### Scenario: No Web Bluetooth
- **WHEN** the browser has no Web Bluetooth support
- **THEN** the page shows a message naming the missing support and disables Connect

### Requirement: Show and edit the schedule
The page SHALL show Monday to Sunday with 8 slots per day, each with an enable toggle and an `HH:MM` time. After a connect it SHALL display the table read from the clock. A slot that is unused on the clock (`0x0000`) SHALL show as empty and disabled.

#### Scenario: Load
- **WHEN** the clock holds Monday slot 1 at 07:00 enabled
- **THEN** the page shows Monday slot 1 as 07:00 with the toggle on

#### Scenario: Edit
- **WHEN** the user changes a time or toggle
- **THEN** the page marks the table as modified and not yet saved to the clock

### Requirement: Validate before upload
The page SHALL refuse to upload, and SHALL mark the offending slots, when two enabled slots on one weekday have the same time. The same time on different weekdays SHALL be allowed.

#### Scenario: Duplicate on one day
- **WHEN** two enabled Tuesday slots are both 06:30
- **THEN** both are marked and Upload is disabled

#### Scenario: Same time on two days
- **WHEN** Monday and Tuesday both have an enabled slot at 06:30
- **THEN** Upload stays enabled

### Requirement: Upload with the shared encoding
On Upload the page SHALL write one 113-byte value in the encoding of `ble-schedule-service`: byte 0 is 1, then 7 weekdays (Monday first) of 8 little-endian 16-bit slots, bit 15 the enable flag and bits 0 to 10 the minute of the day. After the write the page SHALL read the characteristic back and show success only if the value equals what it wrote. A rejected write or a mismatch SHALL be shown as an error, and the page SHALL keep the edited table.

#### Scenario: Upload changes the alarm
- **WHEN** the user sets Monday slot 1 to 07:00 enabled and presses Upload
- **THEN** the bytes written start `0x01 0xA4 0x81`, the read-back matches, the page shows success, and the clock console prints `ble: write ok`

#### Scenario: Rejected write
- **WHEN** the clock rejects the write with an ATT error
- **THEN** the page shows an error and the edited table stays on screen

#### Scenario: Link dropped
- **WHEN** the clock drops the connection (idle timeout or alarm start) before or during Upload
- **THEN** the page shows disconnected, keeps the edited table, and the user can reconnect and upload again

### Requirement: Clock time is not touched
The page SHALL NOT offer to set the clock time. It SHALL touch only the schedule characteristic.

#### Scenario: No time control
- **WHEN** the user looks at the page
- **THEN** there is no control that sets the clock time
