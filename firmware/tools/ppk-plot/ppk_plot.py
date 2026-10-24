#!/usr/bin/env python3
"""Scatter plot of a PPK2 CSV export (Timestamp(ms),Current(uA)).

Large files (30M+ rows) are streamed in chunks and reduced to one min and one
max point per time bin, so spikes survive decimation.
"""
import argparse
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("csv")
    ap.add_argument("-o", "--out", help="output image (default: <csv>.png)")
    ap.add_argument("-b", "--bins", type=int, default=5000, help="time bins (default 5000)")
    ap.add_argument("--log", action="store_true", help="log y axis")
    ap.add_argument("--show", action="store_true", help="open interactive window")
    a = ap.parse_args()

    # Load whole file (~0.5 GB for 30M rows), bin by time below.
    chunks = list(pd.read_csv(a.csv, chunksize=2_000_000, dtype=np.float64))
    t_max = chunks[-1].iloc[-1, 0]
    width = t_max / a.bins if t_max > 0 else 1.0
    lo = np.full(a.bins, np.inf)
    hi = np.full(a.bins, -np.inf)
    for c in chunks:
        t = c.iloc[:, 0].to_numpy()
        i = np.minimum((t / width).astype(np.int64), a.bins - 1)
        v = c.iloc[:, 1].to_numpy()
        np.minimum.at(lo, i, v)
        np.maximum.at(hi, i, v)
    centers = (np.arange(a.bins) + 0.5) * width / 1000.0  # s
    ok = np.isfinite(lo)
    print(f"{sum(len(c) for c in chunks)} samples, {t_max/1000:.1f} s, "
          f"min {lo[ok].min():.3f} uA, max {hi[ok].max():.3f} uA", file=sys.stderr)

    fig, ax = plt.subplots(figsize=(14, 6))
    ax.scatter(centers[ok], hi[ok], s=4, label="max per bin", color="tab:red")
    ax.scatter(centers[ok], lo[ok], s=4, label="min per bin", color="tab:blue")
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("Current [µA]")
    if a.log:
        ax.set_yscale("log")
    ax.grid(True, alpha=0.3)
    ax.legend()
    ax.set_title(a.csv.rsplit("/", 1)[-1])
    fig.tight_layout()
    out = a.out or a.csv.rsplit(".", 1)[0] + ".png"
    fig.savefig(out, dpi=150)
    print(out, file=sys.stderr)
    if a.show:
        plt.show()


if __name__ == "__main__":
    main()
