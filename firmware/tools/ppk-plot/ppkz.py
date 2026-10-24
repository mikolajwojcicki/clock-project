"""PPK2 packed capture: `<stem>.ppkz` (zstd) plus `<stem>.ppkz.json` sidecar.

Body (before compression): all current samples, then all digital bytes
(column-major). Current is little-endian float32 (`<f4`) or float64 (`<f8`),
digital is one uint8 per sample with bit n = Dn. No timestamps are stored:
t = t0_ms + i * 1000 / sample_rate_hz, rounded to `timestamp_decimals`.
"""
import json

import numpy as np
import zstandard as zstd

FORMAT = "ppkz"
VERSION = 1
LEVEL = 19
WINDOW_LOG = 27  # 128 MiB window; decoder default limit is the same
CHUNK = 2_000_000  # rows per chunk; ppk_plot sums per chunk, keep it equal for CSV and ppkz
TS_DECIMALS = 2
CUR_DECIMALS = 3


def load_meta(path):
    with open(f"{path}.json") as f:
        meta = json.load(f)
    if meta.get("format") != FORMAT or meta.get("format_version") != VERSION:
        raise ValueError(f"{path}.json: not a {FORMAT} v{VERSION} sidecar")
    return meta


def timestamps(meta, i0, n):
    """Timestamps [ms] of samples i0..i0+n-1, snapped to the printed decimals."""
    scale = 10.0 ** meta["timestamp_decimals"]
    t = meta["t0_ms"] + np.arange(i0, i0 + n) * (1000.0 / meta["sample_rate_hz"])
    return np.rint(t * scale) / scale


def write_capture(path, n, cur_dtype, current_chunks, digital_chunks, level=LEVEL):
    """Write the zstd body. Chunks are arrays; digital_chunks may be None."""
    cur_dtype = np.dtype(cur_dtype).newbyteorder("<")
    total = n * cur_dtype.itemsize + (n if digital_chunks is not None else 0)
    params = zstd.ZstdCompressionParameters.from_level(
        level, window_log=WINDOW_LOG, enable_ldm=True, threads=-1)
    cctx = zstd.ZstdCompressor(compression_params=params)
    with open(path, "wb") as f, cctx.stream_writer(f, size=total) as w:
        for a in current_chunks:
            w.write(np.ascontiguousarray(a, dtype=cur_dtype))
        if digital_chunks is not None:
            for a in digital_chunks:
                w.write(np.ascontiguousarray(a, dtype=np.uint8))


def _read_exact(r, n):
    parts, got = [], 0
    while got < n:
        b = r.read(n - got)
        if not b:
            raise ValueError("capture body is truncated")
        parts.append(b)
        got += len(b)
    return b"".join(parts)


def iter_chunks(path, meta, sizes=None):
    """Yield (t_ms float64, current_uA float64, bits uint8 or None) chunks.

    Current is rounded to the stored decimals, so it equals the CSV value.
    `sizes` is an iterable of chunk lengths (default: CHUNK rows each).
    """
    n = meta["n_samples"]
    dtype = np.dtype(meta["current"]["dtype"])
    has_dig = meta["digital"] is not None
    if sizes is None:
        sizes = [min(CHUNK, n - i) for i in range(0, n, CHUNK)]
    cur_scale = 10.0 ** meta["current"]["decimals"]
    # one decompressor per stream: a ZstdDecompressor cannot serve two streams at once
    with open(path, "rb") as f_cur, open(path, "rb") as f_dig, \
            zstd.ZstdDecompressor(max_window_size=1 << WINDOW_LOG).stream_reader(f_cur) as cur, \
            zstd.ZstdDecompressor(max_window_size=1 << WINDOW_LOG).stream_reader(f_dig) as dig:
        if has_dig:  # second reader skips the current column
            skip = n * dtype.itemsize
            while skip:
                skip -= len(_read_exact(dig, min(skip, 1 << 24)))
        i0 = 0
        for m in sizes:
            v = np.frombuffer(_read_exact(cur, m * dtype.itemsize), dtype=dtype).astype(np.float64)
            if dtype.itemsize == 4:
                v = np.rint(v * cur_scale) / cur_scale
            bits = np.frombuffer(_read_exact(dig, m), dtype=np.uint8) if has_dig else None
            yield timestamps(meta, i0, m), v, bits
            i0 += m
