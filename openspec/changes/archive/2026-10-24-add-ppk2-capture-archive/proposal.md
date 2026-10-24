# Proposal

## Why

PPK2 Data Logger CSV exports at 100 kS/s run at about 150 MB per minute: the
four runs of 2026-10-24 total 2.4 GB and stay untracked, so the deep-sleep
evidence is not in git. GitHub warns above 50 MB and rejects files above
100 MB per file, and the next planned capture (8 min) would be about 1.2 GB as
CSV. A lossless binary form compressed with zstd measured 6.2 MB for the 96 s
run (241 MB CSV, 39 times smaller); plain `zstd -19` on the CSV gave 21.3 MB.

## What Changes

- New converter in `firmware/tools/ppk-plot/` that packs a PPK2 CSV export into
  a zstd-compressed binary capture plus a JSON sidecar, checks every sample
  against the CSV before reporting success, and can unpack back to a CSV with
  the same values.
- `ppk-plot` reads the packed capture directly, with the same options as for
  a CSV.
- PPK2 CSV exports under `notes/measurements/` become local-only (ignored);
  the packed capture and sidecar are what git tracks.
- The four 2026-10-24 captures are packed and committed; the run notes link
  the packed files.
- The evidence-log rule "measurements are raw CSV" is widened to "raw CSV or a
  lossless packed capture that unpacks to it".
- Git LFS is not introduced. The 54 MB `ppk2-20261021T184647.ppk2` already on
  `origin/main` stays as it is; no history rewrite.

## Capabilities

### New Capabilities

- `ppk2-capture-archive`: packed PPK2 capture format, the pack/verify/unpack
  tool, `ppk-plot` support, and the size limit for tracked captures.

### Modified Capabilities

- `thesis-evidence-log`: "Measurements with conditions" accepts a lossless
  packed capture as the raw measurement, not only CSV.

## Impact

- Code: `firmware/tools/ppk-plot/` (new script, `ppk_plot.py` input handling,
  `requirements.txt` gains `zstandard`; the venv wrapper reinstalls on change).
- Repo: `.gitignore` rule for PPK2 CSV exports; new tracked files under
  `notes/measurements/` (about 70 MB total for today's four runs).
- Docs: `notes/log/2026-10-24_deep-sleep-current.md` links, `AGENTS.md`
  evidence-log line, `notes/ai-use.md` entry. The in-progress change
  `add-deep-sleep-and-current-measurement` says "CSV" in its spec delta and
  tasks 4.2-4.5; its wording is aligned so both changes agree.
- No firmware, hardware or thesis LaTeX change. No new system package: `zstd`
  is already installed, and the Python binding lives in the tool's venv.
