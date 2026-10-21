# Handoff: finish `add-deep-sleep-and-current-measurement`

**Date:** 2026-10-22
**Repo:** `/home/wenigz/clock-project`
**Start next session with:** finishing OpenSpec change
`add-deep-sleep-and-current-measurement` (tasks 4.1 to 5.2). Use skill
`openspec-apply-change`. Planning is done; do not re-propose.

## Where things stand

- Tasks 1.x to 3.x of the change are done and committed (sleep loop, WFE,
  `SLEEP_UART` build flag, DC/DC, power-gate states).
- Open in `openspec/changes/add-deep-sleep-and-current-measurement/tasks.md`:
  - 4.1 DK on PPK2 (Source Meter, 3.0 V on `P22`; solder bridge already cut). Flash with OpenOCD first, then switch supply. Photo to `notes/media/`.
  - 4.2 idle current, at least 60 s, `SLEEP_UART=0`, CSV with metadata.
  - 4.3 e-paper refresh current (align with `EPD_BUSY` on Saleae).
  - 4.4 DCF window current (generator on, align with `DCF_PON`).
  - 4.5 alarm current with buzzer (align with `BUZZER_EN`).
  - 4.6 dated note `notes/log/2026-10-<day>_deep-sleep-current.md` with the four-state table and daily energy estimate.
  - 4.7 `notes/ai-use.md` entries, commit measurements + note.
  - 5.1 redraw `firmware/docs/software-flowchart.puml` as event driven; regenerate svg/pdf.
  - 5.2 `openspec validate add-deep-sleep-and-current-measurement --strict`.

## E-paper freeze: solved today (read before measuring)

- Log: `notes/log/2026-10-22_epd-sck-ringing.md`.
- Cause: ringing on `SPI_SCK` (`P0.03`). Without extra load every refresh
  failed (`display FAIL 10043 ms`, BUSY high). One dangling LA probe on
  display `CLK` fixes it (`display ok ~7750 ms`).
- **Current bench state:** the dangling probe stays clipped on `CLK` until the
  user buys a ~100 Ω resistor (47 to 220 Ω). Do not remove it, or refreshes
  fail and the 4.3 measurement is invalid.
- When the resistor is fitted: series in SCK right at `P0.03`, remove all
  probes, verify 3 refreshes + one power cycle on UART (`display ok`), add the
  result to the log note.
- Rule recorded for the perfboard: `hardware/docs/breadboard-wiring-table.md`
  section 2 and `hardware/bom/proto-bom.md` item 12.
- Separate real bug fixed in `b7e7cdc`: `nrfx_gpiote_in_uninit` disconnected
  the BUSY input buffer, so later waits read 0 and reported false success.
- `70b067b`: every refresh prints `display ok|FAIL <ms>` on UART.

## Ops reminders

- Serial ports by id: DK J-Link CDC is `usb-SEGGER_J-Link_*-if00` (was
  `ttyACM3`); `ttyACM0`/`ttyACM1` are the PPK2. Use nix-shell with pyserial.
- Build/flash: `make && make flash` in `firmware/apps/clock` (same as the VS Code task `firmware: clock flash`).
- Restore DK power after PPK2 work: re-bridge `P22`.
- Boot opens a DCF window; keep the generator on for boot-window tests.
