# Design

## Context

`firmware/tools/ppk-plot/` holds `ppk_plot.py` and `ppk-plot.sh`. The wrapper
builds a venv inside `nix-shell` (manylinux wheels need `libstdc++`/`zlib`) and
reinstalls when `requirements.txt` changes. The venv Python is 3.13, which has
no standard-library zstd, and pandas, numpy and matplotlib are already in it.
`zstd` 1.5 is on the system path.

PPK2 Data Logger exports seen so far (headers checked 2026-10-24):

| File | Header | Size | Duration |
| --- | --- | --- | --- |
| `ppk-20261024T114909.csv` | `Timestamp(ms),Current(uA)` | 470 MB | 300 s |
| `ppk-20261024T132153.csv` | `...,D0-D7` | 1.10 GB | 400 s |
| `ppk-20261024T142752.csv` | `...,D0-D7` | 574 MB | 210 s |
| `ppk-20261024T143919.csv` | `...,D0-D7` | 242 MB | 96 s |

Rows are `0.01` ms apart at 100 kS/s, current has three decimals, and `D0-D7`
is an 8-character string with `D0` first (`10000000` = `D0` high).
`2026-10-21-21-41-screen-measurement.csv` is a Saleae export and is out of
scope.

Measured on the 96 s file: float32 current + uint8 bits, zstd -19
`--long=27`: 6.2 MB. zstd -19 on the CSV text: 21.3 MB. The app's own
`.ppk2` (deflate, 6 bytes/sample): about 17 MB by scaling the 54 MB 315 s file.
A float32 round trip gave the CSV text exactly for the first 3 M rows,
including a 52.9 mA inrush.

## Goals / Non-Goals

**Goals:**
- One command per direction (pack, unpack), usable by a second agent or the
  supervisor without reading the code.
- Exact agreement with the CSV, proven by the tool on every pack.
- Memory use bounded for the 1.1 GB export on the laptop (streamed, not one
  `read_csv` of the whole file).

**Non-Goals:**
- Reading the app's `.ppk2` files.
- Git LFS, history rewrite, or moving `ppk2-20261021T184647.ppk2`.
- Saleae `.sal` or Saleae CSV exports.
- Storing conditions (temperature, supply, commit) in the sidecar: they stay in
  the dated log note as today.

## Decisions

**Format: two files, `<stem>.ppkz` and `<stem>.ppkz.json`.**
`.ppkz` is one zstd frame (level 19, long-distance matching with a 128 MiB
window) over a column-major body: all current samples, then all digital bytes.
Column-major keeps the slowly varying current and the mostly constant bits in
separate runs, which is where the 39x comes from. The `<stem>` is the CSV's
stem, so `ppk-20261024T143919.csv` packs to `ppk-20261024T143919.ppkz`.
Alternatives: `.csv.zst` (3.4x larger, but readable with `zstdcat`); Parquet
(needs pyarrow in the venv, about the same size, harder to read without
Python); HDF5 (heavier dependency). The sidecar keeps the format self-describing
for a reader with only numpy and zstd.

**Sample types.** Current is little-endian float32 (`<f4`), the type the PPK2
app stores internally. If any sample fails the round trip at three decimals
(possible above roughly 65 mA, where float32 steps exceed 0.001), the pack
switches that file to float64 (`<f8`) and records it in the sidecar instead of
failing. Digital is one `uint8` per sample, bit `n` = `Dn`; absent when the CSV
has no `D0-D7` column.

**Sidecar fields.** `format: "ppkz"`, `format_version: 1`, `sample_rate_hz`,
`t0_ms` (timestamp of the first packed row in the source),
`n_samples`, `current: {dtype, unit: "uA", decimals: 3}`,
`digital: {channels: 8, bit_order: "bit n = Dn"}` or `null`,
`source: {name, bytes, sha256, header}`, `window_s: [from, to]` or `null`,
`packed_bytes`, `tool: "ppk-pack <git short hash>"`.

**Timing.** The sample rate comes from the first two timestamps; every step
must equal it to within 1e-6 ms or the pack fails with the row number. The PPK2
exports observed print timestamps with up to two decimals, so unpack writes
`t0_ms + i / rate` formatted with up to two decimals and trailing zeros
dropped, as the app does (`0`, `0.01`, `699.41`).

**Streaming.** CSV is read in chunks (2 M rows) with the digital column as a
string; current and bits go to temporary column files on disk, which are then
concatenated into one zstd stream. Verification reads the `.ppkz` back
(streamed decompression) and re-reads the CSV in the same chunks, comparing
`%.3f` of the stored current with the CSV text and the bits byte for byte. On
any mismatch or error the partial outputs are removed. The CSV is opened
read-only.

**Commands.** New script `ppk_archive.py` with subcommands `pack <csv>
[--from S] [--to S] [-o OUT]` and `unpack <ppkz> [-o CSV]`, run through a
wrapper `ppk-archive.sh` that shares the venv logic of `ppk-plot.sh`.
`ppk_plot.py` picks its loader from the file extension; both loaders yield
the same `(t_ms, current)` chunks, so binning, `--mean`, `--from/--to`,
`--max-ua` and `--decimate` stay unchanged. A shared loader module avoids two
copies of the format code.

**Python zstd binding: `zstandard` from PyPI, pinned in `requirements.txt`.**
It installs as a wheel into the existing venv; no system change. Alternative:
pipe through the `zstd` CLI (no new dependency, but harder to stream and to
report errors).

**Git.** `.gitignore` gets `notes/measurements/ppk-*.csv`. `.ppkz` and
`.ppkz.json` are tracked normally. A pack of 50 MB or more prints a warning
with the duration that would fit (pro rata), and the user re-packs with
`--from/--to`.

## Risks / Trade-offs

- [The CSV is deleted locally and the pack had a bug that the check missed] →
  the check compares the printed CSV text, not parsed floats; tasks keep the
  CSVs until the plot and `--mean` of each packed file agree with the CSV.
- [A later PPK app version changes the CSV layout] → pack checks the header
  against the two known layouts and refuses others with the header printed.
- [zstd -19 is slow: about 1.7 min for the 96 s file, roughly 20 min for the
  1.1 GB file] → acceptable once per capture; a `--level` option is allowed
  for testing, and the default stays 19.
- [Binary evidence is less inspectable than CSV for a reviewer] → unpack gives
  the CSV back, and the sidecar names the source SHA-256 so the regenerated
  values can be tied to the original export.
- [Repo grows by about 4 MB per captured minute] → fine for the planned runs
  (about 70 MB for today's four, about 30 MB for the next 8 min capture);
  revisit LFS if the measurements directory passes about 1 GB.

## Migration Plan

1. Land the tool (firmware commit), then pack today's four CSVs and commit
   the `.ppkz` files with the `.gitignore` rule and note links (docs commit).
2. Keep the CSVs locally until each packed file is checked with `ppk-plot
   --mean`; deleting them is the user's call.
3. Rollback: revert the commits; the CSVs are untouched by the tool.

## Open Questions

- Whether to also pack Saleae CSV exports later. Not needed now: the only one
  is 11 MB and already tracked.
