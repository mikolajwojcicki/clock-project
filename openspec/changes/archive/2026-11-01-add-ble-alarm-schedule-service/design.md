# Design

## Context

The clock app (`firmware/apps/clock`) is bare-metal nRF5 SDK 17.1.0, SoftDevice-free. `main.c` runs a busy-poll loop (`for(;;) app_poll()`, no WFE). `board_time_init` starts LFCLK with `nrfx_clock_*` and RTC1 is the timebase. `app_uart`, `nrfx_gpiote` and `nrfx_spim` are in use. The alarm table is 7 slots in RAM, lost at reset. See proposal.md for motivation.

A SoftDevice changes four things: it owns the flash start and RAM start, it owns LFCLK/POWER/CLOCK, it owns RTC0 and TIMER0, and it forces flash writes through `sd_flash_*`. This change is ordered after stage 5 (deep sleep). Artifacts are written now; code waits for the gate in tasks.md.

## Goals / Non-Goals

**Goals:**
- Read and replace the 56-slot table over one GATT characteristic.
- Keep the alarm path independent of BLE: alarm wins, buzzer timing untouched.
- Keep the radio off except in a user-opened 60 s window.

**Non-Goals:**
- Pairing, bonding, notifications, BLE time set.
- BLE current measurement (stage 5 change).
- Web page (stage 9).

## Decisions

**SoftDevice S112 7.2.0.** Peripheral-only, smaller than S132 (about 100 KB flash vs 152 KB; sizes to be confirmed from the release notes). We only advertise and accept a write. S112 supports nRF52832. Rejected: S132 (central role unused, costs flash and RAM).

**Clock ownership.** The SoftDevice starts LFCLK from the 32.768 kHz crystal (`NRF_CLOCK_LF_SRC_XTAL`, 20 ppm) through `nrf_sdh`. `board_time_init` stops calling `nrfx_clock_*`, enables the SoftDevice first, then starts RTC1 as today. The SoftDevice uses RTC0, so RTC1 does not clash. `app_uart`, `nrfx_gpiote` and `nrfx_spim` stay. GPIOTE and SPIM are not SoftDevice-reserved.

**Memory layout.** App flash origin moves above the S112 region. RAM origin moves above the SoftDevice RAM. Values come from the `nrf_sdh_ble_default_cfg_set` call at boot (the log prints the required RAM start) and are written into `clock_gcc_nrf52.ld`. The last 4 KB flash page is reserved for the schedule record and excluded from the app region.

**Storage.** `nrf_fstorage` with the SoftDevice backend on the last page. Record: magic, version, 112 B table, CRC16. Each accepted change erases the page, then writes. At boot, a missing record or bad CRC means all slots disabled. Rejected: FDS (garbage collection is not needed for one record). Energy: flash idle cost is zero; one erase + write per schedule change is a few events in the device lifetime. The alarm state is never stored.

**Wire format.** One characteristic, 113 B: version byte (1), then 7 days x 8 slots x u16 LE (bit 15 enable, bits 0-10 minute). Validation runs in a pure function `schedule_decode(const uint8_t*, len, table_out)` that returns an error code and never touches the live table on failure. The GATT write handler calls it in the authorize-write path, so a bad value gets an ATT error (`BLE_GATT_STATUS_ATTERR_INVALID_ATT_VAL_LENGTH` or `..._WRITE_NOT_PERMITTED`-class error as appropriate) before the value is stored. Long write: ATT MTU requested 247; the characteristic uses variable length with a 113 B maximum and the handler accepts the executed prepared write. Rejected: 7 per-day characteristics (not atomic across days).

**Identity.** Name `ShakeClock` in the scan response, service UUID in the advertising data. Base UUID `e1430d6d-6e9b-4b29-ba99-6f30cbe203d0`, schedule characteristic at 0x0002 (service at 0x0001). Vendor-specific UUID registered with `sd_ble_uuid_vs_add`.

**Advertising window.** New `board_button_held_ms()` reports how long any DK button has been held; the final PCB maps its own pin. The main loop opens the window when the hold reaches 2000 ms, no alarm is active and no window is open. Connectable undirected, 100 ms interval, 0 dBm. A 60 s one-shot timer on the app timebase (RTC1 tick) closes it. UART prints `ble: adv start` and `ble: adv stop`. A press while the alarm sounds only silences it (existing path wins; the hold check is skipped when the alarm is active).

**Link policy.** Open link, no security. A connect stops advertising. The app records connect time and last-write time from the RTC1 tick and calls `sd_ble_gap_disconnect` at 30 s after the last write or 60 s after connect with no write. Default connection parameters are accepted.

**Alarm priority.** On alarm start the app calls `ble_stop()` which stops advertising and disconnects at once. The buzzer and motion-dismiss code is not changed. A table write never calls the trigger check; the trigger runs only on minute change, so the spec rule holds without special cases.

**UART cutover.** `alarm.h` set/get takes (day, slot). `alarm <d> <s> HH:MM on|off` is the main form; `alarm <d> HH:MM on|off` stays as an alias for slot 1. `alarm?` prints slots that are enabled or non-zero (up to 56 lines). Both UART and BLE paths call one function `alarm_table_replace`/`alarm_slot_set` that validates duplicates and triggers a save.

**Module split.** New files in `firmware/apps/clock/src`: `ble_sched.c/.h` (SoftDevice init, advertising, GATT, link policy), `sched_codec.c/.h` (pure encode/decode/validate), `sched_store.c/.h` (fstorage + CRC). `alarm.c` keeps only the table and trigger logic. `main.c` gains the hold check and calls `ble_sched_poll()` from `app_poll()`.

## Risks / Trade-offs

- **Busy-poll loop plus SoftDevice** works but wastes power; the power cost is measured in stage 5, not here. Gate: stage 5 first so the SoftDevice lands on a WFE loop.
- **Flash write blocks the CPU and the radio** for about 85 ms per page erase, with the SoftDevice scheduling it around radio events. A write during the 2 s before an alarm is acceptable; alarms fire on the RTC1 tick and are not lost, but the buzzer can start up to one erase late. Accepted.
- **Open link, no pairing:** anyone in range during the 60 s window can overwrite the schedule. Accepted by the user: the window is opened by hand, and the table holds no secrets.
- **Flash wear:** 10,000 erase cycles per page minimum; one erase per write, far above the expected count.
- **Memory numbers are unverified** until the first SoftDevice build; the linker values in tasks come from the boot log, not from this document.
- **`nrfx_clock` removal** may break the DCF or e-paper timing if any code assumes HFCLK is on; HFCLK for SPIM is handled by the SoftDevice-aware path, to be checked on the bench.
