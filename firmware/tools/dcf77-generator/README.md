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
