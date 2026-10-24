# Tasks

## 1. Format and pack/unpack tool

- [ ] 1.1 Add `zstandard` (pinned) to `firmware/tools/ppk-plot/requirements.txt` and add `ppk-archive.sh` that reuses the `ppk-plot.sh` nix-shell/venv logic and runs `ppk_archive.py`. Verify: `firmware/tools/ppk-plot/ppk-archive.sh --help` prints usage and the venv reinstalls once after the requirements change.
- [ ] 1.2 Write the shared format module (sidecar schema, column-major body, `<f4` current with `<f8` fallback, uint8 digital with bit n = Dn, streaming reader yielding `(t_ms, current, bits)` chunks) as in design.md "Format", "Sample types" and "Sidecar fields". Verify: a unittest in `firmware/tools/ppk-plot/test_ppk_archive.py` (stdlib `unittest`, run through the wrapper's venv) writes and reads back a small synthetic capture with and without digital channels.
- [ ] 1.3 Implement `pack <csv> [--from S] [--to S] [-o OUT] [--level N]`: header check for the two known layouts, streamed chunks, uniform-timing check with the row of the first irregular step, read-back verification of every row (`%.3f` text and bits), float64 fallback, removal of partial outputs on failure, 50 MB warning with the duration that would fit. The source CSV is opened read-only. Verify: unittests cover both layouts, a gap, a repeated timestamp, an unknown header, a value that needs float64, a `--from/--to` window, and that a forced mismatch leaves no output files.
- [ ] 1.4 Implement `unpack <ppkz> [-o CSV]` writing the source header, timestamps formatted like the PPK2 app (`0`, `0.01`, `699.41`), current with three decimals and the `D0-D7` string. Verify: a unittest round trip compares the unpacked text with the synthetic source CSV line for line, and on the real 96 s file `cmp` of the current and digital columns (`cut -d, -f2-`) between source and unpacked CSV reports no difference.
- [ ] 1.5 Document pack, unpack and the format in a `README.md` in `firmware/tools/ppk-plot/` (commands, sidecar fields, the 50 MB rule, that CSVs stay local). Verify: the commands in the README run as written on `notes/measurements/ppk-20261024T143919.csv`.
- [ ] 1.6 Commit the tool (`firmware:` prefix) after 1.1-1.5 pass.

## 2. ppk-plot reads packed captures

- [ ] 2.1 Make `ppk_plot.py` choose the loader by extension (`.ppkz` or CSV) and stream the packed file through the same binning, `--mean`, `--from/--to`, `--max-ua` and `--decimate` path. Verify: on `ppk-20261024T143919.csv` and its `.ppkz`, `ppk-plot.sh --mean` prints the same sample count, min, max and mean, and `--from 100 --to 103` prints the same window summary line for both.
- [ ] 2.2 Update the `ppk-plot` README section and `--help` text for the new input. Verify: `ppk-plot.sh --help` names both inputs.
- [ ] 2.3 Commit (`firmware:` prefix).

## 3. Pack and track today's captures

- [ ] 3.1 Add `notes/measurements/ppk-*.csv` to `.gitignore`. Verify: `git status --short notes/measurements` no longer lists the four CSVs.
- [ ] 3.2 Pack `ppk-20261024T114909.csv`, `ppk-20261024T132153.csv`, `ppk-20261024T142752.csv` and `ppk-20261024T143919.csv`. Verify: each pack exits 0, each `.ppkz` is under 50 MB, and `ppk-plot.sh --mean` on each `.ppkz` gives the same mean as the CSV (the note's numbers: 3.789 µA for 114909, 3306.370 µA for 132153).
- [ ] 3.3 In `notes/log/2026-10-24_deep-sleep-current.md`, change each "Raw CSV: ... (kept local)" line to link the `.ppkz` and name the CSV it came from. Verify: every run section (A, A2, A3, A4) links an existing `.ppkz`.
- [ ] 3.4 Commit `.gitignore`, the `.ppkz`/`.ppkz.json` files and the note together (`docs:` prefix). Verify: `git show --stat HEAD` lists no file over 50 MB.

## 4. Evidence rules and the in-progress measurement change

- [ ] 4.1 Update the evidence-log line in `AGENTS.md` ("Measurements are CSV with ...") to allow a verified packed PPK2 capture, matching the delta in `specs/thesis-evidence-log/spec.md`. Verify: the line names `ppk-archive.sh pack` and the 50 MB rule.
- [ ] 4.2 In `openspec/changes/add-deep-sleep-and-current-measurement`, change "CSV" to "CSV or packed capture" in `specs/low-power-sleep-policy/spec.md` (requirement "Measurements are stored with conditions") and in tasks 4.2-4.5. Verify: `openspec validate add-deep-sleep-and-current-measurement --strict` passes.
- [ ] 4.3 Add an entry to `notes/ai-use.md` for the tool code (tool, output, how it was checked). Verify: the entry names `ppk_archive.py` and the round-trip checks from 1.3-1.4.
- [ ] 4.4 Run `openspec validate add-ppk2-capture-archive --strict` and commit 4.1-4.3 (`docs:` prefix). Verify: validation passes.

## Workflow follow-up

- Deleting the local CSVs is the user's decision after 3.2 passes.
- Archive this change after review; the archive merges the `ppk2-capture-archive` spec and the `thesis-evidence-log` delta into `openspec/specs/`.
