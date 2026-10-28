# Spec Delta

## MODIFIED Requirements

### Requirement: Observable behavior is unchanged
After the rewrite, the clock app, alarm, e-paper face, buzzer tone and button silence SHALL behave as before.

#### Scenario: Clock app regression
- **WHEN** the clock is set over UART and an alarm two minutes ahead is set
- **THEN** the face refreshes when the time is set and when the alarm starts, the alarm sounds with the same tone and pattern, and a button silences it

#### Scenario: Bring-up regression
- **WHEN** the bring-up app is built and run
- **THEN** its existing checks (e-paper, LIS3DH, buzzer, buttons) give the same results as the recorded notes
