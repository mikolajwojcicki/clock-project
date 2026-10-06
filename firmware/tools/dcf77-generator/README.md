# DCF-77 generator (Nucleo-F411RE)

Bench signal source: emits logic-level DCF-77 frames on `PA0` (Arduino `A0`) so
the DK bring-up DCF test can be checked without radio reception. Bare-metal C,
no STM32Cube, no RTOS. Lab equipment, not product firmware.

Board: Nucleo-F411RE, revision `MB1136 C-04` (8 MHz ST-LINK `MCO` into
`OSC_IN`, so HSE bypass is expected; HSI 16 MHz is the fallback).

## Build and flash

From this directory, with the Nucleo on USB:

```bash
make          # arm-none-eabi-gcc, warnings are errors
make flash    # openocd -f board/st_nucleo_f4.cfg, program verify reset
make clean
```

ST-LINK needs USB access (`/etc/udev/rules.d/60-openocd.rules` already covers
`0483:374b`). Re-plug the board if `make flash` reports `LIBUSB_ERROR_ACCESS`.

## Serial console

ST-LINK virtual COM port on `USART2` (`PA2`/`PA3`), 115200 8N1:

```bash
picocom -b 115200 /dev/ttyACM0
```

Press the black RESET button to see the startup banner (clock source, default
time, commands).

## Pins

| Signal | Pin | Note |
| --- | --- | --- |
| DCF output | `PA0` (`A0`) | push-pull, high during pulse |
| LED `LD2` | `PA5` | mirrors `PA0` |
| VCP TX/RX | `PA2` / `PA3` | AF7 |

## Output

Pulse at the start of seconds 0 to 58: 100 ms = bit `0`, 200 ms = bit `1`.
Second 59 has no pulse, so the next pulse marks the minute. A frame encodes the
minute that starts at the next mark. Zone bit 17 = CEST, 18 = CET.

The default start time is the build time, zone CEST (shown in the banner).

## UART commands

Single keys act immediately. `T` starts a line, finished with Enter.

| Input | Effect |
| --- | --- |
| `T YYYY-MM-DD HH:MM S\|W` | Set time (`S` = CEST, `W` = CET). Applies to the next frame; bad date or format prints `ERR` and changes nothing |
| `p` | Wrong parity (bit 28) in the next frame, once |
| `d` | Wrong date in the next frame (day + 1, wraps to day 1), parity correct, once |
| `m` | Skip the pulse in the next second, once |
| `g` | 10 ms glitch at ms 500 of the next second, once |
| `s` | Toggle silence: `PA0` stays low; resumes at the next frame start after the second press |
| `n` | Clear pending one-shot faults (does not change silence) |
| `?` | Print the command list |

## Log format

One line when each frame starts (about when the DK sees its first pulse):

```text
frame 2026-10-06 20:15 CEST Tue bits=<59 chars, bit 0 first> [parity fault] [wrong-date fault]
```

The date shown is the date actually encoded, so a wrong-date frame shows the
wrong date. Match the `bits=` string to the DK bring-up log.
