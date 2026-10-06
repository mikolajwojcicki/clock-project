# DCF-77 generator (Nucleo-F411RE)

Bench signal source: emits logic-level DCF-77 frames on `PA0` (Arduino `A0`) so
the DK bring-up DCF test can be checked without radio reception. Bare-metal C,
no STM32Cube, no RTOS. Lab equipment, not product firmware.

Board: Nucleo-F411RE, revision `MB1136 C-04` (8 MHz ST-LINK `MCO` into
`OSC_IN`, so HSE bypass is expected). The PLL runs the core at 77.5 MHz
(HSE 8 MHz: M=8, N=310, P=4; HSI fallback: M=16), so TIM3 divides it to an
exact 77.5 kHz carrier. With HSI the carrier is disabled (about 1% off).

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
| Carrier | `PA6` (`D12`) | TIM3 CH1, 77.5 kHz square wave to coupling loop via series resistor to GND |
| LED `LD2` | `PA5` | mirrors `PA0` |
| VCP TX/RX | `PA2` / `PA3` | AF7 |

## Carrier keys and levels

| Key | Action |
| --- | --- |
| `l` | cycle level 0 / -6 / -12 / -20 dB (prints `TIM3_CCR1` values) |
| `r` | carrier on/off (`PA6` low when off) |

Reset default: on, -20 dB. Duty is out of `ARR+1 = 1000`; the pulse value
gives a 15% dip while `PA0` is high.

| Level | Idle `CCR` | Pulse `CCR` |
| --- | --- | --- |
| 0 dB | 500 | 48 |
| -6 dB | 167 | 24 |
| -12 dB | 80 | 12 |
| -20 dB | 32 | 5 |

Build flag `make CFLAGS_EXTRA=-DGEN_FORCE_HSI` skips HSE to test the fallback.

## Test record

2026-10-06, Nucleo-F411RE `MB1136 C-04`, firmware build `Oct  6 2026 21:01:39`.

| Item | Result |
| --- | --- |
| Clock source | External 8 MHz (HSE bypass from ST-LINK MCO) |
| Analyzer (Saleae, `PA0` on ch 1, 1 MS/s, 190 s) | Widths 100.0 / 200.0 ms; spacing 1000 ms, minute gap 2000.19 ms (limit 2 ms); 3 frames decoded from pulses match the UART log bit for bit |
| Fault keys on analyzer (260 s) | `g`: one 10 ms pulse at ms 500; `m`: one 2000 ms gap; `s`: 40 s low, resumes at next frame start; `p`: decoded frame matches the `[parity fault]` log line |
| DK bring-up DCF test (button 3, `PA0` -> 1 kOhm -> `P0.25`, GND common) | `t=160s valid=157 invalid=0`; 3 minute markers 60 s apart; the 2 complete frames decoded from the DK log equal the generator `bits=` strings |
| DK with `g` | `pulse width=10 ms period=500 ms -> invalid`, then back to valid (`t=210s valid=205 invalid=1`) |
| DK with `m` | One `period=2000 ms` pulse, reported as `-- minute marker --` (mid-frame). The DK classifier cannot tell a dropped second from a minute gap by period alone |

Captures were not saved to the repository (Logic 2 capture ids 2 to 5, CSV in `/tmp`).
Known quirk: `PA0` pulses for about 5 ms while OpenOCD resets the board.
