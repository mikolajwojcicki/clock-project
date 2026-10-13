# Clock (nRF52 DK breadboard)

LFXO + RTC1 civil clock. The e-paper shows weekday, HH:MM and seconds, and refreshes when the minute changes. UART prints every second.

Task 4.1 in the thesis roadmap stays open until a photo and a 24 h drift CSV exist.

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
