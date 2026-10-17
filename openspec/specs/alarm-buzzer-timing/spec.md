# alarm-buzzer-timing Specification

## Purpose
Keep the alarm buzzer on its 1 s on / 1 s off pattern and keep button silence responsive while the e-paper refreshes.

## Requirements

### Requirement: Buzzer pattern holds during display refresh
While an alarm sounds, the buzzer SHALL follow 1 s on / 1 s off from the first second of the alarm. A display refresh that starts at the same time SHALL NOT turn the pattern into a continuous tone.

#### Scenario: Alarm starts on a minute change
- **WHEN** an alarm starts on the minute change and the display refreshes at that moment
- **THEN** `BUZZER_EN` shows 1.0 s bursts separated by 1.0 s gaps from the first burst, each within 100 ms

#### Scenario: No button, no stop before timeout
- **WHEN** nobody presses a button
- **THEN** the alarm stops 59 s after the first burst starts, and the last burst is a 1 s burst

### Requirement: Silence stays responsive during refresh
A button press while the display refreshes SHALL silence the alarm within 100 ms.

#### Scenario: Press during refresh
- **WHEN** a button is pressed while the display refreshes
- **THEN** `BUZZER_EN` goes quiet within 100 ms
