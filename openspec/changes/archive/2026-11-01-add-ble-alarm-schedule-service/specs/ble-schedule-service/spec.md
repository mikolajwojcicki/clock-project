# Spec Delta

## Purpose
Let a phone or browser read and replace the 56-slot alarm table over Bluetooth Low Energy, with advertising limited to a short window that the user opens by hand.

## ADDED Requirements

### Requirement: GATT service layout
The firmware SHALL expose one custom GATT service with a 128-bit UUID `e1430d6d-6e9b-4b29-ba99-6f30cbe203d0` and one schedule characteristic (UUID offset 0x0002 from the same base) with read and write properties. It SHALL NOT offer notify or indicate. The device SHALL advertise the name `ShakeClock` and the service UUID. The characteristic SHALL be writable with long writes and with an ATT MTU of up to 247.

#### Scenario: Discovery
- **WHEN** a central connects and discovers services
- **THEN** it finds the service and one read/write schedule characteristic

#### Scenario: Name
- **WHEN** a central scans during the advertising window
- **THEN** it sees the name `ShakeClock`

### Requirement: Schedule encoding
The schedule characteristic value SHALL be 113 bytes: byte 0 is the format version (1), followed by 7 weekdays (Monday to Sunday) of 8 slots each, 2 bytes per slot as an unsigned 16-bit little-endian value. Bit 15 is the enable flag and bits 0 to 10 are the minute of the day (0 to 1439). A read SHALL return the current table in this encoding.

#### Scenario: Read
- **WHEN** Monday slot 1 is 07:00 enabled and a central reads the characteristic
- **THEN** byte 0 is 1 and bytes 1 and 2 are `0xA4 0x81` (bit 15 set, 420 minutes)

#### Scenario: Unused slot
- **WHEN** a slot is unused
- **THEN** its two bytes are `0x00 0x00`

### Requirement: Atomic validated write
A write SHALL replace the whole table or nothing. The firmware SHALL reject the write with an ATT error and leave the table and the saved record unchanged if the length is not 113, the version is not 1, a minute value is above 1439, or two enabled slots on one weekday have the same minute. An accepted write SHALL replace the table, save it to flash, and print `ble: write ok` and the table on the console.

#### Scenario: Valid write
- **WHEN** a central writes a valid 113-byte value
- **THEN** the write succeeds, the console prints `ble: write ok` and the table, and a following read returns the written value

#### Scenario: Wrong length
- **WHEN** a central writes 21 bytes
- **THEN** the write fails with an ATT error and the table is unchanged

#### Scenario: Minute out of range
- **WHEN** a slot holds minute 1440
- **THEN** the write fails with an ATT error and the table is unchanged

#### Scenario: Duplicate time
- **WHEN** two enabled Tuesday slots hold the same minute
- **THEN** the write fails with an ATT error and the table is unchanged

### Requirement: Advertising window
The firmware SHALL NOT advertise after start-up. Holding any button for 2 s while no alarm is active SHALL start connectable advertising at a 100 ms interval and 0 dBm for 60 s. The console SHALL print `ble: adv start` and `ble: adv stop`. Advertising SHALL stop when the 60 s end, or when a central connects. A button press during an active alarm SHALL only silence the alarm.

#### Scenario: Idle at start-up
- **WHEN** the firmware has just started and no button is held
- **THEN** no advertising packets are sent

#### Scenario: Open the window
- **WHEN** a button is held for 2 s with no alarm active
- **THEN** advertising starts and `ble: adv start` is printed

#### Scenario: Window ends
- **WHEN** 60 s pass with no connection
- **THEN** advertising stops and `ble: adv stop` is printed

#### Scenario: Press during alarm
- **WHEN** the alarm sounds and a button is held for 2 s
- **THEN** the alarm is silenced and advertising does not start

### Requirement: Connection policy
The link SHALL be open: no pairing or bonding. The firmware SHALL terminate the connection 30 s after the last accepted or rejected schedule write, or 60 s after connect if there was no write.

#### Scenario: Idle link
- **WHEN** a central connects and does not write for 60 s
- **THEN** the firmware disconnects it

#### Scenario: After a write
- **WHEN** a central writes and then stays silent
- **THEN** the firmware disconnects it 30 s after that write

### Requirement: Alarm takes priority
When an alarm starts, the firmware SHALL stop advertising and terminate any connection at once. The buzzer pattern, timeout and motion dismiss SHALL behave as without BLE.

#### Scenario: Alarm during a connection
- **WHEN** a central is connected and the alarm starts
- **THEN** the link is dropped and the buzzer pattern starts on time

### Requirement: Time is not writable over BLE
The BLE service SHALL expose the alarm schedule only. The clock time SHALL stay settable only by DCF-77 sync and the console.

#### Scenario: No time characteristic
- **WHEN** a central lists all characteristics
- **THEN** there is no clock-time characteristic
