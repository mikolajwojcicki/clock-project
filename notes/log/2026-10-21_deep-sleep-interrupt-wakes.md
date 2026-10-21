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
