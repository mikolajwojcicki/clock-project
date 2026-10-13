# Resume here (next session)

**Last stop:** 2026-10-13 — clock UART + upright e-paper on DK. Full note: [`log/2026-10-13_clock-uart-and-epaper.md`](log/2026-10-13_clock-uart-and-epaper.md).

**Firmware tip:** `1486fbd` (`firmware: uppercase weekday abbreviations on the face`). Parent `main` may be ahead of origin; do not push unless asked.

## First action tomorrow (~2 min)

```bash
picocom -b 115200 /dev/ttyACM0
```

Press DK RESET, send `2026-10-14 HH:MM:SS`, confirm `set` and upright panel (ribbon at bottom → weekday top, seconds by ribbon). Exit: Ctrl-A Ctrl-X.

## Then

1. Photo for task 4.1 → `notes/media/`
2. Start 24 h UART drift log → `notes/measurements/`
3. Tick OpenSpec task 4.1 only after photo + CSV exist

## Do not restart unless broken

UART stack is `app_uart` / UARTE (not register UART). E-paper writes both RAMs before and after full update. No Zephyr / nrf-bm.
