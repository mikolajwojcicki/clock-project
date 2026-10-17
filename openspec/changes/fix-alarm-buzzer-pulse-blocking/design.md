## Context

`poll_alarm()` in `apps/clock/src/main.c` runs from `app_poll()`. `app_poll()` is called from `board_wait_ms()` and from the BUSY wait in `epd.c`. It is not called inside the SPI transfer in `spi_bus.c`, which spins on `NRF_SPIM0->EVENTS_END`. The e-paper image data goes out over SPI at 250 kHz, so each transfer blocks for a long time. Measured: first tone 3.9 s and 5 s in capture 2, 7.8 s in capture 3. These numbers are close to a full refresh. Nothing here is confirmed yet.

When `poll_alarm()` does not run, the last buzzer state (on) persists. That matches the solid tone.

## Goals / Non-Goals

**Goals:** correct 1 s pattern from t = 0, 100 ms button response during refresh, a recorded SDK audit.

**Non-Goals:** the rewrite itself (`rewrite-firmware-on-nrf5-sdk-drivers`), deep sleep, e-paper partial refresh, glyph fixes (`fix-epaper-clock-face-glyphs`).

## Decisions

1. **Confirm before fixing.** Add one temporary UART line with `board_ms()` at refresh start and end, compare with the Saleae edges, remove it. Reason: the hypothesis is inferred from timing, not from code.
2. **Fix candidates, pick one after step 1.** (a) Call `app_poll()` or `alarm_step()` between SPI chunks in `epd.c`. (b) Drive the 1 s gating from a hardware timer or RTC compare event that toggles the PWM, so it no longer depends on the main loop. (c) Delay the draw while the alarm is in its first seconds. Prefer (b) if the SDK audit favors a timer-driven design anyway; otherwise (a), because it is the smallest diff. (c) is rejected: the user sees a stale minute while the alarm sounds.
3. **SDK audit as a recorded finding.** For every file under `firmware/src/` and `firmware/apps/clock/src/`, classify each hardware access as: nrfx driver call, `app_*` library call, direct peripheral register, or software GPIO toggling. Write one table into `notes/log/`. The verdict `rewrite` applies when any peripheral that has an nrfx driver (SPIM, PWM, RTC, UARTE, GPIOTE, TIMER) is driven by registers or by software toggling in product code. Direct register use of a peripheral with no driver value (for example `NRF_POWER` flags) is not a reason to rewrite.
4. **Preliminary reading, to be confirmed by the audit.** `board.c` includes `app_uart.h`. `spi_bus.c` uses `NRF_SPIM0` registers, `buzzer.c` uses `NRF_PWM0` registers, `board.c` uses `NRF_RTC1` registers. These are hardware peripherals, so this is not bit-banging in the strict sense, but it bypasses the nrfx drivers the project says it prefers.

## Risks / Trade-offs

- A capture needs a quiet minute and the Saleae; allow two runs.
- Fix (a) adds `alarm_step()` calls inside a blocking path; keep them cheap.
- If the cause is not the refresh, step 1 will show it and this change stops with a new hypothesis, not a guess fix.
