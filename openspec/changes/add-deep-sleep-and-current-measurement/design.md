# Design

## Context

`firmware/apps/clock/src/main.c` runs `for(;;) { app_poll(); ... }` with no WFE. `poll_uart`, `poll_clock`, `poll_alarm` and `poll_dcf` all spin on `board_ms()`. Buttons are read by `board_any_button_pressed()`. `app_uart` keeps UARTE and HFCLK on. `epd.c` waits on `EPD_BUSY` in a loop. DCF pulses are timed from a 1 ms-class poll. The motion path already wakes on INT1 through `nrfx_gpiote`. LFXO and RTC1 are the timebase (`board_time_init`). See proposal.md for motivation and specs/low-power-sleep-policy/spec.md for the contract.

Constraints: bare-metal nRF5 SDK 17.1.0, no RTOS, SoftDevice-free now. `add-ble-alarm-schedule-service` later adds a SoftDevice that owns RTC0 and LFCLK and expects RTC1 as the app timebase. This change must leave that path free. Hardware is the nRF52 DK (PCA10040) with a PPK2.

## Goals / Non-Goals

**Goals:**
- Idle current limited by the nRF52832 System ON sleep with RTC, not by firmware spinning.
- Every wake source is an interrupt; every deadline is an RTC1 compare.
- Four measured states with CSV and conditions.

**Non-Goals:**
- System OFF mode. The 1 Hz clock and alarms need RTC1, so System ON only.
- DC-DC tuning and DK-vs-module overhead removal. DC-DC is enabled if it is one line; the perfboard (stage 6) is where module overhead is measured.
- Changing alarm, motion-dismiss or DCF decoding rules.
- BLE or SoftDevice. Only the timebase is kept compatible.

## Decisions

**Main loop.** Keep one flag set `volatile uint32_t s_events`. ISRs set bits; the loop runs `__WFE(); __SEV(); __WFE();` guarded by a check that no bit is pending, then handles each bit and sleeps again. Rejected: RTOS or scheduler (locked out by AGENTS.md); a `sd_app_evt_wait` style wrapper (no SoftDevice yet). The existing `poll_*` functions become `handle_*` functions that run only on their event bit, so alarm, DCF and motion logic keep their code.

**Timebase.** Use `nrfx_rtc` on RTC1 directly. Compare 0 gives the 1 Hz clock tick and re-arms itself. Compare channels 1 and 2 serve one-shot deadlines (alarm timeout, DCF retry, DCF window end, e-paper BUSY timeout). Capture of the counter at a GPIOTE edge uses the RTC1 counter read in the ISR (no PPI capture in this change). `board_ms()` stays derived from the RTC1 counter (24-bit, with overflow handling). Rejected: `app_timer`, because it also claims RTC1 and hides compare allocation; TIMER peripherals, because HFCLK would run. This matches the BLE design, where the SoftDevice owns RTC0.

**Buttons.** GPIOTE PORT event (SENSE, low-power) on DK buttons P0.13-16, then a debounce deadline on RTC1 and a read of the pins. Stage 10 only changes pin numbers. Rejected: per-pin IN events (they hold the GPIOTE IN channel active, which costs current). The hold timing in the BLE design (`board_button_held_ms`) can be built on the same debounce deadline.

**UART.** Build flag `SLEEP_UART` (default 1 for bring-up use). With `SLEEP_UART=0` the UARTE is never initialized and `con_printf` compiles to nothing, so HFCLK is requested only by SPIM and the radio-free paths. Idle, DCF and alarm currents are measured with `SLEEP_UART=0`. The default build keeps the CLI for bench use. Rejected: UART RX wake with the UARTE on (hundreds of µA; defeats the idle number).

**E-paper refresh.** `epd.c` gets an async refresh: start refresh, return; the BUSY falling edge arrives as a GPIOTE IN event; an RTC1 compare is the timeout. The MCU sleeps in WFE during the wait. The panel goes to deep sleep after the refresh. SPIM is uninitialized or idle between transfers so HFCLK is not requested. The existing rule that a refresh does not run inside a DCF window stays.

**DCF window.** `DCF_PON` is on only in a window. GPIOTE IN events on `DCF_OUT` (both edges) give edge times from the RTC1 counter (30.5 µs). Widths go to the existing `dcf77_classify_width`, so decode and rejection reasons are unchanged. A window-end deadline and a "missed second" deadline use RTC1 compares. Between windows the DCF GPIOTE channel is disabled so the pin causes no wake. Rejected: keeping ms sampling (wakes every ms in a window, measurable and avoidable).

**Alarm and motion.** The alarm start is an RTC1-tick check on minute change (as now). While the alarm sounds, the buzzer toggles by PWM-like timing already in `buzzer.c`; if it needs a timer, the buzzer state counts as awake current and is measured as the alarm state. INT1 wake stays on GPIOTE. The accelerometer is powered down outside an alarm (existing `motion_disarm`).

**Measurement method.** PPK2 in Source Meter mode supplies the nRF DK through P22 (VDD_nRF), 3.0 V, with the J-Link interface not powered by the same rail. Flash first, then switch the supply to the PPK2. UART logging is off, so state timing comes from the Saleae digital markers already present (`CLOCK_SEC_MARK`, `DCF_PON`, `BUZZER_EN`, plus `EPD_BUSY`). PPK2 CSV and Saleae export are aligned by a marker edge visible in both (the DCF_PON rising edge at window start, since PPK2 shows the step). Each state is measured for at least 60 s for idle and for its natural duration for the others. Record: PPK2 serial and mode, supply, ambient temperature from a room thermometer, `git rev-parse --short HEAD`, and which DK parts stay on the rail. Compare idle with the nRF52832 datasheet figure for System ON, RTC running, and explain the gap.

**Flowchart.** `software-flowchart.puml` shows a poll loop. After the code lands, redraw it as event-driven: event bits, ISR sources, WFE. Regenerate svg and pdf with the existing method in `firmware/docs/`.

## Risks / Trade-offs

- DK idle is dominated by non-chip parts on the rail → isolate through P22, report the caveat in the note, repeat on the perfboard (`build-perfboard-prototype`).
- Missed event when WFE races an ISR → use `__SEV(); __WFE(); __WFE();` and check the pending bits with interrupts disabled before sleeping.
- RTC1 24-bit counter wraps every 512 s → handle wrap in `board_ms()` and in edge-time subtraction; unit-check the subtraction with a throwaway script.
- Debounce and BUSY handling change timing of existing behavior → keep the UART-on build for regression checks on motion-dismiss and DCF sync (existing bench steps).
- GPIOTE IN channels keep HFCLK-independent logic but raise current when active → enable only for the DCF window, EPD refresh and alarm.
- SPIM or the e-paper driver may hold HFCLK on after a transfer → check on the PPK2 trace; the idle number must not show a step from HFCLK.
- PPK2 resolution and noise at tens of µA → average over 60 s, note the PPK2 spec in the record.

## Migration Plan

1. Add RTC1 compare tick and event loop, keep UART on, check clock, alarm, motion and DCF behavior unchanged.
2. Move buttons, e-paper BUSY and DCF edges to interrupts one at a time, each followed by the same regression check.
3. Add the `SLEEP_UART` switch and measure.
4. Update the flowchart.

Rollback is a git revert of the firmware commits; there is no stored state.
