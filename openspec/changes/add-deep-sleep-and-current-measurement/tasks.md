# Tasks

## 1. RTC1 timebase and event loop

- [x] 1.1 In `firmware/src/board.c` move `board_ms()` to the `nrfx_rtc` RTC1 counter with 24-bit wrap handling; add compare 0 as a 1 Hz tick and compares 1 and 2 as one-shot deadlines (set, cancel, expired callback). Verify: `make` builds, and a throwaway host-side check of the wrap subtraction (values across 0x00FFFFFF to 0) gives the right delta.
- [x] 1.2 Add `s_events` bits in `firmware/apps/clock/src/main.c` and replace `for(;;) app_poll()` with a WFE loop that sleeps when no bit is set (`__SEV(); __WFE(); __WFE();` with the pending check). Turn `poll_clock`, `poll_alarm`, `poll_uart`, `poll_dcf` into handlers run on their bit. Verify on the DK with UART on: after `YYYY-MM-DD HH:MM:SS` the UART second line advances at 1 s per line for 2 min, and `CLOCK_SEC_MARK` toggles at 1 Hz on the Saleae.
- [x] 1.3 Move the alarm trigger, alarm timeout and buzzer timing to the minute tick and RTC1 deadlines. Verify on the DK: set an alarm for the next minute, the buzzer starts within 1 s, a button press silences it, and a timeout run ends the alarm at the existing timeout.
- [x] 1.4 Regression with UART on: repeat the motion-dismiss checks from `notes/log/2026-10-20_motion-dismiss.md` (still, tap-to-silence, button with no motion). Record the UART text in a dated note under `notes/log/`. Verify: same results as the earlier note.
- [x] 1.5 Commit the firmware (`firmware:` prefix) after 1.1-1.4 pass.

## 2. Interrupt wake sources

- [x] 2.1 Buttons: GPIOTE PORT (SENSE) wake on P0.13-16, read on wake, no debounce (decided 2026-10-21: a press only silences the alarm, one edge is enough; the stage 7 hold timer brings its own RTC1 deadline). Verified on the DK: the hold run stopped the alarm 6.3 s after start, between 1 s phase changes, so only the button wake can have run the handler; with no button activity `CLOCK_SEC_MARK` keeps 1 s spacing (`notes/measurements/2026-10-21_deep-sleep_clock-sec-mark.sal`).
- [x] 2.2 E-paper: `wait_idle()` in `firmware/src/epd.c` sleeps in WFE on a GPIOTE IN event for the `EPD_BUSY` falling edge or an RTC1 timeout, then disables the channel; `app_poll()` arms the app wake so deadlines hold during the wait. Chosen instead of a start-and-return API: same sleep during BUSY, far less code. Verified on the DK: panel updates at the set and at the minute change; Saleae shows `EPD_BUSY` high 2.29 s per refresh and buzzer phases intact when an alarm starts during a refresh (`notes/log/2026-10-21_deep-sleep-interrupt-wakes.md`). The proof that the CPU sleeps during BUSY moves to task 4.3 (PPK2).
- [x] 2.3 DCF: edges are GPIOTE IN events on `DCF_OUT` timestamped from the RTC1 counter and fed to `dcf77_classify_width` (already in `firmware/src/dcf77_sync.c`); the window end is an RTC1 deadline armed through `arm_wake` (`dcf77_sync_deadline_ms`); the GPIOTE channel is released when the window closes. No missed-second deadline (decided 2026-10-21, see design.md). Verify: with the generator from the 2026-10-18 notes, `dcf sync` prints `dcf: ok` with the generated time, and wrong-date and parity faults still print `mismatch` and `parity`; with the generator silent the window ends in `timeout`.
- [x] 2.4 Regression with UART on: repeat 1.4 and the DCF generator run after 2.1-2.3. Verify: same outcomes as the earlier notes; log the results in a dated note under `notes/log/`.
- [x] 2.5 Commit the firmware after 2.1-2.4 pass.

## 3. Power gating and UART switch

- [x] 3.1 Add the `SLEEP_UART` build flag (default 1) in `firmware/apps/clock/armgcc/Makefile` and `board.c`: with 0, no `app_uart` init and `con_printf` is a no-op. Enable DC-DC (`NRF_POWER->DCDCEN`) in `board_safe_state` if the DK allows it. Verify: both `make` and `make SLEEP_UART=0` build and the 0 build runs the clock on the panel.
- [x] 3.2 Check the power-gate states with the Saleae on a `SLEEP_UART=0` build: `DCF_PON` high (receiver powered down) outside a window, accelerometer and buzzer driver off outside an alarm, panel deep sleep after a refresh. Verify: a capture saved under `notes/measurements/` shows each state.
- [x] 3.3 Commit the flag and DC-DC change as its own commit.

## 4. Current measurement

- [x] 4.0 Leak check, no code. With the PPK2 disconnected and USB plugged in, measure the voltage on the nRF side of P22 and on DK P1 `VDD` with a multimeter. Verify: both values are in the note. A nonzero P22 value confirms back-feed. The P1 `VDD` value becomes the Source Meter voltage for A and C. Done 2026-10-24: P22 nRF side ~1.5 V, P1 `VDD` ~3.0 V; back-feed confirmed, Source Meter 3.0 V (`notes/log/2026-10-24_deep-sleep-current.md`).
- [ ] 4.1 Prepare the DK for the PPK2 (configurations A, B, C in design.md). Flash first with OpenOCD (`SLEEP_UART=0`), then switch the supply. Verify: in C the clock runs on the panel on PPK2 power. Photograph each wiring as `YYYY-MM-DD_deep-sleep_ppk2-<A|B|C>.jpg` under `notes/media/`.
- [ ] 4.2 A, nRF alone: idle for at least 60 s between minute changes, all breadboard wires unplugged from the DK. Verify: a CSV or packed capture exists with the configuration, PPK2 serial, mode, supply, room temperature and `git rev-parse --short HEAD`; the note compares it with the nRF52832 System ON, RTC-running figure and explains the gap.
- [ ] 4.3 B and C: idle for at least 60 s, then e-paper refresh (align with `EPD_BUSY` on the Saleae). Verify: one CSV or packed capture per configuration and state; the note gives refresh duration and average current, and compares C with A + B.
  - Note (2026-10-27, `fix-idle-pin-leaks`): C idle figures before C6 include the `EPD_RST` (2.07 mA) and `SCK`/`MOSI` (about 1.1 mA) pin leaks. Runs from C6 on use the fixed firmware (`07dd2de`, `a762be5`). From C6 a further 0.73 mA is drawn by the LIS3DH breakout itself and is still open.
- [ ] 4.4 B and C: DCF window using the generator (align with `DCF_PON`). Verify: CSVs or packed captures plus average current and window length are in the note.
- [ ] 4.5 B and C: alarm (align with `BUZZER_EN`), plus one Ampere Meter run in series with the buzzer `+` wire on `5V`. Verify: CSVs or packed captures plus average currents are in the note.
- [ ] 4.6 Write the dated note `notes/log/2026-10-<day>_deep-sleep-current.md`: setup, a table of the four states for A (idle only), B and C (current, duration), the C vs A + B comparison, what failed, and a daily energy estimate from C plus the buzzer. Verify: each CSV is linked and the daily figure follows from the table.
- [ ] 4.7 Add the entries to `notes/ai-use.md` for any draft text kept. Commit measurements and the note together (`firmware:` or `thesis:` per the files touched).

## 5. Flowchart and close-out

- [ ] 5.1 Redraw `firmware/docs/software-flowchart.puml` as event driven (event bits, ISR sources, WFE) and regenerate the svg and pdf the same way as before. Verify: the new svg renders and shows the sleep loop.
- [ ] 5.2 Run `openspec validate add-deep-sleep-and-current-measurement --strict`. Verify: passes.

## Workflow follow-up

- Archive the change after bench results are in. Stage 7 (`add-ble-alarm-schedule-service`) starts only after this.
