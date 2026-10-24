# ppk-plot / ppk-archive

Tools for Nordic PPK2 Data Logger exports (`Timestamp(ms),Current(uA)` with an optional `D0-D7` column, 100 kS/s).
All commands run from the repo root. The wrappers build a venv in `.venv/` inside `nix-shell` on first use
and reinstall when `requirements.txt` changes.

## ppk-plot

```sh
firmware/tools/ppk-plot/ppk-plot.sh notes/measurements/ppk-20261024T143919.csv --mean
```

Min/max-per-bin scatter plot of a CSV export, streamed in chunks. Options: `--from/--to` (s), `--max-ua`,
`--decimate N`, `--mean`, `--log`, `-b BINS`. Run with `--help` for all.

## ppk-archive: keep captures in git

A CSV export is about 150 MB per minute. `pack` turns it into a zstd file of about 4 MB per minute that
holds every sample, so the evidence can be committed. GitHub warns above 50 MB and rejects above 100 MB.

```sh
# CSV -> notes/measurements/ppk-20261024T143919.ppkz + .ppkz.json (about 40 s for 96 s of capture)
firmware/tools/ppk-plot/ppk-archive.sh pack notes/measurements/ppk-20261024T143919.csv

# a window of a long capture (seconds from the start of the export)
firmware/tools/ppk-plot/ppk-archive.sh pack notes/measurements/ppk-20261024T132153.csv --from 0 --to 200 -o /tmp/part.ppkz

# .ppkz -> CSV (refuses to overwrite an existing file unless --force)
firmware/tools/ppk-plot/ppk-archive.sh unpack notes/measurements/ppk-20261024T143919.ppkz -o /tmp/ppk-check.csv
```

`pack` options: `--from S`, `--to S`, `-o OUT`, `--level N` (zstd, default 19).

**Verification.** After writing, `pack` reads the `.ppkz` back and compares every sample with the CSV:
timestamp, current at three decimals, and each digital bit. It exits 0 only if all match, names the first
differing row otherwise, and leaves no output behind. The CSV is opened read-only and never changed.
`pack` also refuses a CSV with an unknown header, a missing or repeated timestamp, or a malformed `D0-D7`
value, and prints the data row and file line of the first problem. With `--to`, rows after the window end
are not read, so irregularities after it are not checked.

**50 MB rule.** A `.ppkz` committed to git must be under 50 MB. `pack` prints a warning with the number of
seconds that would fit if the output is 50 MB or more; pack the capture again as shorter windows.

**CSVs stay local.** `notes/measurements/ppk-*.csv` is git-ignored. The `.ppkz` and `.ppkz.json` are the
tracked evidence; `unpack` gives the CSV back (byte for byte on the 2026-10-24 captures).
The sidecar holds the SHA-256 of the original CSV.

### Format

`<stem>.ppkz.json` (sidecar, text) describes `<stem>.ppkz`:

| Field | Meaning |
| --- | --- |
| `format`, `format_version` | `"ppkz"`, `1` |
| `sample_rate_hz` | samples per second |
| `t0_ms` | timestamp of sample 0 in the source CSV |
| `timestamp_decimals` | decimals of the CSV timestamps (2) |
| `n_samples` | rows stored |
| `current` | `dtype` `<f4` (float32) or `<f8`, `unit` `uA`, `decimals` 3 |
| `digital` | `{channels: 8, bit_order: "bit n = Dn"}` or `null` |
| `source` | CSV `name`, `bytes`, `sha256`, `header` line |
| `window_s` | `[from, to]` given to `pack`, or `null` |
| `packed_bytes`, `tool` | size of the `.ppkz`; `ppk-pack <git describe>` |

`<stem>.ppkz` is one zstd frame (level 19, 128 MiB window) over the column-major body: `n_samples` current
values, then `n_samples` digital bytes (absent if `digital` is `null`). Decode without this tool:

```python
import json, numpy as np, zstandard
meta = json.load(open("x.ppkz.json"))
body = zstandard.ZstdDecompressor(max_window_size=1 << 27).stream_reader(open("x.ppkz", "rb")).read()
n = meta["n_samples"]
cur = np.frombuffer(body, dtype=meta["current"]["dtype"], count=n)               # uA
dig = np.frombuffer(body, dtype=np.uint8, count=n, offset=cur.nbytes) if meta["digital"] else None
t_ms = meta["t0_ms"] + np.arange(n) * 1000.0 / meta["sample_rate_hz"]
```

(`read()` loads the whole body; the tools stream it in 2 M-row chunks.) Current is float32 as in the PPK2
app; if any value would not read back at three decimals the whole file switches to `<f8`.

## Tests

```sh
PPK_SCRIPT=test_ppk_archive.py firmware/tools/ppk-plot/ppk-plot.sh
```
