# Bring-up (nRF52 DK breadboard prototype)

Test firmware for the breadboard prototype in
[`hardware/docs/breadboard-prototype-guide.md`](../../../hardware/docs/breadboard-prototype-guide.md).
SoftDevice-free, nRF5 SDK 17.1.0, register-level drivers from
[`firmware/src/`](../../src/), pin map in
[`firmware/boards/dk_breadboard_pins.h`](../../boards/dk_breadboard_pins.h).

Run the guide's blinky controller test (section 7) first. This app replaces it
only after blinky passes.

## Build, flash, erase

From this directory, with the DK on USB:

```bash
make          # -> armgcc/_build/nrf52832_xxaa.hex
make flash    # OpenOCD, on-board J-Link
make erase    # mass-erase
```

VS Code: **Terminal → Run Task…** → `firmware: bringup build`,
`firmware: bringup flash`, `firmware: serial console`.

## Serial console

```bash
picocom -b 115200 /dev/ttyACM0
```

Exit with `Ctrl-A Ctrl-X`. Press the DK `RESET` button to see the banner:

```text
clock-project bringup 7020ca1 (built Oct  4 2026 16:30:00)
reset reason: 0x00000004
buttons: 1=display 2=lis3dh 3=dcf77 4=buzzer (press 2/3 again to stop)
```

- The first field after `bringup` is the git commit. Copy it to the test record
  `Firmware commit:` field. `-dirty` means uncommitted changes were built.
- `reset reason` is `NRF_POWER->RESETREAS`. `0x00000000` means power-on or
  brown-out, `0x00000001` the reset pin, `0x00000004` a software reset
  (OpenOCD after flashing).

## Safe states

The first thing `main()` does is drive the guide section 6.2 states:

| Signal | Pin | State |
| --- | --- | --- |
| `DCF_PON` | `P0.24` | High (receiver off) |
| `EPD_CS` | `P0.11` | High |
| `SENSOR_CS` | `P0.30` | High |
| `BUZZER_EN` | `P0.31` | Low |

Every test returns to these states when it ends, also on failure.

An erased chip (`make erase`) or a chip running other firmware does **not**
drive these pins. They float. Flash this app before you connect the control
wires, and after every `make erase`.

## Buttons

One test runs at a time. Each test prints its steps and ends with one line
that starts with `PASS`, `FAIL`, or `STOPPED` and names the test.

| DK button | Test | Stops |
| --- | --- | --- |
| 1 | Display | By itself |
| 2 | LIS3DH | Button 2 again, or 60 s |
| 3 | DCF-77 | Button 3 again, or 10 min |
| 4 | Buzzer | By itself, about 1.3 s |

Pressing another button while a test runs prints
`button N ignored: ... test already running`.

## Test 1: display (guide section 9.2)

Before the first run, read the label on the display's flat cable. This driver
uses the Waveshare **V4** (SSD1680) command set. If the label shows another
version, record it; a busy timeout or a garbled image is then expected.

1. Press button 1.
2. Wait about 3 s for the full refresh (the panel flashes several times).
3. Expect `PASS display: ...` and this image on the panel: a 4 px black border
   and a black-and-white checkerboard of 16 px squares.
4. Remove USB. The image must stay.

`FAIL display: EPD_BUSY did not clear within 10 s` means the controller never
reported ready. `BUSY` has an internal pull-up, so a missing `BUSY` wire
gives this failure. Check the guide section 9.2 list. A `PASS` line with a blank
panel means the firmware ran but the panel did not take the data: check
`DIN`, `CLK`, `CS`, `DC`, and `BS1 = 0`.

The panel is left in deep sleep. The next run wakes it with a hardware reset.
