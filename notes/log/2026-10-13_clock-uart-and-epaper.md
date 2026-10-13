# 2026-10-13 — clock UART console and upright e-paper

OpenSpec task **4.1** (RTC clock on LFXO + e-paper): firmware works on the DK; photo and 24 h drift CSV still missing.

- **Done:** Clock app on PCA10040 receives `YYYY-MM-DD HH:MM:SS` over the DK VCOM (`/dev/ttyACM0`, 115200 8N1), echoes the line, prints `set … weekday`, advances on LFXO/RTC1, and full-refreshes the Waveshare 2.13" V4 each minute. Panel layout with the ribbon at the **bottom**: weekday on top (`WT` for Tuesday), `HH:MM` middle, seconds near the ribbon. Minute updates stay upright (`22:00` → `22:01` → `22:02`).
- **Failed / discarded:**
  - Register UART (`NRF_UART0`) printed the banner but ignored host RX (picocom Enter / keystrokes).
  - Saleae chasing RX while SPI hung the CPU.
  - Gate-scan TB bit and a software 180° rotate made the glass worse.
  - Glyph row flip masked buffer swap (`2`→`5`, `Wt`→`M` on alternate minutes).
  - Lowercase weekday letters (`Wt`) drew as `W:` (no lowercase glyphs).
- **Fix:**
  - Console: Nordic `app_uart_fifo` + UARTE EasyDMA, same path as `examples/peripheral/uart` (flow control off). Clock-only via `-DCLOCK_APP` and `firmware/apps/clock/config/sdk_config.h`.
  - SPI: clear `EVENTS_END` with a read-back before `START` (1-byte transfers never finished; CPU stuck in `spi_bus_transfer`, UART RX never drained). Call `spi_bus_init()` before the first refresh.
  - E-paper: Waveshare `Init(FULL)` sequence; write RAM `0x24` then `0x26`; after full update write both again (GxEPD2 `writeImageAgain`) before deep sleep so SSD1680 buffer swap does not flip the next minute.
  - Digit `0` oval glyph; weekday abbreviations uppercase.
- **Why:** Thesis need is a console-settable LFXO clock on the breadboard panel before sleep/BLE work. Official UART example proved the VCOM path; buffer-swap write matches SSD1680 practice.
- **Evidence:** Commits `2b7e3fd` … `1486fbd` (UART through weekday fix). Latest clock tip at session end: `1486fbd`. How to re-check: open picocom first, RESET DK, send a time line, wait two minute refreshes.

## Handoff for tomorrow

Resume from here — do not reopen UART/RX or e-paper orientation unless something regresses.

1. **Close task 4.1 evidence:** light-theme photo `notes/media/YYYY-MM-DD_clock_face.ext` (ribbon at bottom, `WT` / time / seconds). Start a 24 h UART capture → CSV under `notes/measurements/` with instrument (or “DK VCOM”), temperature, supply, firmware commit.
2. **Optional polish:** seconds on the face only update each minute (full refresh); UART already prints every second — decide if that is enough for the thesis photo.
3. **Not today:** WFE/deep sleep, BLE schedule upload, LIS3DH dismiss, DCF sync, perfboard solder — roadmap after 4.1 evidence is filed.
4. **Dirty tree left alone:** `firmware/docs/software-flowchart.svg`, `hardware/kicad/.history`, teams dump / `1.png` / `2.png` / `Latex.pdf` — do not mix into firmware commits.
