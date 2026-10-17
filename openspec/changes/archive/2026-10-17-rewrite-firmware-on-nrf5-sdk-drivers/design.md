## Context

Observed in the tree (grep, 2026-10-17): `spi_bus.c` sets `NRF_SPIM0->PSEL/FREQUENCY/CONFIG/TXD/RXD` and polls `EVENTS_END`; `buzzer.c` configures `NRF_PWM0` sequences and spins `STOP_SPIN` times for `EVENTS_STOPPED`; `board.c` configures `NRF_RTC1` and its overflow ISR, and uses `app_uart`. The other modules (`epd.c`, `lis3dh.c`) sit on `spi_bus` and `nrf_gpio` calls.

None of it is software bit-banging of a bus. It is register-level peripheral use that bypasses nrfx. The audit decides whether this counts as the problem the author means.

## Goals / Non-Goals

**Goals:** nrfx driver use for every peripheral that has one; no behavior change; DK regression pass.

**Non-Goals:** deep sleep, SoftDevice / BLE, new features, a new module layout, moving to nRF Connect SDK.

## Decisions

1. **Gate on the audit.** First task checks the verdict in `notes/log/`.
2. **Keep `spi_bus.h`, `buzzer.h`, `board.h` interfaces.** Only the `.c` files change. Callers (`epd.c`, `lis3dh.c`, apps) stay, so each driver swap is one commit and one DK check.
3. **One peripheral per commit,** in this order: SPIM (`nrfx_spim`), PWM (`nrfx_pwm`), RTC / time base (`nrfx_rtc` + `nrfx_clock`, or `app_timer`), GPIO and buttons (`nrfx_gpiote` only if an interrupt is needed now; plain `nrf_gpio` is the SDK HAL and stays). Reason: dense history for the thesis, easy bisect.
4. **Time base.** `board_ms()` stays a function; its source changes to the driver. If `app_timer` is chosen, record why in the note (it brings RTC1 ownership, which the SoftDevice later also wants).
5. **Blocking waits.** Prefer a driver event handler and a flag over a spin loop. Where blocking stays (init), keep a timeout.
6. **Verification per step.** Build, flash, run the clock app set-time and alarm scenario, compare the Saleae pattern of `BUZZER_EN` to the pre-rewrite capture.

## Risks / Trade-offs

- nrfx PWM sequence semantics differ from the raw setup; the 2.7 kHz tone must be re-measured.
- `sdk_config.h` needs the nrfx modules enabled; the SDK `sdk_config.h` is large. Enable only what is used.
- RTC1 ownership: `app_timer` and the SoftDevice both use it. Decide now to avoid a second rewrite at BLE time.
- Doing this before stage 4 and 5 delays them; doing it after means rewriting more code.
