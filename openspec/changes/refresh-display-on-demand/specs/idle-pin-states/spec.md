# Spec Delta

## MODIFIED Requirements

### Requirement: Peripheral traffic is unchanged
The display and the accelerometer SHALL work as before the change: the panel refreshes according to the display refresh policy, the accelerometer identifies itself, and motion during an alarm still silences it.

#### Scenario: Accelerometer after the change
- **WHEN** the clock starts with the LIS3DH fitted
- **THEN** the accelerometer passes its identity check and motion dismiss works
