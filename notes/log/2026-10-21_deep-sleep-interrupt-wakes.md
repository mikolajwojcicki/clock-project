# 2026-10-21 Interrupt wake sources: buttons, e-paper BUSY, DCF

Change: `add-deep-sleep-and-current-measurement`, tasks 2.1 to 2.4. Group 1 results are in `2026-10-21_deep-sleep-wfe-regression.md`.

## Setup

- nRF52 DK (PCA10040) on the breadboard, Waveshare 2.13" panel, same wiring as the group 1 note.
- Instrument: Saleae Logic, digital, 1 MS/s. Ch 0 `CLOCK_SEC_MARK`, Ch 1 `BUZZER_EN`, Ch 2 `SENSOR_INT1`, Ch 3 `EPD_BUSY` (`P0.29`).
- Supply: USB from the DK. Temperature: room, not logged.

## 2.1 Buttons

- GPIOTE PORT (SENSE) wake on P0.13 to P0.16, pins read on wake. No debounce: the only press action is to silence the alarm, so one edge is enough. Dropped from the plan with the user (design.md Buttons updated).
- Evidence: the hold run stopped the alarm 6.3 s after start, between 1 s phase changes, so only the button wake can have run the handler.

## 2.2 E-paper BUSY

- `wait_idle()` in `firmware/src/epd.c` now enables a GPIOTE IN event on the `EPD_BUSY` falling edge, sleeps in WFE, and wakes on the edge, on a handler deadline or on a 10 s RTC1 timeout. It disables and uninitialises the channel afterwards. This replaces the planned start-and-return async API: the MCU still sleeps for the whole BUSY time, with much less code. `app_poll()` now arms the app wake itself, so the buzzer keeps its phase while the CPU sleeps in a driver wait.
- Results (capture `notes/measurements/2026-10-21_deep-sleep_epd-busy-refresh.sal`, firmware `a0ad997`):
  - `EPD_BUSY` high for 2.29 s during the master activation (9.9035 s to 12.1899 s into the capture). It is also high while the panel sits in deep sleep after a refresh.
  - The panel showed 12:20 and then 12:21 in the earlier run without the BUSY probe. I did not ask about the panel in this capture run.
  - An alarm started at the same moment as a refresh. 30 buzzer bursts came 1.994 to 2.006 s apart.
- Not yet proven: that the CPU really sleeps during the BUSY window. That needs the PPK2 trace in task 4.3.

## What went wrong on the way

- First BUSY capture showed Ch 3 toggling at hundreds of kHz for two minutes (87 million edges). The probe wire lay next to the SPI clock. The panel update stalled until the experiment ended; the BUSY edge interrupt probably fired nonstop. Without the probe the panel updated within 15 s. After moving the probe and hanging the analyzer so the wires cross the noisy lines at right angles, Ch 3 was clean outside one burst during the third refresh's SPI data, when the edge event is off.
- The 1.7 GB CSV export from that capture blocked the Saleae export tool for minutes. Closing the capture ended it. Large exports of a noisy channel need a shorter capture.
- Between 6.2338 s and 9.9035 s `EPD_BUSY` stays low for 3.7 s, so the refresh is CPU-bound there, not BUSY-bound: SPI writes at 250 kHz plus `clock_face_black()` per pixel, twice per face. This awake time is likely larger than the BUSY time. Check it in task 4.3.

## 2.3 DCF

- Code: DCF edges were already GPIOTE IN events timestamped from the RTC1-derived `board_ms()` (1 ms), and the window end is now an RTC1 deadline in `arm_wake` (`dcf77_sync_deadline_ms`). No missed-second deadline: the decoder only changes state when a pulse arrives, and such a deadline would turn a silent window from `timeout` into `pulse` (decided with the user, design.md updated).
- Bench: DK with DCF-1060N module, Nucleo generator on `/dev/ttyACM2`, no wire (antenna to antenna), carrier 0 dB, same as the 2026-10-18 note. Firmware: the WFE loop with e-paper and button wakes.

| Case | Result |
|---|---|
| Valid frames | `dcf: ok unset -> 2026-10-17 21:25:00` 216 s after `dcf sync`, matching the generator frames 21:24 and 21:25 |
| Key `d` (wrong date) | `dcf: rejected mismatch (time unchanged unset)` 175 s after `dcf sync` |
| Key `p` (parity) | `dcf: rejected parity (time unchanged unset)` |
| Key `s` (silence) | window ended with `dcf: timeout (time unchanged unset)` |

Same reasons as 2026-10-18.

- Caveats: the first run of this session started with a boot DCF window still open on the DK, so its "valid frame" case only saw that old window time out after 8 s; I reran it from a closed window (the `ok` line above). The silence case reused the window opened for the `d` run: it ended in `timeout` 300 s after opening, with the generator silent for the last 63 s only. That is a weaker test than a full silent window.
- The DK clock showed `unset` before the sync, so a reset must have happened between the earlier alarm runs and this session. I did not find the cause.

## 2.4 Regression after 2.1 to 2.3

Firmware: WFE loop with button wake, e-paper BUSY wake and DCF window deadline. Same wiring and test alarm as the group 1 note (Wednesday 12:01, clock set to 12:00:30). Saleae ch 0 `CLOCK_SEC_MARK`, ch 1 `BUZZER_EN`, ch 2 `SENSOR_INT1`.

| Case | Result |
|---|---|
| Taps | 20 events, `MOTION event 20 (stop)` and `ALARM stop (motion)` 8.3 s after `ALARM 12:01` |
| Button, no motion | `MOTION event 1` at start, `ALARM stop (button)` 1.4 s later, no other `MOTION` line |
| Shake, no alarm | no `MOTION` or `ALARM` line in 45 s; 0 edges on Ch 1 and Ch 2 in 50 s |
| Board still | no `MOTION` line; Ch 2 had no edge in 105 s; `ALARM stop (timeout)` 60.00 s after start; buzzer edges from 35.228 s to 94.233 s |

DCF generator results are in the 2.3 section above. Same outcomes as the 2026-10-20 note and the group 1 note. No boot banner appeared during any of these runs, so the DK did not reset in them. The unexplained INT1 failure from the group 1 note did not come back in this session's four runs.

Captures in `notes/measurements/`: `2026-10-21_deep-sleep_group2-motion-tap.sal`, `..._group2-motion-button.sal`, `..._group2-motion-shake-no-alarm.sal`, `..._group2-motion-still-timeout.sal`.

## Open observation: panel stale after a boot-time DCF sync

The user saw the panel stay on the old time after a reset, even though the boot DCF sync succeeded and minutes passed; setting the time by hand first and then syncing updated the panel. I could not reproduce it on the 2.2 build (`cb19354-dirty`, the WFE loop with e-paper BUSY wake):
- OpenOCD soft reset, generator at 0 dB: `dcf: ok unset -> 2026-10-17 21:18:00` at 157.6 s, the UART clock ran on, and the panel showed 21:18 and followed the minutes (user check).
- DK reset button, same generator: panel updated normally (user check).
- An earlier attempt with the generator unplugged only gave `rejected pulse` and `timeout`, as expected without a signal.

Not explained. Candidates not tested: a different firmware on the board at that time (I flashed the pre-change firmware `a7c7cbf` and a diagnostic build earlier in the session), or a sync that finishes while a refresh is still running. Reopen if it shows again; the UART log would show `display FAIL` if a refresh failed.

## 3.1 SLEEP_UART flag and DC-DC

- `make SLEEP_UART=0` builds into `armgcc/_build_nouart` with the console compiled to nothing (no UARTE init, `con_printf` empty); the default is 1. Text size 26948 B against 22540 B without the UART. `NRF_POWER->DCDCEN` is enabled in `board_safe_state()` for all apps (the DK carries the inductors).
- Bench: no-UART build flashed, generator at 0 dB. The boot DCF sync set the clock, `CLOCK_SEC_MARK` started toggling at 176.7 s and kept 1 s spacing (65 of 68 gaps within 0.99 to 1.01 s; the rest are refresh catch-ups), and the user saw the panel show the synced time and follow the minutes. Capture: `notes/measurements/2026-10-21_deep-sleep_nouart-boot-sync.sal`.
- DC-DC: the DK kept running with it enabled, so the inductors are fitted. Its current effect is not measured separately (that needs a PPK2 run with and without the line).

## 3.2 Power-gate states (no-UART build)

Setup: `SLEEP_UART=0` build, DK reset by OpenOCD about 3.3 s into a 330 s capture, generator at 0 dB. Saleae ch 0 `DCF_PON` (`P0.24`), ch 1 `BUZZER_EN`, ch 2 `SENSOR_INT1`, ch 3 `EPD_BUSY` (`P0.29`). Capture: `notes/measurements/2026-10-21_deep-sleep_power-gate-states.sal`.

| State | Evidence |
|---|---|
| Receiver off outside a window | `DCF_PON` high at the start, low from 11.33 s (boot window open), high again from 140.74 s (sync done) and no edge for the last 190 s |
| Buzzer driver off | `BUZZER_EN` low for all 330 s, no edge (no alarm was set: the build has no console) |
| Accelerometer off, no wake | `SENSOR_INT1` low for all 330 s, no edge. This shows only that INT1 is quiet; the sensor's own current is a PPK2 question for 4.2 |
| Panel in deep sleep after a refresh | `EPD_BUSY` high 3 times in each refresh: 2.29 s while the update runs, then high in deep sleep. Between refreshes it has no edge: 148.358 s to 200.662 s (52 s), and again 209.220 s to 260.668 s |

Refreshes follow the sync (140.76 s) and each minute change (200.66, 260.67, 320.67 s), as before.

The task text had the `DCF_PON` polarity reversed: `board_safe_state()` drives `DCF_PON` high to power the receiver down, and `dcf77_sync_open()` pulls it low only while a window is open. Task 3.2 was corrected.

Not covered: accelerometer and buzzer in an alarm (needs the UART build, covered by the group 1 and 2 notes), and any current figure.

## Open: stale panel content after a refresh (no-UART + PPK2)

Symptom (user): the border flashes (panel runs the update) but the glass keeps the old minute. Irregular: sometimes every other minute, sometimes two minutes in a row. Seen on the no-UART build with the PPK2 at 3.3 V. UART + PPK2 and UART + USB runs in the same session followed the minutes.

Suspects, in order: (1) SPI image bytes not landing / RAM swap leaving the old buffer, (2) supply sag on the PPK2 rail during the SPI burst, (3) `draw()` never retries after a failed refresh (`s_drawn_min` is set even when `epd_refresh` returns false).

### Working capture (keep this)

- File: `notes/measurements/2026-10-21-21-41-screen-measurement.csv` (11 MB, ~524k rows, ~314 s). User recorded and exported it by hand after the MCP capture was lost.
- Firmware: no-UART (`SLEEP_UART=0`), PPK2 3.3 V, DCF generator on.
- Saleae digital channels (confirmed):
  - Ch 0 `SPI_SCK` `P0.03`
  - Ch 1 `SPI_MOSI` `P0.04`
  - Ch 2 `EPD_CS` `P0.11` (active low)
  - Ch 3 `EPD_DC` `P0.12`
- CS-fall bursts (gap > 0.5 s):

| Burst | Time (s) | Span | CS falls | Note |
|---|---|---|---|---|
| 0 | 4.491–4.711 | 0.22 s | 41 | short, boot |
| 1 | 6.996–14.890 | 7.89 s | 3811 | full refresh (boot draw) |
| 2 | 183.319–183.321 | ~1 ms | 37 | short, not a full RAM write |
| 3 | 243.232–243.233 | ~1 ms | 35 | short, ~60 s later |
| 4 | 303.238–303.239 | ~1 ms | 37 | short, ~60 s later |
| 5 | 305.524–313.104 | 7.58 s | 3450 | full refresh |

### SPI decode (Mode 0, MSB first; DC 0=cmd 1=data)

Full refresh (boot burst0+1, and minute burst4+5) matches `epd_refresh()`:
1. Init SPI through `0x12`…`0x18` (~2 ms of CS activity)
2. Quiet gap **2.285 s** (`wait_idle()` after init; BUSY high)
3. `0x24` / `0x26` RAM writes, `0x22`/`0x20` update, second RAM write, `0x10` sleep (~8 s, ~16k data bytes)

Failed minute attempts (burst2 @ 183.3 s, burst3 @ 243.2 s):
- Same init through `0x18`, then **no further SPI** until the next minute (~60 s)
- No `0x24`/`0x26`, no `0x22`/`0x20`, no `0x10`
- This is `wait_idle()` returning false after the init block in `epd_refresh()`, before `write_both_rams()`
- Visible “border flash, old content” fits: HW/soft reset + init run; image RAM and master activation never run

Burst4 @ 303.2 s is not a failure: it is the init half of the successful refresh that continues at 305.5 s after the same 2.285 s BUSY wait.

Not yet known: whether `wait_idle()` fails from a BUSY timeout (10 s) or from `nrfx_gpiote_in_init(EPD_BUSY)` failing. Needs a BUSY trace or a UART build that logs which path failed.

`draw()` now leaves `s_drawn_min` unchanged on failure and retries after a 2 s backoff. `wait_idle` prints `display FAIL <stage>: busy-timeout|gpiote`.

### UART 20-time forced refresh (user scored glass)

Script `/tmp/epd20.py`: set 20 distinct times, 18 s apart. User scored first 10 on the glass. Firmware was the UART diagnostic build. Serial showed **no `display FAIL`** on #01–#05 (and the draw blocked ~8 s = full refresh path), yet several of those were STALE on the glass:

| # | Time | Glass |
|---|---|---|
| 01 | 00:00 | STALE |
| 02 | 01:11 | STALE |
| 03 | 02:22 | SHOW |
| 04 | 03:33 | STALE |
| 05 | 04:44 | STALE |
| 06 | 05:55 | SHOW |
| 07 | 06:06 | SHOW |
| 08 | 07:17 | STALE |
| 09 | 08:28 | SHOW |
| 10 | 09:39 | STALE |

So there are **two** failure modes:
1. Saleae short burst: abort after init (`wait_idle` false) — no RAM write.
2. This UART run: `epd_refresh` returns true (~8 s full path) but the glass keeps the previous image (border may flash).

Mode 2 is not explained by the draw-retry bug. Next: SPI+BUSY capture while scoring another 20, or prove whether RAM bytes change on a STALE “success”.

Power: nRF Connect PPK app was running during follow-up (`pc-nrfconnect-ppk`), so the Mode 2 UART run was very likely on the PPK2 rail. A/B on USB is the next check for supply sag. `wait_idle` now logs `display wait <stage>: <ms>|idle-0ms` and falls back to polling if GPIOTE IN init fails (no longer hard-fails with `gpiote`).

Root cause candidate for Mode 2: `wait_idle` returned success when BUSY never went high (`after-init: 0ms`, boot `activate: idle-0ms`). RAM write / deep-sleep then ran without a real update. Fix: wait up to 100 ms for BUSY to rise; for soft-reset / after-init / activate a missing pulse is `display FAIL <stage>: no-busy` (triggers draw retry).
