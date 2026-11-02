# 2026-11-02 first consultation

Change `prepare-first-consultation`. Date is the preparation date; rename with `git mv` if the meeting is on another day.

## Pack

Files by path, no bundle. Datasheets are cited from `clock-project-latex/bibliography.bib`.

| Item | Path | Check |
| --- | --- | --- |
| Hardware block diagram | `hardware/docs/hardware-block-diagram.pdf` | `pdfinfo`: 1 page |
| L1 overview | `firmware/docs/diagrams/l1-overview.pdf` | 1 page |
| L2 modules | `firmware/docs/diagrams/l2-modules-{a,b}.pdf` | 1 page each |
| L3 detail | `firmware/docs/diagrams/l3-detail-{a..f}.pdf` | 1 page each |
| BLE overview | `firmware/docs/diagrams/ble-overview-{a,b}.pdf` | 1 page each |
| BLE sequence | `firmware/docs/diagrams/ble-sequence-{a,b}.pdf` | 1 page each |
| Prototype | nRF52 DK running `firmware/apps/clock/`, web page `web/` | see demo |
| Latest measurement | `notes/log/2026-11-02_refresh-on-demand.md` | fallback evidence |

Not in the pack: `firmware/docs/software-flowchart.*` (replaced by the layered set).

Show order: L1, L3 parts, BLE overview. L2 and the sequences in reserve.

Drift check 2026-11-02: `git diff --stat 8c415fe HEAD -- firmware/apps/clock firmware/src firmware/boards web` is empty, working tree clean on those paths. The diagrams match the code.

## Not implemented or partial

From `firmware/docs/diagrams/README.md` (spec-vs-code table and as-built notes). Paths relative to `firmware/` unless they start with `web/`.

| # | Behavior | State | Source |
| --- | --- | --- | --- |
| 1 | Battery measurement | not implemented, SAADC not used | none (no SAADC driver; `docs/diagrams/README.md` peripherals table) |
| 2 | System OFF | not used; deepest state is System ON sleep | `src/board.c` |
| 3 | Supply switch for LIS3DH and e-Paper | none; LIS3DH power-down by register, panel deep sleep by command; only DCF has `PON` gating | `src/lis3dh.c:lis3dh_power_down`, `src/epd.c:refresh_panel` |
| 4 | Low-battery indicator on the face | not implemented (needs 1) | `src/clock_face.c:clock_face_black` |
| 5 | Alarm status on the face | not drawn; face shows weekday, HH:MM, seconds | `src/clock_face.c:clock_face_black` |
| 6 | BLE disconnect after a rejected write | only an accepted write restarts the 30 s timer | `src/ble_sched.c:ble_sched_poll` |
| 7 | 2 s hold during an alarm only silences it | hold not read while the alarm runs; may start advertising after silence | `apps/clock/src/main.c:poll_ble`, `src/board.c:board_button_held_ms` |
| 8 | DCF retry at the next full hour | retry 1 h after the window closed | `apps/clock/src/main.c:poll_dcf` |
| 9 | Console `dcf sync` via scheduled path | opens the window directly, skips the alarm check | `apps/clock/src/main.c:accept_line` |
| 10 | Alarm timeout 59 s vs 60 s | code 60 s; two specs disagree | `src/alarm.h`, `src/alarm.c:alarm_step` |
| 11 | Buzzer "on" as steady level | 2.7 kHz PWM tone (timing matches) | `src/buzzer.c:buzzer_tone_start` |
| 12 | `alarm?` lists slots with non-zero time | lists enabled slots only | `apps/clock/src/main.c:accept_line` |
| 13 | Refresh request during a refresh kept; no refresh in DCF window | request can be lost; window can open during a refresh (likely, not run) | `apps/clock/src/main.c:draw`, `src/epd.c:wait_idle` |
| 14 | Wake sources: free-running 1 s RTC tick | one RTC1 compare at the earliest deadline; UARTE0 and SoftDevice also wake | `apps/clock/src/main.c:arm_wake`, `src/board.c:board_wake_at` |
| 15 | UART does not keep the MCU awake | default `SLEEP_UART=1` keeps UARTE0 RX on | `src/board.c:con_init`, `apps/clock/armgcc/Makefile` |
| 16 | LIS3DH identity check at start | clock app never calls `lis3dh_who_am_i` | `src/lis3dh.c`, `apps/clock/src/main.c:main` |
| 17 | Service UUID in the advertising packet | UUID in scan response; spec UUID differs from used one | `src/ble_sched.c:ble_sched_init`, `src/ble_sched.c:s_base`, `web/schedule.js` |
| 18 | Panel sleep after BUSY timeout | timeout returns before deep-sleep 0x10 (likely, not run) | `src/epd.c:refresh_panel` |
| 19 | Boot refresh | done but not in the spec list | `apps/clock/src/main.c:main` |

## Demo rehearsal

## Board vs block diagram

## Consultation

## Changed requirements
