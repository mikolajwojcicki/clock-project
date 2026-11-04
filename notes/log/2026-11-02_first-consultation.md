# 2026-11-02 first consultation

Change `prepare-first-consultation`. Date is the preparation date; rename with `git mv` if the meeting is on another day.

## Pack

Shown as one merged PDF, [`notes/consultation/first-consultation/pakiet.pdf`](../consultation/first-consultation/pakiet.pdf), with the Polish crib sheet `sciagawka.md` next to it (reading and show order, page numbers). The table lists the sources. Datasheets are cited from `clock-project-latex/bibliography.bib`.

| Item | Path | Check |
| --- | --- | --- |
| Hardware block diagram | `hardware/docs/hardware-block-diagram.pdf` | `pdfinfo`: 1 page |
| L1 overview | `firmware/docs/diagrams/l1-overview.pdf` | 1 page |
| L2 modules | `firmware/docs/diagrams/l2-modules-{a..e}.pdf` | 1 page each |
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

2026-11-02 evening, firmware built from `a4bf3ae` (`-dirty` from unrelated files; firmware, board and web paths equal to `8c415fe`), default build (`SLEEP_UART=1`). nRF52 DK on J-Link USB, flashed with `make flash` (verify OK), reset over SWD. UART log captured on the DK VCOM at 115200 Bd. DCF input from the DCF-1060N receiver indoors (not the Nucleo generator). Web page served from `web/` on `localhost:8000` in Chromium.

| Step | Result | Observed |
| --- | --- | --- |
| Boot refresh | pass | `display ok 3716 ms` 3 s after reset; DCF window opened at boot because the time was unset |
| Set time | pass | console line `2026-11-02 21:04:45` → `set ... poniedziałek`, then one line per second. Panel did not redraw until the DCF window closed (21:09:29), by design (`apps/clock/src/main.c` main loop: no refresh inside a window). Deferred refresh at 21:09:33 (3725 ms), author confirmed correct time and weekday |
| Alarm from web page | pass | `ble adv` from console. First connection 21:06:15-21:07:15 without a write; second connection at 21:07:41, `ble: write ok`, `alarm 1 1 21:12 on` at 21:07:57; disconnect 30 s later |
| Alarm sounds | pass | `ALARM 21:12 poniedziałek` at 21:12:00, `display ok` at 21:12:04; author heard the buzzer |
| Motion dismiss | pass | author shook the board from 21:12:07; `MOTION event 1..20`, `ALARM stop (motion)` at 21:12:14 (about 7 s of shaking) |
| DCF sync attempt | fail | boot window 21:04:29-21:09:29: 9 × `dcf: rejected pulse`, then `dcf: timeout (time unchanged)`. Indoor reception at the desk, evening; no frame decoded. Generator path not tried this run |

For the meeting: the DCF step is stated as failed indoors. Either show it with the Nucleo generator or show the log.

## Board vs block diagram

`hardware/docs/hardware-block-diagram.pdf` draws the target PCB. The board on the desk (author's check, 2026-11-02):

| Diagram block | Desk | Match |
| --- | --- | --- |
| USB-C | none; power from the DK J-Link USB | no |
| MCP73831 | not in the power path today | no |
| Li-Ion | not connected today | no |
| AP2112K 3,3 V | DK onboard regulator instead | no |
| MDBT42Q-512KV2 nRF52832 | nRF52 DK PCA10040 (bare nRF52832 QFAA, no module) | partial: same SoC |
| e-paper | Waveshare 2.13" over the 8-pin header | yes |
| LIS3DH | LIS3DH breakout, INT1 wired | yes |
| DCF-1060N | DCF-1060N (MAS6180 inside) | yes |
| buzzer | AP-1205V-P1 via NPN and flyback diode | yes |
| przyciski, SWD | DK onboard buttons, SWD via DK J-Link | partial: no separate 1×4 SWD header |

The whole power chain (USB-C → MCP73831 → Li-Ion → AP2112K) is not on the desk. The battery path per `notes/agent-reference/dk-prototype-power.md` exists but was not used in this run. Say this at the meeting: the diagram is the target, the desk is the DK prototype.

## Consultation

## Changed requirements
