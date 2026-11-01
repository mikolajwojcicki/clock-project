# 2026-11-01 Web Bluetooth schedule page bench results

Setup: nRF52 DK, clock app `d798b6d-dirty` on S112 7.2.0. Page served from `web/` with `python3 -m http.server 8000` on `localhost`. Browser: Chromium 150.0.7871.124 on NixOS (BlueZ). UART console captured on the DK J-Link port, 115200 baud.

## Browser

|Item|Result|
|---|---|
|Flag needed|`chrome://flags/#enable-experimental-web-platform-features` enabled|
|Secure context|`http://localhost:8000` accepted|

## Results

|Task|Check|Result|
|---|---|---|
|3.1|Page served and opened|ok, flag above|
|3.2|2 s DK button hold, then Connect|`ble: adv start` (no `adv cmd` line, so it was the button hold), `ble: connect`; page showed connected and the table|
|3.3|Sunday slot 1 set to 23:44, Upload|page showed success; console `ble: write ok`, then `alarm 7 1 23:44 on`|
|3.4|Alarm at set time|`ALARM 23:44 niedziela` at 23:44:00; buzzer sounded; `ALARM stop (button)` at 23:44:14|
|3.5|Same-day duplicate|both slots marked, Upload greyed out (user observation)|
|3.6|Idle drop|page showed disconnected and kept the edits (user observation); earlier capture: `ble: connect` at 94.0 s, `ble: disconnect` at 154.2 s, 60.2 s with no write|

## Console excerpt (3.2 to 3.4)

```text
2026-11-01 23:43:12 niedziela
ble: adv start
2026-11-01 23:43:19 niedziela
ble: connect
2026-11-01 23:43:56 niedziela
ble: write ok
alarm 7 1 23:44 on
2026-11-01 23:44:00 niedziela
ALARM 23:44 niedziela
ble: disconnect
2026-11-01 23:44:14 niedziela
ALARM stop (button)
```

## Notes

- Weekday numbering: the page's Sunday slot became `alarm 7`, which matches the firmware (1 = Monday). The task text said Monday because the clock date was set so the test could run today; Sunday was used.
- The alarm start dropped the link in the same second (alarm priority, as in the firmware spec).
- Not verified: the page's own read-back comparison message was not captured in the console log.
- Earlier in the session the DK dropped off USB and the UART showed garbled lines for a while; the capture was restarted with a reconnecting script. Cause not identified.
