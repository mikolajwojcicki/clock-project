## 1. Gate

- [ ] 1.1 Confirm stage 5 (`add-deep-sleep-and-current-measurement`) is archived and the clock app runs a WFE loop. Do not start section 2 before this.

## 2. SoftDevice and memory layout

- [ ] 2.1 Add S112 7.2.0 (`$NRF5_SDK_DIR/components/softdevice/s112`) to `firmware/apps/clock/armgcc/Makefile`: hex merge or flash step, include paths, `nrf_sdh`, `nrf_sdh_ble`, `nrf_sdh_soc`, `nrf_ble_gatt`, `nrf_ble_qwr` sources, `-DS112 -DSOFTDEVICE_PRESENT -DNRF_SD_BLE_API_VERSION=7`.
- [ ] 2.2 Update `sdk_config.h`: enable `NRF_SDH`, `NRF_SDH_BLE` (peripheral 1, central 0, one GATT service, MTU 247), `NRF_SDH_SOC`, `NRF_SDH_CLOCK_LF_SRC` = XTAL, `NRF_FSTORAGE` with the SoftDevice backend.
- [ ] 2.3 Boot-enable the SoftDevice in `board_time_init`; remove the `nrfx_clock_*` calls; keep RTC1. Read the required RAM start from the `nrf_sdh_ble_default_cfg_set` log and set flash and RAM origins in `clock_gcc_nrf52.ld`. Reserve the last 4 KB page.
- [ ] 2.4 Flash SoftDevice and app with OpenOCD (add a Makefile target). Verify on the DK: clock, e-paper, DCF sync, motion dismiss and buzzer still work with no BLE window open. Record the flash and RAM sizes in a note under `notes/log/`.

## 3. Alarm table and codec

- [ ] 3.1 Change `alarm.c/.h` to 7 x 8 slots addressed by (day, slot). Reject duplicate enabled HH:MM on one day. Keep the trigger on minute change only.
- [ ] 3.2 Add `sched_codec.c/.h`: encode and decode the 113 B format, return an error code, never touch the live table on failure.
- [ ] 3.3 Add `sched_store.c/.h`: `nrf_fstorage` record (magic, version, table, CRC16) in the last page; load at boot, bad record means all slots disabled; save after each accepted change.
- [ ] 3.4 Update the UART parser in `main.c`: `alarm <d> <s> HH:MM on|off`, short form as slot 1 alias, `alarm?` lists used slots. Update the boot banner text.
- [ ] 3.5 Verify on the DK: set slots over UART, reset, `alarm?` shows them. Corrupt the page and confirm all slots are disabled. Reset while the buzzer sounds and confirm it stays off.

## 4. BLE service

- [ ] 4.1 Add `ble_sched.c/.h`: GAP name `ShakeClock`, vendor UUID base `e1430d6d-6e9b-4b29-ba99-6f30cbe203d0`, service 0x0001, characteristic 0x0002 (113 B, read and write, variable length), MTU 247, long write.
- [ ] 4.2 Validate writes through `sched_codec` in the authorize path; reject with an ATT error. On accept: replace the table, save, print `ble: write ok` and the table.
- [ ] 4.3 Add `board_button_held_ms()` for the DK buttons. In the main loop open the window on a 2 s hold when no alarm is active: 100 ms interval, 0 dBm, 60 s timer, print `ble: adv start` and `ble: adv stop`.
- [ ] 4.4 Link policy: no pairing; disconnect 30 s after the last write or 60 s after connect with no write.
- [ ] 4.5 Alarm priority: on alarm start call a stop function that ends advertising and drops the link. Hold during an alarm only silences it.

## 5. Verification

- [ ] 5.1 With nRF Connect for Desktop (BLE app): scan finds `ShakeClock`; connect; read returns 113 B; write a valid table and confirm `ble: write ok` and a matching `alarm?`.
- [ ] 5.2 Write bad values: 21 B, version 2, minute 1440, duplicate minute. Each fails and the table is unchanged.
- [ ] 5.3 Window and link timing: no advertising after boot; 2 s hold opens it; it closes after 60 s; idle link drops at 60 s; link drops 30 s after a write. Time them from the UART log.
- [ ] 5.4 Alarm during a connection: set an alarm a minute ahead, stay connected, confirm the link drops and the buzzer starts on time. Confirm a write during the matching minute does not fire the alarm.
- [ ] 5.5 Write a dated note under `notes/log/` with the results, and run `openspec validate add-ble-alarm-schedule-service --strict`.
