# Bring-up (nRF52 DK breadboard prototype)

> Frozen archive of `../bringup` as of commit `f13e003` (4 DK buttons: 1 display,
> 2 LIS3DH, 3 DCF-77, 4 buzzer). Build with `make` here; not maintained further.

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

## Test 2: LIS3DH (guide section 10.2)

1. Put the board on a stable table and press button 2.
2. Expect `WHO_AM_I = 0x33 (expect 0x33)` and `identification passed`.
3. Move or tap the board. Each movement prints
   `INT1 event N: src=0x.. X Y Z` with the axes that crossed about 256 mg.
4. Stop moving. The events must stop. A high-pass filter removes gravity, so a
   still board gives no events in any orientation.
5. Press button 2 again (or wait 60 s).

The last line is `PASS lis3dh: N INT1 events (...)` if at least one event
arrived, else `STOPPED lis3dh: 0 INT1 events`. The sensor is powered down at
the end.

| Result | Meaning |
| --- | --- |
| `FAIL lis3dh: WHO_AM_I mismatch 0xff` | `MISO` reads high: no sensor answer. Check `SDO`, `CS`, `VIN`. |
| `FAIL lis3dh: WHO_AM_I mismatch 0x00` | `MISO` reads low or the clock does not reach the sensor. Check `SCK`, `GND`. |
| ID passes, no events | Check the `INT1` wire to `P0.23`. `INT1` has an internal pull-down, so a missing wire gives no events. |

## Test 3: DCF-77 (guide section 11.3)

Connect `PON` to `P0.24` only after this firmware is flashed: it holds `PON`
high (receiver off) except during this test. Connect `OUT` to `P0.25` only
after you measured its levels with the logic analyzer (guide section 11.2).

1. Place the antenna away from the computer and USB cable.
2. Press button 3. `PON` goes low and the receiver turns on.
3. Each pulse prints `pulse width=... ms period=... ms -> 0 | 1 | invalid`.
   A good signal gives about one line per second, widths near 100 ms (`0`)
   or 200 ms (`1`), and `-- minute marker --` once per minute.
4. Every 10 s a `t=...s valid=... invalid=...` line shows the counts.
5. Press button 3 again, or wait 10 min. `PON` goes high again.

The last line is always `STOPPED dcf77: valid=... invalid=... minute_markers=...
overruns=...`. Zero valid pulses is an observation for the test record
(location, time, antenna direction), not a wiring failure.

With `OUT` unconnected, the input floats: expect no lines or random `invalid`
lines.

The firmware assumes `OUT` is active high (pulse = high) with no pull
resistor. If every width is about 800 to 900 ms, the output is inverted: set
`DCF_OUT_ACTIVE_LEVEL` to `0` in
[`dk_breadboard_pins.h`](../../boards/dk_breadboard_pins.h). If `OUT` never
changes but the logic analyzer showed it only pulls low, it is open collector:
set `DCF_OUT_PULL` to `NRF_GPIO_PIN_PULLUP`. Rebuild and flash after either
change.

Classifier windows: `0` = 40 to 140 ms, `1` = 150 to 260 ms, minute marker =
1700 to 2300 ms between pulse starts. Host check:

```bash
gcc -I firmware/src firmware/tests/dcf77_classify_test.c \
    firmware/src/dcf77_classify.c -o /tmp/dcf_test && /tmp/dcf_test
```

If `gcc` is not on `PATH`, prefix the command with `nix-shell -p gcc --run '...'`.

## Test 4: buzzer (guide section 12.3)

The AP-1205V-P1 drive type is not confirmed. It can have its own oscillator
(needs a steady level) or need an external tone. This test tries both:

1. Press button 4.
2. `step 1/2: BUZZER_EN steady high for 500 ms`, then 300 ms off.
3. `step 2/2: 2.7 kHz tone for 500 ms`.
4. Expect `PASS buzzer: both steps ran, BUZZER_EN low; ...`. `PASS` means the
   firmware finished both steps. Listen for the sound yourself.

Write in the test record which step sounded. That is the buzzer's drive type.
If neither step sounds, follow guide section 12.3 with USB removed.

If the DK resets during a step, the next banner shows the reset reason.
`0x00000000` (power-on or brown-out) means the buzzer current pulled the
supply down. Record it and stop the buzzer test.
