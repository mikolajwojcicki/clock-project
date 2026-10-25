# Handoff — deep-sleep current measurement (2026-10-24 → resume 2026-10-25)

Repo: `/home/wenigz/clock-project` (NixOS, nRF52 DK PCA10040 **v3.0.1**, PPK2).
OpenSpec change in progress: `openspec/changes/add-deep-sleep-and-current-measurement/`
(14/23 tasks; see its `tasks.md` / `design.md`). All results so far are in
**`notes/log/2026-10-24_deep-sleep-current.md`**. Read it end to end first
(runs A … A6, the 4.0 leak check, datasheet comparison). Do not repeat its
content; append to it. Last measurement commit: `5ca22ef` (run A6).

## Where we stopped

User chose **run A7 = live voltage sweep** (no soldering) as the next step.
It was proposed but **not yet run**. Procedure:

1. PPK2 output off, unplug DK USB 10 s, plug back in, wait 3 s, start recording
   (Data Logger, 100 kS/s, digital channels on), PPK2 Source Meter on at **3.3 V**.
2. DCF generator into `P0.25`; unplug once `D0` toggles at 1 Hz (~160 s in).
3. Hold 3.3 V 60 s, then step live to 3.2 / 3.1 / 3.0 / 2.9 V, 60 s each.
4. Export CSV, pack to `.ppkz` (tool below), give the agent the path.

Interpretation: steep idle-vs-VDD curve (low near 3.0 V, high at 3.3 V) ⇒
interface-MCU coupling confirmed ⇒ next step is cutting **SB20–SB27** (DK guide
v3.x.x solder-bridge table: isolate SWD, UART `P0.05–08`, RESET `P0.21`, SWO
`P0.18`). Flat ~2 µA ⇒ coupling theory wrong, investigate further.

## Key facts not obvious from the note

- Setup the user chose: **DK USB stays connected**; SB9 cut; PPK2 Source Meter
  on P22. With USB unplugged and pin reset enabled, the unpowered interface MCU
  holds `P0.21` at ~1.3 V → nRF stuck in reset (confirmed).
- Firmware on the chip: clock app, no-UART build, **pin reset restored**
  (`c7edb52`), `make erase` keeps debug access (`20783f3`). Nothing to flash for A7.
  Build/flash: `firmware/apps/clock/armgcc`, `make SLEEP_UART=0 flash ACM=/dev/ttyACM0`.
- `D0` (logic port) = firmware marker pin `P0.22`: low during init, rises at
  init done, then toggles each clock second.
- Datasheet: `temp-resources/docs/nrf/nRF52832_PS_v1.9.pdf` §17.1.1, System ON
  1.2–1.9 µA typ (DC/DC, 3 V); our config ≈ 2 µA expected. DK guide:
  `temp-resources/docs/nrf/nRF52_DK_User_Guide_v3.x.x.pdf`. `pdftotext` needs
  `nix-shell -p poppler-utils`.
- Conditions already recorded in the note: PPK2 S/N, room 24 °C.
- Captures: commit `.ppkz` + `.ppkz.json` only; raw `ppk-*.csv` stay local
  (gitignored). Pack/unpack + reading: `firmware/tools/ppk-plot/`
  (`ppk-archive.sh pack <csv>`; venv `.venv/bin/python`; run inside
  `nix-shell -p python3 stdenv.cc.cc.lib zlib` with `LD_LIBRARY_PATH` built from
  `NIX_LDFLAGS`, because numpy needs libstdc++).
- Yesterday's analysis scripts in `/tmp` are gone (reboot). They computed:
  per-10 s means, `D0` edge times, burst detection (>1 mA runs), idle median
  between ticks, tick charge from the averaged profile around `D0` edges
  (window −2 ms … +20 ms, baseline = median of the tail). For A7, add per-step
  idle medians keyed to the voltage-step times.

## Open items (separate from A7)

1. Minute redraw without a panel: ~3.1–3.3 mA flat for 9.1 s every 60 s.
   `wait_idle()` in `firmware/apps/clock/src/epd.c` loops `board_sleep()` until
   BUSY/deadline, yet current looks like CPU awake. Check before run B (panel).
2. Thesis bib entry `nRF52832PS` (in `clock-project-latex/`) cites PS v1.4; the
   figures used come from v1.9. Asked user whether to update; unanswered.
3. `hardware/kicad/.history` shows modified; untracked files in repo root
   (`1.png`, `2.png`, `Latex.pdf`, `teams-dump*`) are the user's; leave alone.
4. Commits `f3c9e8c`, `af2bb63`, `a18fb39` (perfboard shield for the DK) came
   from another session; not part of this work.

## Rules reminders

- Commit each logged run (note + `.ppkz` + sidecar) with a thesis-readable body.
- Ask the user only via the `ask` tool. UART work: nix-shell with pyserial.

## Suggested skills

- `openspec-apply-change`: continue `add-deep-sleep-and-current-measurement` tasks.
- `context-mode`: when analyzing large capture output.
- `caveman-commit`: commit messages for each run.
