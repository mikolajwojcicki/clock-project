# Tasks

## 1. RTC1 timebase and event loop

- [ ] 1.1 In `firmware/src/board.c` move `board_ms()` to the `nrfx_rtc` RTC1 counter with 24-bit wrap handling; add compare 0 as a 1 Hz tick and compares 1 and 2 as one-shot deadlines (set, cancel, expired callback). Verify: `make` builds, and a throwaway host-side check of the wrap subtraction (values across 0x00FFFFFF to 0) gives the right delta.
- [ ] 1.2 Add `s_events` bits in `firmware/apps/clock/src/main.c` and replace `for(;;) app_poll()` with a WFE loop that sleeps when no bit is set (`__SEV(); __WFE(); __WFE();` with the pending check). Turn `poll_clock`, `poll_alarm`, `poll_uart`, `poll_dcf` into handlers run on their bit. Verify on the DK with UART on: after `YYYY-MM-DD HH:MM:SS` the UART second line advances at 1 s per line for 2 min, and `CLOCK_SEC_MARK` toggles at 1 Hz on the Saleae.
- [ ] 1.3 Move the alarm trigger, alarm timeout and buzzer timing to the minute tick and RTC1 deadlines. Verify on the DK: set an alarm for the next minute, the buzzer starts within 1 s, a button press silences it, and a timeout run ends the alarm at the existing timeout.
- [ ] 1.4 Regression with UART on: repeat the motion-dismiss checks from `notes/log/2026-10-20_motion-dismiss.md` (still, tap-to-silence, button with no motion). Record the UART text in a dated note under `notes/log/`. Verify: same results as the earlier note.
- [ ] 1.5 Commit the firmware (`firmware:` prefix) after 1.1-1.4 pass.

## 2. Interrupt wake sources

- [ ] 2.1 Buttons: replace `board_any_button_pressed()` polling with a GPIOTE PORT (SENSE) wake on P0.13-16 plus an RTC1 debounce deadline. Verify on the DK: press during sleep sets the button event and silences an alarm; with the Saleae, no button activity means no extra wake edges on `CLOCK_SEC_MARK` timing.
- [ ] 2.2 E-paper: make the refresh in `firmware/src/epd.c` asynchronous. Start the refresh, sleep, finish on the `EPD_BUSY` falling edge (GPIOTE IN) or an RTC1 timeout, then send the panel to deep sleep and disable the IN channel. Verify on the DK: the time on the panel updates at the minute change, and the Saleae shows `EPD_BUSY` high for the refresh and no CPU spin (UART off).
- [ ] 2.3 DCF: in `firmware/src/dcf77_sync.c` feed `dcf77_classify_width` from GPIOTE edges on `DCF_OUT` timestamped by the RTC1 counter; use RTC1 deadlines for the window end and a missed second; disable the DCF GPIOTE channel outside a window. Verify: with the generator from the 2026-10-18 notes, `dcf sync` prints `dcf: ok` with the generated time, and a corrupt-pulse run still prints the same rejection reason as before.
- [ ] 2.4 Regression with UART on: repeat 1.4 and the DCF generator run after 2.1-2.3. Verify: same outcomes as the earlier notes; log the results in a dated note under `notes/log/`.
- [ ] 2.5 Commit the firmware after 2.1-2.4 pass.

## 3. Power gating and UART switch

- [ ] 3.1 Add the `SLEEP_UART` build flag (default 1) in `firmware/apps/clock/armgcc/Makefile` and `board.c`: with 0, no `app_uart` init and `con_printf` is a no-op. Enable DC-DC (`NRF_POWER->DCDCEN`) in `board_safe_state` if the DK allows it. Verify: both `make` and `make SLEEP_UART=0` build and the 0 build runs the clock on the panel.
- [ ] 3.2 Check the power-gate states with the Saleae on a `SLEEP_UART=0` build: `DCF_PON` low outside a window, accelerometer and buzzer driver off outside an alarm, panel deep sleep after a refresh. Verify: a capture saved under `notes/measurements/` shows each state.
- [ ] 3.3 Commit the flag and DC-DC change as its own commit.

## 4. Current measurement

- [ ] 4.1 Prepare the DK for the PPK2 (Source Meter, 3.0 V on P22, interface MCU isolated). Flash first with OpenOCD, then switch the supply. Verify: the clock runs on the panel on PPK2 power. Photograph the wiring with the name `YYYY-MM-DD_deep-sleep_ppk2-wiring.jpg` under `notes/media/`.
- [ ] 4.2 Measure idle for at least 60 s on `SLEEP_UART=0`. Verify: a CSV exists with the PPK2 serial, mode, supply, room temperature and `git rev-parse --short HEAD`; the note compares it with the nRF52832 System ON, RTC-running figure and explains the gap (DK parts on the rail).
- [ ] 4.3 Measure e-paper refresh (align PPK2 with `EPD_BUSY` on the Saleae). Verify: a CSV, the refresh duration and average current are in the note.
- [ ] 4.4 Measure the DCF window using the generator (align with `DCF_PON`). Verify: a CSV plus average current and window length are in the note.
- [ ] 4.5 Measure the alarm with the buzzer on (align with `BUZZER_EN`). Verify: a CSV plus average current are in the note.
- [ ] 4.6 Write the dated note `notes/log/2026-10-<day>_deep-sleep-current.md`: setup, a table with the four states (current, duration), what failed, and a daily energy estimate from the durations. Verify: each CSV is linked and the daily figure follows from the table.
- [ ] 4.7 Add the entries to `notes/ai-use.md` for any draft text kept. Commit measurements and the note together (`firmware:` or `thesis:` per the files touched).

## 5. Flowchart and close-out

- [ ] 5.1 Redraw `firmware/docs/software-flowchart.puml` as event driven (event bits, ISR sources, WFE) and regenerate the svg and pdf the same way as before. Verify: the new svg renders and shows the sleep loop.
- [ ] 5.2 Run `openspec validate add-deep-sleep-and-current-measurement --strict`. Verify: passes.

## Workflow follow-up

- Archive the change after bench results are in. Stage 7 (`add-ble-alarm-schedule-service`) starts only after this.
