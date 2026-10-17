## 1. Gate

- [x] 1.1 Read the verdict in `notes/log/` from `fix-alarm-buzzer-pulse-blocking` task 4.2; if `keep`, mark this change dropped and stop

## 2. Rewrite, one peripheral per commit

- [x] 2.1 `spi_bus.c` on `nrfx_spim`; enable the module in `sdk_config.h`; build; run the bring-up e-paper and LIS3DH checks
- [x] 2.2 `buzzer.c` on `nrfx_pwm`; re-measure the 2.7 kHz tone on Saleae
- [x] 2.3 `board.c` time base on `nrfx_rtc` + `nrfx_clock` (or `app_timer`); `board_ms()` unchanged for callers; record the choice and why
- [x] 2.4 Replace remaining spin loops with driver events or flags, keeping a timeout on every blocking init path
- [x] 2.5 Search `firmware/src/` and `firmware/apps/` for leftover `NRF_SPIM`, `NRF_PWM`, `NRF_RTC`, `NRF_UARTE`, `NRF_TIMER` use; none remains

## 3. Verify on the DK

- [ ] 3.1 Clock app: set time, set alarm, see face refresh, alarm tone and pattern, button silence
- [ ] 3.2 Bring-up app: same results as the recorded notes
- [ ] 3.3 Compare `BUZZER_EN` capture to the pre-rewrite capture (tone frequency, 1 s pattern)

## 4. Record

- [ ] 4.1 Update `firmware/README.md` build notes if `sdk_config.h` or the Makefile changed
- [ ] 4.2 Add a dated note in `notes/log/`: what was replaced, what was kept, results, firmware commits
