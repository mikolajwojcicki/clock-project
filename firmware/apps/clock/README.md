# Clock (nRF52 DK breadboard)

LFXO + RTC1 civil clock. The e-paper shows weekday, HH:MM and seconds, and refreshes when the minute changes. UART prints every second. Console uses Nordic `app_uart` / UARTE (SDK UART example path). Hold the module with the ribbon at the bottom: weekday on top, seconds by the ribbon.

Task 4.1 in the thesis roadmap stays open until a photo and a 24 h drift CSV exist. Session handoff: [`notes/handoff-next.md`](../../../notes/handoff-next.md).

## Build and flash

From this directory, DK on USB:

```bash
make
make flash
```

## Set the time

```bash
picocom -b 115200 /dev/ttyACM0
```

Picocom does not show keystrokes unless started with `--echo`. Enter sends a carriage return. Type one line and press Enter:

```text
2026-10-13 21:05:00
```

The reply starts with `set`. After that, each second is one line:

```text
2026-10-13 21:05:01 wtorek
```

## 24 h drift

Leave it running. Copy the UART lines into `notes/measurements/`. Record the meter, temperature, supply and firmware commit next to the CSV. Compare the last line with a reference clock.

## DCF-77 sync

Wiring (from the Nucleo generator): generator `PA0` -> 1 kΩ -> DK `P0.25`, GND common. The receiver module `PON` is driven by `DCF_PON` (high = off).

- `dcf sync` opens a 5 min window now. A second command while one is open prints `dcf: window already open`.
- Windows open at boot (time not set), at 03:00, and as hourly retries after a failed window.
- At most 3 retries, then idle until the next 03:00. A window waits while the alarm sounds and is dropped after 5 min.
- Two valid frames exactly one minute apart are needed. Log lines: `dcf: ok <before> -> <after>`, `dcf: rejected <reason> (time unchanged ...)`, `dcf: timeout (time unchanged ...)`.
- Constants: `DCF_RETRIES`, `DCF_RETRY_MS`, `DCF_LATE_MS` in `src/main.c`; `DCF77_WINDOW_MS` in `dcf77_sync.h`.
- `P0.22` (`CLOCK_SEC_MARK`) is low from reset until init ends (LFXO start, first refresh), then goes high once and toggles on every clock second tick. Its first rising edge is the PPK2 digital trigger that marks "init done". Probe it with a logic analyzer next to generator `PA0` and `P0.25` to measure the second boundary.
- Pin reset is off (no `CONFIG_GPIO_AS_PINRESET`): with DK USB unplugged the unpowered interface MCU holds `P0.21` at about 1.3 V and kept the nRF in reset. The DK RESET button does nothing with this build. Run `make erase` once before the first flash, because UICR keeps `PSELRESET` from older builds. Flashing `bringup` or `blinky` turns pin reset back on.
