# 2026-11-01 BLE schedule service bench results

Setup: nRF52 DK, S112 7.2.0, clock app at `3c5a96a`. Central: laptop BlueZ + bleak. Scripts hands-free via the UART command `ble adv` (same window as the 2 s button hold). Times are from the bleak and UART log, in seconds.

## Results

|Check|Result|
|---|---|
|No advertising after boot|Scan found nothing until the hold or `ble adv` (earlier runs)|
|Window length|`ble: adv cmd` at 0.5, `ble: adv stop` at 60.5: 60 s|
|Idle link|Scan found `ShakeClock`, link dropped at 61.8 with no write. UART `connect` line is late (script polls UART between BLE calls), so the exact connect time is not logged. [INFERENCE] about 60 s after connect|
|Link after write|Write ok at 36.3, disconnect at 66.4: 30.1 s (earlier run: 30.4 s)|
|Alarm during link|Write at 13.1, `ALARM 12:01` at 28.1, link dropped 0.3 s later; alarm stopped on timeout 60 s after start|
|Write in the matching minute|Write ok at clock 12:01:28 with slot 12:01 on. Log runs to 12:02:48 with no `ALARM` line|

## Not verified
- Buzzer by ear during the connected alarm.
- A button hold during an alarm only silences it (task 4.5).
- MTU 247 negotiation: BlueZ stayed at 23, the 113 B write passed as a long write (not confirmed).
- Task 2.4: flash and RAM sizes, motion dismiss under the SoftDevice.
