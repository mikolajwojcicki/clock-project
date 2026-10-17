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

Results: `clock`, `bringup`, `bringup-4button` build and flash. Clock app accepts a time over UART and prints each second. Bring-up tests 1-4 pass (3.1 and 3.2 confirmed on the DK by the author).

Saleae capture of `BUZZER_EN` (P0.31, ch 0, 2 MS/s, 30 s, clock app at `30c3b7c-dirty`, alarm 07:00 set over UART): 10 bursts, each 1.00 s (0.998-1.004), start-to-start 2.00 s (1.997-2.004), tone 2695.8 Hz. That matches the 1 s on / 1 s off, 2.7 kHz design from the pre-rewrite note (`2026-10-17_alarm-schedule-and-buzzer.md`). The pre-rewrite raw capture was not archived; the comparison is against the design values, not a stored file.
