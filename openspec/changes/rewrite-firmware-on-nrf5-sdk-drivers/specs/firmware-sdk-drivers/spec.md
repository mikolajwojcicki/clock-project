## ADDED Requirements

### Requirement: Peripherals go through nrfx drivers
Product firmware SHALL access SPI, PWM, timers, RTC, UART, GPIO interrupts and the clock system only through nrfx drivers or nRF5 SDK libraries. It SHALL NOT bit-bang a bus that has a hardware peripheral.

#### Scenario: Source review
- **WHEN** `firmware/src/` and `firmware/apps/clock/src/` are searched for direct `NRF_SPIM`, `NRF_PWM`, `NRF_RTC`, `NRF_UARTE` and `NRF_TIMER` register access
- **THEN** none is found outside the SDK

### Requirement: Observable behavior is unchanged
After the rewrite, the clock app, alarm, e-paper face, buzzer tone and button silence SHALL behave as before.

#### Scenario: Clock app regression
- **WHEN** the clock is set over UART and an alarm two minutes ahead is set
- **THEN** the face refreshes each minute, the alarm sounds with the same tone and pattern, and a button silences it

#### Scenario: Bring-up regression
- **WHEN** the bring-up app is built and run
- **THEN** its existing checks (e-paper, LIS3DH, buzzer, buttons) give the same results as the recorded notes

### Requirement: Rewrite is gated by the audit
The rewrite SHALL start only when the SDK audit records the verdict `rewrite`.

#### Scenario: Verdict is keep
- **WHEN** the audit verdict is `keep`
- **THEN** this change is closed without code changes
