# dcf77-time-sync Specification

## Purpose
Define when and how the clock corrects its time from DCF-77 minute frames, and how the receiver is power-gated so it draws current only during a sync window.


## Requirements

### Requirement: Timed daily sync window
The device SHALL open one DCF-77 sync window every day at 03:00 local time, and SHALL open one when it starts with its time not yet set. A window SHALL last at most 5 minutes. Outside a window the receiver SHALL be powered off (PON high).

#### Scenario: Window opens on schedule
- **WHEN** the clock reaches 03:00:00
- **THEN** the receiver is powered and the device starts decoding the pulse stream

#### Scenario: Idle receiver is off
- **WHEN** no sync window is open
- **THEN** PON is high

#### Scenario: Window length is bounded
- **WHEN** a window has been open for 5 minutes without an accepted sync
- **THEN** the window ends and the receiver is powered off

### Requirement: Two consecutive valid frames set the time
The device SHALL change its time only after it decodes two complete frames in a row, each passing all marker, parity and field-range checks, whose decoded times are exactly one minute apart. A frame whose checks pass but whose time does not follow the previous frame SHALL NOT be accepted and SHALL NOT be used as the first frame of a new pair that already counted the rejected one.

#### Scenario: Good pair accepted
- **WHEN** two consecutive frames decode to minutes M and M+1 with valid markers and parities
- **THEN** the clock time is set from the second frame and the result is logged as success

#### Scenario: Wrong date with correct parity
- **WHEN** the generator sends a frame whose date differs from the sequence while its parity is correct
- **THEN** the pair is rejected, the clock time is unchanged, and the log names a time mismatch as the reason

### Requirement: Invalid frame leaves the time unchanged
The device SHALL reject a frame with a bad parity bit, a bad marker, a field out of range, or missing or invalid pulses. A rejected frame SHALL NOT change the time and SHALL be logged with its reason.

#### Scenario: Parity error
- **WHEN** a decoded frame fails a parity check
- **THEN** the clock time is unchanged and the log names parity as the reason

#### Scenario: No valid pulses
- **WHEN** no pair of valid frames is decoded before the window ends
- **THEN** the clock time is unchanged and the log names timeout as the reason

### Requirement: Accepted sync aligns the second boundary
On acceptance the device SHALL set the time to the decoded minute with seconds 0 and SHALL align its second boundary to the start of the minute-start pulse, so the clock second changes within 50 ms of the DCF-77 second. The receiver output delay (about 34 ms on the bench) SHALL NOT be compensated.

#### Scenario: Seconds follow the pulse
- **WHEN** a sync is accepted
- **THEN** the clock second changes within 50 ms of the next pulse start seen at the generator output

### Requirement: Retry after a failed window
After a window that ended without an accepted sync, the device SHALL retry at the next full hour, at most 3 retries per day. After the retries are used it SHALL wait for the next 03:00 window.

#### Scenario: Retry scheduled
- **WHEN** the 03:00 window ends without an accepted sync
- **THEN** another window opens at 04:00

#### Scenario: Retries exhausted
- **WHEN** the windows at 03:00, 04:00, 05:00 and 06:00 all fail
- **THEN** no further window opens until 03:00 the next day

### Requirement: Receiver powered down after window
The device SHALL power the receiver off when a window ends, whether it was accepted, rejected or timed out.

#### Scenario: Window ends
- **WHEN** a sync is accepted or the window ends
- **THEN** PON is high

### Requirement: Alarm takes precedence over a window
The device SHALL NOT open a scheduled or retry window while an alarm is sounding, and SHALL open it when the alarm stops if the window time has not passed by more than 5 minutes.

#### Scenario: Alarm at sync time
- **WHEN** the scheduled window time is reached while the alarm sounds
- **THEN** the window opens after the alarm stops, or is skipped if more than 5 minutes have passed

### Requirement: Operator can open a window and read the result
The device SHALL open a window on a UART command `dcf sync`, using the same code path as a scheduled window. For every window it SHALL print one result line stating success with the decoded time, or the rejection reason, and SHALL print the clock time before and after.

#### Scenario: Generator-driven sync
- **WHEN** the operator sends `dcf sync` while the Nucleo generator emits valid frames
- **THEN** the clock time is set to the generator time, one `dcf:` result line with before and after is printed, and PON is high afterwards

#### Scenario: Window current recorded
- **WHEN** a generator-driven window is measured
- **THEN** its current is recorded with instrument, supply and firmware commit for the stage 5 data
