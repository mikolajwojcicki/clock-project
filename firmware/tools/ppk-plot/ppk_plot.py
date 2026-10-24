#!/usr/bin/env python3
"""Scatter plot of a PPK2 capture: CSV export (Timestamp(ms),Current(uA)[,D0-D7])
or packed capture (.ppkz, see ppk-archive.sh).

Large files (30M+ rows) are streamed in chunks and reduced to one min and one
max point per time bin, so spikes survive decimation.
"""
import argparse
import sys

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator, MultipleLocator
import numpy as np
import pandas as pd

import ppkz


def load(path):
    """-> (chunks of (t_ms, current_uA) arrays, row count, last timestamp [ms])."""
    if path.endswith(".ppkz"):
        meta = ppkz.load_meta(path)
        t_last = ppkz.timestamps(meta, meta["n_samples"] - 1, 1)[0]
        return ((t, v) for t, v, _ in ppkz.iter_chunks(path, meta)), meta["n_samples"], t_last
    # Load whole file (~0.5 GB for 30M rows), the last timestamp is needed before binning.
    chunks = [(c.iloc[:, 0].to_numpy(), c.iloc[:, 1].to_numpy())
              for c in pd.read_csv(path, chunksize=ppkz.CHUNK, usecols=[0, 1], dtype=np.float64)]
    return chunks, sum(len(t) for t, _ in chunks), chunks[-1][0][-1]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("capture", help="PPK2 CSV export or .ppkz packed capture")
    ap.add_argument("-o", "--out", help="output image (default: <capture>.png)")
    ap.add_argument("-b", "--bins", type=int, default=5000, help="time bins (default 5000)")
    ap.add_argument("--log", action="store_true", help="log y axis")
    ap.add_argument("--show", action="store_true", help="open interactive window")
    ap.add_argument("--from", dest="t0", type=float, help="window start [s] (default 0)")
    ap.add_argument("--to", dest="t1", type=float, help="window end [s] (default end of file)")
    ap.add_argument("--max-ua", type=float, help="drop samples above this current [uA]")
    ap.add_argument("--decimate", type=int, default=1, metavar="N",
                    help="keep every Nth CSV row (plot only; default 1)")
    ap.add_argument("--mean", action="store_true",
                    help="print mean current of the entire file (all rows)")
    a = ap.parse_args()

    chunks, n_total, t_last = load(a.capture)
    t0 = (a.t0 if a.t0 is not None else 0.0) * 1000.0
    t1 = (a.t1 if a.t1 is not None else t_last / 1000.0) * 1000.0
    if t1 <= t0:
        sys.exit("--to must be greater than --from")
    width = (t1 - t0) / a.bins
    lo = np.full(a.bins, np.inf)
    hi = np.full(a.bins, -np.inf)
    n_used = 0
    v_sum = 0.0
    row = 0  # global row index of the chunk start, keeps decimation phase across chunks
    for t, v in chunks:
        v_sum += v.sum()  # mean covers every row, ignoring window/filter/decimate
        m = (t >= t0) & (t <= t1)
        if a.max_ua is not None:
            m &= v <= a.max_ua
        if a.decimate > 1:
            m &= (np.arange(row, row + len(t)) % a.decimate) == 0
        row += len(t)
        t, v = t[m], v[m]
        n_used += len(t)
        i = np.minimum(((t - t0) / width).astype(np.int64), a.bins - 1)
        np.minimum.at(lo, i, v)
        np.maximum.at(hi, i, v)
    ok = np.isfinite(lo)
    if not ok.any():
        sys.exit("no samples left after window/filter")
    centers = (t0 + (np.arange(a.bins) + 0.5) * width) / 1000.0  # s
    print(f"{n_used}/{n_total} samples, {t0/1000:.3f}-{t1/1000:.3f} s, "
          f"min {lo[ok].min():.3f} uA, max {hi[ok].max():.3f} uA", file=sys.stderr)
    if a.mean:
        print(f"mean {v_sum / n_total:.3f} uA over all {n_total} rows", file=sys.stderr)

    fig, ax = plt.subplots(figsize=(14, 6))
    ax.scatter(centers[ok], hi[ok], s=4, label="max per bin", color="tab:red")
    ax.scatter(centers[ok], lo[ok], s=4, label="min per bin", color="tab:blue")
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("Current [µA]")
    ax.set_xlim(t0 / 1000.0, t1 / 1000.0)
    if (t1 - t0) / 1000.0 >= 120:
        ax.xaxis.set_major_locator(MultipleLocator(60))  # 60 s majors
    ax.xaxis.set_minor_locator(AutoMinorLocator(6))  # 10 s minors on 60 s majors
    if a.log:
        ax.set_yscale("log")  # log axis has its own minor ticks
    else:
        ax.yaxis.set_minor_locator(AutoMinorLocator(5))
    ax.grid(True, which="major", alpha=0.5)
    ax.grid(True, which="minor", alpha=0.2, linewidth=0.5)
    ax.legend()
    ax.set_title(a.capture.rsplit("/", 1)[-1])
    fig.tight_layout()
    out = a.out or a.capture.rsplit(".", 1)[0] + ".png"
    fig.savefig(out, dpi=150)
    print(out, file=sys.stderr)
    if a.show:
        plt.show()


if __name__ == "__main__":
    main()
