# 2026-11-02 refresh on demand: policy run

Firmware `8c415fe` (`SLEEP_UART=0`). Configuration C (one 3.0 V rail from the PPK2 Source Meter on `P22`, panel and DCF generator fitted, DK USB and SWD ribbon unplugged). PPK2 S/N `FACEF4EE8A0A`, Data Logger, 100 kS/s, 557 s. Pack: [`ppk-20261102T174048`](../measurements/ppk-20261102T174048.ppkz) (42.9 MB, packed with `ppk-archive.sh pack`; the sidecar `.json` records the CSV SHA-256). Windows below come from 10 ms bin means.

The author pressed a button about every 60 s from 78 s on. Capture started at power-on, so the boot refresh is the first event.

## Result

| Quantity | Value |
| --- | --- |
| Idle current, outside events | 74.8 µA (99.9 % of bins below 99.2 µA) |
| Refreshes in the run | 10 events: boot (0.4 s), then 78, 137, 193, 255, 313, 379, 436, 493, 553 s |
| Spacing of the presses | 56-66 s: one refresh per press |
| Activity between presses | none; no refresh and no step above 118 µA (10 ms bins) |
| Full refresh, 3.73 s | 313, 436, 493, 553 s: 16.89-16.94 mC gross, 16.58-16.63 mC net of idle |

No refresh per minute: the policy removes the 1440 refreshes a day of C9.

## Which refreshes count

`D3` (`EPD_BUSY`) was wired to the PPK2 for the first part of the run. The panel then did not refresh fully (author's observation: the screen did not update entirely). The author unplugged `D3` at about 380 s; its last edge is at 379.6 s. The panel is sensitive to the extra load on `EPD_BUSY`.

| Events | Shape | Use |
| --- | --- | --- |
| boot 0.4 s, 78 s | 11.44 s at 10-12 mA, 115-136 mC | not used: `EPD_BUSY` timeout path |
| 137, 193, 255, 379 s | 1.44 s at 6.5 mA, 9.4-9.7 mC | not used: aborted refresh |
| 313, 436, 493, 553 s | 3.73 s at 4.5 mA, 16.9 mC | used; matches C9 (16.75 mC) |

One of the four used events (313 s) is from before the `D3` unplug, and it has the same shape as the three after it. The six others are artefacts of `D3` loading, not of the policy, so the charge per refresh is taken from the four full ones. The run is therefore proof of one refresh per press and none between, and of the charge per full refresh, but not of six clean refreshes.

## Daily energy of the policy (3.0 V rail)

Idle 74.8 µA gives 1.79 mAh a day. Net refresh charge 16.59 mC (mean of the four).

| Presses + alarm starts per day | Refresh charge | Total per day |
| --- | --- | --- |
| 10 | 0.046 mAh | 1.84 mAh |
| 20 (`[INFERENCE]`, assumed use) | 0.092 mAh | **1.89 mAh** |
| 50 | 0.230 mAh | 2.03 mAh |

C9 with a refresh every minute was 8.5 mAh a day, so 20 presses a day is 4.5 times lower. Idle is 95 % of the budget at 20 presses; the 72 µA of the peripherals on the rail is now the target, not the panel. DCF windows and buzzer are not in this table.

The number of presses per day is an assumption, not a measurement.

## Alarm refresh and buzzer pattern (task 1.2)

UART bench on `8c415fe`: one `display ok` per alarm start, none at stop by button, motion or timeout (UART log, 12:01 and 12:02 alarms). The buzzer pattern was not recorded on `D2`. The author set an alarm over BLE, heard it sound 1 s on / 1 s off, and vouches for this. This is an author observation, not a measurement.
