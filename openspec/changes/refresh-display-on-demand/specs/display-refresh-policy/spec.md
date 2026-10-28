# Spec Delta

## Purpose
Defines when the clock app refreshes the e-paper face, so that the panel and its supply are used only when the user needs the time and the device does not spend energy on unread refreshes.

## ADDED Requirements

### Requirement: No refresh on minute change
The firmware SHALL NOT refresh the display only because the minute changed.

#### Scenario: Idle across minutes
- **WHEN** the clock runs for 10 minutes with no button press, no alarm and no time change
- **THEN** no display refresh starts

### Requirement: Button press refreshes the face
When no alarm is sounding, a button press SHALL start one refresh that shows the current time, weekday and alarm status. Holding a button SHALL NOT start a second refresh, and a press during a refresh SHALL NOT queue another.

#### Scenario: Press while idle
- **WHEN** a button is pressed at 14:32:10 with no alarm sounding
- **THEN** one refresh starts, and the face shows 14:32

#### Scenario: Button held
- **WHEN** a button is held for 5 s
- **THEN** exactly one refresh starts

#### Scenario: Press during refresh
- **WHEN** a button is pressed while a refresh is running
- **THEN** no further refresh starts after it

### Requirement: Alarm start refreshes the face
The firmware SHALL refresh the face when an alarm starts. It SHALL NOT refresh when the alarm stops, whether by button, motion or timeout, and a button press that silences an alarm SHALL NOT start a button refresh.

#### Scenario: Alarm starts
- **WHEN** an enabled alarm slot matches the clock minute
- **THEN** one refresh starts and the buzzer keeps its 1 s on / 1 s off pattern

#### Scenario: Alarm silenced by button
- **WHEN** a button silences the alarm
- **THEN** no refresh follows

### Requirement: Time changes refresh the face
The firmware SHALL refresh the face after the time is set over UART and after a DCF-77 sync is accepted.

#### Scenario: Time set over UART
- **WHEN** the operator sets the time
- **THEN** one refresh shows the new time

#### Scenario: DCF-77 sync accepted
- **WHEN** a sync is accepted
- **THEN** one refresh shows the corrected time

### Requirement: Refresh does not run inside a sync window
A refresh SHALL NOT run while a DCF-77 sync window is open. A refresh requested in a window SHALL run after the window closes.

#### Scenario: Press during window
- **WHEN** a button is pressed while a window is open
- **THEN** the refresh starts after the window closes

### Requirement: Daily energy of the policy is measured
A capture of at least 10 minutes with a button press every 60 s and no other refresh SHALL be logged with mean idle current and charge per refresh, and the daily energy estimate in `notes/log/` SHALL use these figures.

#### Scenario: Run with presses
- **WHEN** the policy run is recorded on one 3.0 V rail with the panel fitted
- **THEN** the capture shows one refresh per press and none between presses, and the log lists the daily charge with its assumed presses per day
