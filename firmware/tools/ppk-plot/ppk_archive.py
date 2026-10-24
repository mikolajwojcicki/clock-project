#!/usr/bin/env python3
"""Pack a PPK2 CSV export into a small verified capture, or unpack it again.

pack:   <stem>.csv -> <stem>.ppkz + <stem>.ppkz.json (zstd, checked sample by sample)
unpack: <stem>.ppkz -> <stem>.csv with the same values as the source CSV

Format details: README.md. Run through ppk-archive.sh.
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile

import numpy as np
import pandas as pd

import ppkz

KNOWN_HEADERS = {  # header line -> has digital column
    "Timestamp(ms),Current(uA)": False,
    "Timestamp(ms),Current(uA),D0-D7": True,
}
LIMIT_BYTES = 50_000_000  # GitHub warns above 50 MB
STEP_TOL_MS = 1e-6


class PackError(Exception):
    pass


def _row_msg(row):  # row: 0-based data row
    return f"data row {row + 1} (line {row + 2})"


def read_header(path):
    with open(path, newline="") as f:
        line = f.readline()
    header = line.rstrip("\r\n")
    if header not in KNOWN_HEADERS or line[len(header):] != "\n":
        raise PackError(f"unknown CSV header {line!r}; known: {list(KNOWN_HEADERS)} with LF line ends")
    return header


def parse_digital(col, g0):
    """Strings like '10000000' (D0 first) -> uint8, bit n = Dn."""
    b = col.to_numpy().astype("S9").view(np.uint8).reshape(-1, 9)
    bad = (b[:, 8] != 0) | ((b[:, :8] != 48) & (b[:, :8] != 49)).any(axis=1)
    if bad.any():
        raise PackError(f"bad D0-D7 value at {_row_msg(g0 + int(np.argmax(bad)))}")
    return np.packbits(b[:, :8] - 48, axis=1, bitorder="little").ravel()


def csv_chunks(path, header):
    """Yield (g0, t_ms, current, bits or None); g0 = 0-based row of the chunk start."""
    cols = {"Timestamp(ms)": np.float64, "Current(uA)": np.float64}
    if KNOWN_HEADERS[header]:
        cols["D0-D7"] = str
    g0 = 0
    with pd.read_csv(path, header=0, chunksize=ppkz.CHUNK, dtype=cols) as reader:
        for c in reader:
            t = c.iloc[:, 0].to_numpy()
            v = c.iloc[:, 1].to_numpy()
            for name, a in (("timestamp", t), ("current", v)):
                if not np.isfinite(a).all():
                    raise PackError(f"non-numeric {name} at {_row_msg(g0 + int(np.argmin(np.isfinite(a))))}")
            bits = parse_digital(c.iloc[:, 2], g0) if KNOWN_HEADERS[header] else None
            yield g0, t, v, bits
            g0 += len(t)


def cents(x, scale):
    return np.rint(x * scale)


def current_matches(stored, csv, scale):
    return cents(stored, scale) == cents(csv, scale)


def check_timing(t, g0, prev, t_first, dt_ms, scale):
    ext = t if prev is None else np.concatenate(([prev], t))
    bad = np.abs(np.diff(ext) - dt_ms) > STEP_TOL_MS
    if bad.any():
        row = g0 + (1 if prev is None else 0) + int(np.argmax(bad))
        raise PackError(f"timestamps not evenly spaced at {_row_msg(row)}")
    on_grid = cents(t_first + (g0 + np.arange(len(t))) * dt_ms, scale) == cents(t, scale)
    if not on_grid.all():
        raise PackError(f"timestamp at {_row_msg(g0 + int(np.argmin(on_grid)))} is off the "
                        f"{10 ** -ppkz.TS_DECIMALS} ms grid")


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while b := f.read(1 << 24):
            h.update(b)
    return h.hexdigest()


def tool_id():
    here = os.path.dirname(os.path.abspath(__file__))
    try:
        r = subprocess.run(["git", "describe", "--always", "--dirty", "--abbrev=7"],
                           cwd=here, capture_output=True, text=True, check=True)
        return f"ppk-pack {r.stdout.strip()}"
    except (OSError, subprocess.CalledProcessError):
        return "ppk-pack unknown"


def _tmp_chunks(path, dtype, rows):
    itemsize = np.dtype(dtype).itemsize
    with open(path, "rb") as f:
        while rows:
            m = min(rows, ppkz.CHUNK)
            yield np.frombuffer(f.read(m * itemsize), dtype=dtype)
            rows -= m


def verify(part, meta, src, header, lo, hi, sizes):
    """Read the capture back and compare every sample with the CSV."""
    rd = ppkz.iter_chunks(part, meta, sizes)
    tscale, cscale = 10.0 ** meta["timestamp_decimals"], 10.0 ** meta["current"]["decimals"]
    done = 0
    for g0, t, v, bits in csv_chunks(src, header):
        m = (t >= lo) & (t <= hi)
        if m.any():
            idx = np.flatnonzero(m)
            tr, vr, br = next(rd)
            if len(tr) != len(idx):
                raise PackError("verify: chunk size mismatch")
            diff = (cents(tr, tscale) != cents(t[m], tscale)) | ~current_matches(vr, v[m], cscale)
            if bits is not None:
                diff |= br != bits[m]
            if diff.any():
                k = int(np.argmax(diff))
                raise PackError(f"verify: capture differs from CSV at {_row_msg(g0 + int(idx[k]))}: "
                                f"read back t={tr[k]}, current={vr[k]:.3f}"
                                f"{'' if br is None else f', bits={br[k]:08b}'}")
            done += len(idx)
        if t[-1] > hi:
            break
    if done != meta["n_samples"] or next(rd, None) is not None:
        raise PackError("verify: sample count differs from CSV")


def pack(src, t_from, t_to, out, level):
    header = read_header(src)
    has_dig = KNOWN_HEADERS[header]
    out = out or os.path.splitext(src)[0] + ".ppkz"
    tscale, cscale = 10.0 ** ppkz.TS_DECIMALS, 10.0 ** ppkz.CUR_DECIMALS
    # window bounds snapped to the printed timestamp decimals, so a bound on a sample is inclusive
    lo = -np.inf if t_from is None else np.rint(t_from * 1000.0 * tscale) / tscale
    hi = np.inf if t_to is None else np.rint(t_to * 1000.0 * tscale) / tscale
    if hi < lo:
        raise PackError("--to must not be before --from")
    part, part_json = out + ".partial", out + ".json.partial"
    tmp = tempfile.mkdtemp(prefix=".ppk-pack-", dir=os.path.dirname(os.path.abspath(out)))
    cur_tmp, dig_tmp = os.path.join(tmp, "current.f8"), os.path.join(tmp, "digital.u8")
    try:
        n, need64, t_first, dt_ms, rate, t_win0, prev, sizes = 0, False, None, None, None, None, None, []
        with open(cur_tmp, "wb") as fc, open(dig_tmp, "wb") as fd:
            for g0, t, v, bits in csv_chunks(src, header):
                if t_first is None:
                    if len(t) < 2:
                        raise PackError("need at least two data rows")
                    t_first = t[0]
                    if t[1] <= t[0]:
                        raise PackError(f"timestamps not increasing at {_row_msg(1)}")
                    rate = round(1000.0 / (t[1] - t[0]), 6)
                    dt_ms = 1000.0 / rate
                check_timing(t, g0, prev, t_first, dt_ms, tscale)
                prev = t[-1]
                m = (t >= lo) & (t <= hi)
                if m.any():
                    vm = v[m]
                    if t_win0 is None:
                        t_win0 = t[m][0]
                    if not need64:
                        need64 = not current_matches(vm.astype("<f4").astype(np.float64), vm, cscale).all()
                    fc.write(vm.tobytes())
                    if has_dig:
                        fd.write(bits[m].tobytes())
                    n += len(vm)
                    sizes.append(len(vm))
                if t[-1] > hi:
                    break
        if n == 0:
            raise PackError("no samples in the requested window")
        cur_dtype = "<f8" if need64 else "<f4"
        ppkz.write_capture(
            part, n, cur_dtype,
            (a.astype(cur_dtype) for a in _tmp_chunks(cur_tmp, "<f8", n)),
            _tmp_chunks(dig_tmp, np.uint8, n) if has_dig else None, level)
        meta = {
            "format": ppkz.FORMAT, "format_version": ppkz.VERSION,
            "sample_rate_hz": rate, "t0_ms": float(t_win0),
            "timestamp_decimals": ppkz.TS_DECIMALS, "n_samples": n,
            "current": {"dtype": cur_dtype, "unit": "uA", "decimals": ppkz.CUR_DECIMALS},
            "digital": {"channels": 8, "bit_order": "bit n = Dn"} if has_dig else None,
            "source": {"name": os.path.basename(src), "bytes": os.path.getsize(src),
                       "sha256": sha256(src), "header": header},
            "window_s": None if t_from is None and t_to is None else [t_from, t_to],
            "packed_bytes": os.path.getsize(part), "tool": tool_id(),
        }
        verify(part, meta, src, header, lo, hi, sizes)
        with open(part_json, "w") as f:
            json.dump(meta, f, indent=2)
            f.write("\n")
        os.replace(part, out)
        os.replace(part_json, out + ".json")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)
        for p in (part, part_json):
            if os.path.exists(p):
                os.remove(p)
    dur = n / rate
    print(f"{out}: {n} samples, {dur:.2f} s, "
          f"{'f8' if need64 else 'f4'} current, digital {'yes' if has_dig else 'no'}; "
          f"source {meta['source']['bytes'] / 1e6:.1f} MB -> packed {meta['packed_bytes'] / 1e6:.1f} MB; verified")
    if meta["packed_bytes"] >= LIMIT_BYTES:
        fit = dur * LIMIT_BYTES / meta["packed_bytes"]
        print(f"WARNING: {meta['packed_bytes'] / 1e6:.1f} MB is not under 50 MB. About {fit:.0f} s would "
              f"fit; do not commit, re-pack windows with --from/--to.", file=sys.stderr)


def fmt_ts(c, dec):
    whole, frac = divmod(c, 10 ** dec)
    return str(whole) if frac == 0 else f"{whole}.{frac:0{dec}d}".rstrip("0")


def unpack(src, out, force):
    meta = ppkz.load_meta(src)
    out = out or os.path.splitext(src)[0] + ".csv"
    if os.path.exists(out) and not force:
        raise PackError(f"{out} exists (it may be the original export); use another -o or --force")
    part = out + ".partial"
    dec, scale = meta["timestamp_decimals"], 10.0 ** meta["timestamp_decimals"]
    try:
        with open(part, "w", newline="") as f:
            f.write(meta["source"]["header"] + "\n")
            for t, v, bits in ppkz.iter_chunks(src, meta):
                ts = [fmt_ts(c, dec) for c in np.rint(t * scale).astype(np.int64).tolist()]
                if bits is None:
                    rows = (f"{a},{c:.3f}" for a, c in zip(ts, v.tolist()))
                else:
                    d = (np.unpackbits(bits[:, None], axis=1, bitorder="little") + 48).astype(np.uint8)
                    d = np.ascontiguousarray(d).view("S8").ravel().astype("U8").tolist()
                    rows = (f"{a},{c:.3f},{b}" for a, c, b in zip(ts, v.tolist(), d))
                f.write("\n".join(rows) + "\n")
        os.replace(part, out)
    finally:
        if os.path.exists(part):
            os.remove(part)
    print(f"{out}: {meta['n_samples']} rows")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("pack", help="CSV -> .ppkz + .ppkz.json, verified")
    p.add_argument("csv")
    p.add_argument("--from", dest="t0", type=float, help="window start [s] (default 0)")
    p.add_argument("--to", dest="t1", type=float, help="window end [s] (default end of file)")
    p.add_argument("-o", "--out", help="output .ppkz (default: next to the CSV)")
    p.add_argument("--level", type=int, default=ppkz.LEVEL, help=f"zstd level (default {ppkz.LEVEL})")
    u = sub.add_parser("unpack", help=".ppkz -> CSV")
    u.add_argument("ppkz")
    u.add_argument("-o", "--out", help="output CSV (default: <stem>.csv next to the .ppkz)")
    u.add_argument("--force", action="store_true", help="overwrite an existing CSV")
    a = ap.parse_args()
    try:
        if a.cmd == "pack":
            pack(a.csv, a.t0, a.t1, a.out, a.level)
        else:
            unpack(a.ppkz, a.out, a.force)
    except (PackError, ValueError, OSError, pd.errors.ParserError) as e:
        sys.exit(f"error: {e}")


if __name__ == "__main__":
    main()
