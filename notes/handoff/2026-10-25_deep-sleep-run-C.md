# Handoff — deep-sleep current, runs C (2026-10-25 → next session)

Repo `/home/wenigz/clock-project`, OpenSpec change
`openspec/changes/add-deep-sleep-and-current-measurement/` (see its
`tasks.md`, `design.md`). Run log so far:
`notes/log/2026-10-24_deep-sleep-current.md` (A runs up to A8, commit
`a6004a7`). Previous handoff: `notes/handoff/2026-10-24_deep-sleep-current.md`
(A-run context; superseded by this one for what's next).

## Where we are

1. **A runs done.** SB20/SB21/SB26/SB27 are CUT on the DK (v3.0.1). nRF-only
   idle after the cut: ~2.0 µA at 3.0 V, ~3.5 µA at 3.3 V (A8, committed).
   SB22–SB25 (UART) still closed; user chose to move on without cutting them.
2. **Flashing is currently impossible** (on-board J-Link can't reach the nRF:
   `cannot read IDR`). User ordered a 10-pin IDC cable for DK **DEBUG OUT/IN**
   headers, arrives by Thursday. Alternative documented in
   `scratch/sb-isolation-research/PLAN.md` §9 (branch `autoresearch/...`,
   commit `f09714e`, nested repo, gitignored): jumper TP9→P18 pin 2 (SWDIO),
   TP10→P18 pin 4 (SWDCLK), never P18 pin 1; `make SLEEP_UART=0 flash`.
   The researcher missed the IDC headers; prefer the cable.
   Firmware on the chip: no-UART clock build, `P0.22` marker (low in init,
   high after init, then 1 Hz toggle after DCF sync).
3. **Run C wiring:** breadboard `3V3` rail fed from PPK2 `VOUT` (same line
   as P22), so nRF + display + LIS3DH + DCF on one measured rail, 3.0 V.
   DK USB plugged in. DK `P1 VDD` not connected to the breadboard. `D0` on
   `P0.22`. DCF generator = Nucleo, antenna next to DCF module (no wire).

## Uncommitted captures (commit with a note, NOT the CSVs)

- `notes/measurements/ppk-20261025T195017.ppkz` (+ `.json`) — **C2**, 300 s,
  3.0 V, generator on. Results:
  - boot init 0–12 s ≈ 7.2 mA; DCF window 12–135 s = 4.21 mA flat;
    sync OK, `D0` 1 Hz toggling from 148.9 s;
    idle 150–190 s = **3.87 mA** (p1–p99 3.78–3.97);
    minute refresh ≈ 19 s, mean 6.0 mA, peak 20–21 mA;
    receiver cost = 4.21 − 3.87 = 0.34 mA.
- `notes/measurements/ppk-20261025T201314.ppkz` (+ `.json`) — **C3 isolation**,
  436 s. User unplug times (from recording start): LIS3DH power 3:10,
  DCF `VDD` 5:10, display `VCC` 6:05.
  - idle 3.87 → **2.08 mA** at LIS3DH unplug (−1.8 mA);
    DCF unplug: 0; display unplug: −0.015 mA → 2.06 mA left.
  - **Glitch:** 230–288 s current ≈ 11.8 mA (one refresh lasting ~55 s),
    35 s after LIS3DH unplug, cause unknown (floating MISO/INT1 or display
    BUSY-timeout candidates, unverified). Refreshes here ≈ 10 s vs 19 s in C2,
    unexplained.
- `notes/media/2026-10-25_deep-sleep_ppk2-C.jpg` — user's bench photo, belongs
  with the C note.
- Write the C section into `notes/log/2026-10-24_deep-sleep-current.md` (or a
  new dated note `notes/log/2026-10-25_...`) and commit these.

## Findings to record

- **LIS3DH wiring error since project start:** `3V3` goes into breakout
  **`3Vo`**, not `VIN`. `hardware/docs/breadboard-wiring-table.md`,
  `breadboard-prototype-guide.md` say `VIN`; fix docs (or rewire to `VIN` —
  decide with user). `hardware/docs/perfboard-shield.md` line 45 specifies
  `VIN` with `3Vo` open.
- Breakout power LED: checked against Adafruit schematic (2026-10-25).
  Rev B (STEMMA QT, 2020): green LED + 10 kΩ `R1G$4` from 3.3 V to GND,
  isolated branch → desolder safe, but draws only ≈ 0.1 mA at 3.0 V, so it
  cannot explain 1.8 mA. Rev A (2015) has no LED. Better suspects on rev B:
  BSS138 level shifter with 10 kΩ pull-ups on both sides of `SCL`/`SDA`
  (`R3G$1..4`, ~0.3 mA each while nRF holds SCK/MOSI low) and the AP2112K
  back-fed through `3Vo` `[INFERENCE]`. Perfboard still keeps the breakout.
- **2.07 mA still unexplained** with all peripherals unpowered but signal
  wires still on nRF pins. Suspect back-feed via ESD diodes / breakout
  pull-ups `[INFERENCE]`.

## Next actions (in order)

1. With LIS3DH powered and idle, pull breakout `SCL` then `SDA` wires
   (30 s each), watch idle current. Drop ≈ 0.5 mA per line → pull-ups are
   the cause. Then desolder LED (expect ≈ 0.1 mA drop).
2. Signal-wire unplug run (~8 min): after idle, unplug LIS3DH power, then
   30 s each: SPI (`P0.02/03/04`), CS/control (`P0.11/12/28/29/30`), then
   `P0.23/24/25/31`; keep `D0` on `P0.22`. The group where current drops to
   µA is the leak.
3. Commit C2/C3/(C4…) note + packs; fix LIS3DH docs in a separate commit.
4. Later: run C1 (no generator, DCF timeout ~300 s → record ≥ 8 min),
   alarm/buzzer state, then run B (PPK2 ammeter in series in `3V3`).
5. Remaining OpenSpec tasks: see `tasks.md` (4.x bench runs, 5.1 flowchart,
   5.2 `openspec validate --strict`).

## Analysis recipe (scripts lived in /tmp, recreate)

Run from `firmware/tools/ppk-plot/` with
`PYTHONPATH=. .venv/bin/python script.py` (the `ppk-plot.sh` wrapper only
runs `ppk_plot.py`). Core loop:

```python
import numpy as np, ppkz
meta = ppkz.load_meta(p)
for t, v, b in ppkz.iter_chunks(p, meta):   # t ms, v µA, b digital byte
    ...  # bin by t//5000, D0 = b & 1
```

PPK2 app log (voltage-step and DUT ON/OFF timestamps):
`~/.config/nrfconnect/pc-nrfconnect-ppk/logs/2026-10-25T16_44_49.449Z-log.txt`.
Pack CSVs with `firmware/tools/ppk-plot/ppk-archive.sh pack <csv>`.

## Unrelated dirty files — do not stage

`hardware/kicad/.history`, `temp-resources/shit-to-buy.md`, `1.png`, `2.png`,
`AGENTS.md.baK`, `Latex.pdf`, `teams-dump*`, `temp-resources/display-flipped.jpeg`.

## Suggested skills

- `openspec-apply-change` — continue `add-deep-sleep-and-current-measurement`.
- `context-mode` — large capture / log analysis.
- `caveman-commit` — thesis-readable commit messages per run.
