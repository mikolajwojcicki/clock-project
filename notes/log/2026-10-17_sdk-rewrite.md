# 2026-10-17 — firmware moved to nrfx drivers

OpenSpec change `rewrite-firmware-on-nrf5-sdk-drivers`, tasks 1.1-2.5.

| Peripheral | Before | After | Commit |
|---|---|---|---|
| SPIM0 | registers | `nrfx_spim` blocking | 177b101 |
| PWM0 | registers | `nrfx_pwm` | 5d9b24f |
| RTC1 + LFXO | registers | `nrfx_rtc` + `nrfx_clock` | see git log |
| UART0 | `app_uart` + register init (two paths) | `app_uart` only (UARTE) | see git log |

Choice for the time base: `nrfx_rtc` + `nrfx_clock`, not `app_timer`. `board_ms()` keeps its signature and needs only a free-running counter plus overflow count; `app_timer` would add a scheduler and a second RTC owner.

Blocking waits left: LFXO start (bounded), UART put (bounded), `board_wait_ms` (RTC based). No `NRF_SPIM/PWM/RTC/UART0/CLOCK` register access remains in `firmware/src/` or `firmware/apps/`.

Results: `clock`, `bringup`, `bringup-4button` build. Clock app flashes, accepts a time over UART and prints each second. Bring-up banner prints. Not yet done: Saleae tone capture (2.2, 3.3), clock alarm run (3.1), button tests (3.2).
