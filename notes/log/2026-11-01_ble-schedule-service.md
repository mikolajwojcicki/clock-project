# 2026-11-01 BLE schedule service bench results

Setup: nRF52 DK, S112 7.2.0, clock app at `d798b6d`. Central: laptop BlueZ + bleak. Scripts hands-free via the UART command `ble adv` (same window as the 2 s button hold). Times are from the bleak and UART log, in seconds.

## Results

|Check|Result|
|---|---|
|No advertising after boot|Scan found nothing until the hold or `ble adv` (earlier runs)|
|Window length|`ble: adv cmd` at 0.5, `ble: adv stop` at 60.5: 60 s|
|Idle link|Scan found `ShakeClock`, link dropped at 61.8 with no write. UART `connect` line is late (script polls UART between BLE calls), so the exact connect time is not logged. [INFERENCE] about 60 s after connect|
|Link after write|Write ok at 36.3, disconnect at 66.4: 30.1 s (earlier run: 30.4 s)|
|Alarm during link|Write at 13.1, `ALARM 12:01` at 28.1, link dropped 0.3 s later; alarm stopped on timeout 60 s after start|
|Write in the matching minute|Write ok at clock 12:01:28 with slot 12:01 on. Log runs to 12:02:48 with no `ALARM` line|
|Alarm priority stop|On alarm start, advertising/link are stopped (`ble_sched_stop`). Verified: connected alarm drops the link|

## Flash / RAM (task 2.4)

|Item|Value|
|---|---|
|SoftDevice|S112 7.2.0, flash 0x00000-0x18FFF (0x18000 = 96 KiB)|
|App FLASH|ORIGIN 0x19000, LENGTH 0x66000 (ends before schedule page 0x7F000)|
|App RAM|ORIGIN 0x200022c8, LENGTH 0xdd38 (from `nrf_sdh_ble_enable`)|
|App sizes SLEEP_UART=1|text 36820, data 144, bss 2860 (dec 39824)|
|App sizes SLEEP_UART=0|text 29912, data 44, bss 1872 (dec 31828)|
|Makefile|`make flash_softdevice` then `make flash` via OpenOCD|

## SoftDevice regression (no BLE window)

|Check|Result|
|---|---|
|Clock + UART seconds|ok|
|E-paper refresh|`display ok ~3700 ms`|
|DCF|`dcf: window open` / timeout|
|Buzzer|fires on ALARM (user-heard earlier; timeout/button stop logged)|
|Motion dismiss|not re-exercised under SoftDevice this session|

## Known limitation (task 4.5, accepted)

A button hold that silences a sounding alarm can also open the BLE advertising window once the alarm has stopped, because the same continuous hold is still counted after silence. Accepted: nobody needs to connect over BLE while the alarm is sounding. Alarm-start still stops advertising and drops any link.

## Other gaps

- MTU 247 negotiation: BlueZ stayed at 23; the 113 B write still succeeded (likely as a long write; not confirmed).
- Stage 5 gate: `add-deep-sleep-and-current-measurement` is archived (`2026-10-28-...`); clock uses `board_sleep()` / WFE.
