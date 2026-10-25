# Handoff — fix-idle-pin-leaks (written 2026-10-25, for Tuesday 2026-10-27)

Repo `/home/wenigz/clock-project`. Supersedes
`notes/handoff/2026-10-25_deep-sleep-run-C.md` (its open questions are now
answered; user plans to delete old handoffs).

## Read first

- Change: `openspec/changes/fix-idle-pin-leaks/` (proposal, design, spec
  `specs/idle-pin-states/spec.md`, `tasks.md`). Commit `eaf8456`.
- Evidence: `notes/log/2026-10-24_deep-sleep-current.md`, sections
  "runs C2-C4" and "C5" (commits `598e1a4`, `e929cc7`).
- Parent change still open: `openspec/changes/add-deep-sleep-and-current-measurement/`
  (tasks 4.x wait for this change).

## State in one table

| Leak | Size | Fix | State |
| --- | --- | --- | --- |
| `EPD_RST` (`P0.28`) held high | 2.07 mA | release after refresh | committed `07dd2de`, built, **not flashed** |
| `SCK`/`MOSI` held low vs LIS3DH breakout pull-ups | ~1.8 mA | drive both high between transfers | **not written** (task 1.2) |

## Tuesday scope (no hardware flashing possible)

External SWD cable arrives **Thursday**. DK interface-MCU SWD is cut
(SB20/SB21), so nothing can be flashed on Tuesday.

1. `/opsx:apply fix-idle-pin-leaks` → task 1.2: edit `firmware/src/spi_bus.c`
   (`spi_bus_init` + after `nrfx_spim_uninit` in `spi_bus_transfer`) to set
   `SCK`, `MOSI` high. Design decision 2 explains why the mode-0 low edge at
   `nrfx_spim_init` is harmless (all CS high).
2. Build: `firmware/apps/clock/armgcc` `make` and `make SLEEP_UART=0`; also
   `make` in `apps/bringup` and `apps/bringup-4button` armgcc dirs.
3. Task 1.3: commit on its own (`firmware:` prefix, cite C4).
4. Optional prep for Thursday: OpenOCD flash command for the external probe.

Tasks 2.x (flash, LIS3DH check, run C6) and 3.1 wait for Thursday.

## Facts not in the artifacts

- User decided: **drive high**, not release (asked via `ask`, 2026-10-25).
- C4: user swapped `SDA`/`SDO` wires by accident; the "SDO" step was `MOSI`.
- HAT `RST` measured 19.0 kΩ to GND unpowered — explains only ~0.16 mA of
  the 2.07 mA; rest only when display powered `[INFERENCE]`.
- C5 full pack was 55 MB → repacked window 180-543 s (log notes the offset).
- PPK2 CSVs `notes/measurements/ppk-20261025T*.csv` (1.2-1.5 GB) are
  gitignored and can be deleted by the user.
- Analysis helper: run Python from `firmware/tools/ppk-plot/` with its
  `.venv` (`import ppkz`; `ppkz.load_meta`, `ppkz.iter_chunks`).

## Don't stage

`hardware/kicad/.history`, `temp-resources/shit-to-buy.md`, `1.png`, `2.png`,
`AGENTS.md.baK`, `Latex.pdf`, `teams-dump*`, `temp-resources/display-flipped.jpeg`.

## Suggested skills

- `openspec-apply-change` — implement `fix-idle-pin-leaks` task 1.2/1.3.
- `caveman-commit` — commit messages.
- `context-mode` — only if analysing a new PPK2 capture (Thursday).
