"""Round-trip tests for ppkz / ppk_archive.

Run: PPK_SCRIPT=test_ppk_archive.py firmware/tools/ppk-plot/ppk-plot.sh
"""
import json
import os
import tempfile
import unittest
from unittest import mock

import numpy as np

import ppk_archive
import ppkz

H_DIG = "Timestamp(ms),Current(uA),D0-D7"
H_CUR = "Timestamp(ms),Current(uA)"


def ts_text(i):  # PPK2 style: 0, 0.01, 0.1, 699.41
    return ("%.2f" % (i * 0.01)).rstrip("0").rstrip(".")


def make_rows(n, header=H_DIG, currents=None):
    cur = currents or [round(0.3 + (i % 17) * 0.011, 3) for i in range(n)]
    rows = []
    for i in range(n):
        r = f"{ts_text(i)},{cur[i]:.3f}"
        if header == H_DIG:
            r += "," + "".join("1" if (i >> b) & 1 else "0" for b in range(8))
        rows.append(r)
    return rows


class Base(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.dir = self._tmp.name
        self.addCleanup(self._tmp.cleanup)
        p = mock.patch.object(ppkz, "CHUNK", 7)  # exercise chunk boundaries
        p.start()
        self.addCleanup(p.stop)

    def write_csv(self, rows, header=H_DIG, name="cap.csv"):
        path = os.path.join(self.dir, name)
        with open(path, "w", newline="") as f:
            f.write(header + "\n" + "\n".join(rows) + "\n")
        return path

    def pack(self, csv, t_from=None, t_to=None, out=None):
        ppk_archive.pack(csv, t_from, t_to, out, 3)
        return (out or csv[:-4] + ".ppkz")

    def lines(self, path):
        with open(path) as f:
            return f.read().splitlines()

    def only_source_left(self, *names):
        self.assertEqual(sorted(os.listdir(self.dir)), sorted(names))


class TestFormat(Base):
    def roundtrip(self, dig):
        n = 20
        cur = np.arange(n, dtype="<f4") * 0.5
        bits = np.arange(n, dtype=np.uint8)
        path = os.path.join(self.dir, "x.ppkz")
        ppkz.write_capture(path, n, "<f4", [cur[:11], cur[11:]], [bits] if dig else None, 3)
        meta = {"format": "ppkz", "format_version": 1, "sample_rate_hz": 100000.0, "t0_ms": 5.0,
                "timestamp_decimals": 2, "n_samples": n,
                "current": {"dtype": "<f4", "unit": "uA", "decimals": 3},
                "digital": {"channels": 8} if dig else None}
        got = list(ppkz.iter_chunks(path, meta))
        self.assertEqual([len(c[0]) for c in got], [7, 7, 6])
        t = np.concatenate([c[0] for c in got])
        np.testing.assert_allclose(t, 5.0 + np.arange(n) * 0.01)
        np.testing.assert_array_equal(np.concatenate([c[1] for c in got]), cur.astype(float))
        if dig:
            np.testing.assert_array_equal(np.concatenate([c[2] for c in got]), bits)
        else:
            self.assertTrue(all(c[2] is None for c in got))

    def test_with_digital(self):
        self.roundtrip(True)

    def test_without_digital(self):
        self.roundtrip(False)


class TestPack(Base):
    def check_roundtrip(self, header):
        rows = make_rows(25, header)
        csv = self.write_csv(rows, header)
        ppkz_path = self.pack(csv)
        meta = ppkz.load_meta(ppkz_path)
        self.assertEqual(meta["n_samples"], 25)
        self.assertEqual(meta["digital"] is not None, header == H_DIG)
        self.assertEqual(meta["current"]["dtype"], "<f4")
        self.assertEqual(meta["sample_rate_hz"], 100000.0)
        out = os.path.join(self.dir, "back.csv")
        ppk_archive.unpack(ppkz_path, out, False)
        self.assertEqual(self.lines(out), [header] + rows)

    def test_layout_with_digital(self):
        self.check_roundtrip(H_DIG)

    def test_layout_without_digital(self):
        self.check_roundtrip(H_CUR)

    def test_unpack_refuses_to_overwrite(self):
        csv = self.write_csv(make_rows(10))
        p = self.pack(csv)
        with self.assertRaises(ppk_archive.PackError):
            ppk_archive.unpack(p, csv, False)

    def fails(self, rows, header=H_DIG, msg=""):
        csv = self.write_csv(rows, header)
        with self.assertRaises(ppk_archive.PackError) as cm:
            self.pack(csv)
        self.assertIn(msg, str(cm.exception))
        self.only_source_left("cap.csv")

    def test_gap(self):
        rows = make_rows(25)
        del rows[12]
        self.fails(rows, msg="data row 13 (line 14)")

    def test_repeated_timestamp(self):
        rows = make_rows(25)
        rows[15] = rows[14]
        self.fails(rows, msg="data row 16 (line 17)")

    def test_unknown_header(self):
        self.fails(make_rows(5), header="Time,uA,D", msg="unknown CSV header")

    def test_bad_digital(self):
        rows = make_rows(25)
        rows[9] = rows[9][:-1] + "2"
        self.fails(rows, msg="bad D0-D7 value at data row 10")

    def test_float64_fallback(self):
        cur = [0.5] * 25
        cur[10] = 52900.124  # float32 step is 0.0039 here, 3 decimals do not round-trip
        rows = make_rows(25, currents=cur)
        csv = self.write_csv(rows)
        p = self.pack(csv)
        self.assertEqual(ppkz.load_meta(p)["current"]["dtype"], "<f8")
        out = os.path.join(self.dir, "back.csv")
        ppk_archive.unpack(p, out, False)
        self.assertEqual(self.lines(out), [H_DIG] + rows)

    def test_window(self):
        rows = make_rows(40)
        csv = self.write_csv(rows)
        p = self.pack(csv, t_from=0.00012, t_to=0.00027)  # rows 12..27 (t = 0.12..0.27 ms)
        meta = ppkz.load_meta(p)
        self.assertEqual((meta["n_samples"], meta["t0_ms"]), (16, 0.12))
        self.assertEqual(meta["window_s"], [0.00012, 0.00027])
        out = os.path.join(self.dir, "back.csv")
        ppk_archive.unpack(p, out, False)
        self.assertEqual(self.lines(out), [H_DIG] + rows[12:28])

    def test_forced_mismatch_leaves_nothing(self):
        csv = self.write_csv(make_rows(25))
        real = ppkz.write_capture

        def corrupt(path, n, dtype, cur, dig, level):
            def bad():
                for k, a in enumerate(cur):
                    a = np.array(a)
                    if k == 0:
                        a[3] += 0.01
                    yield a
            real(path, n, dtype, bad(), dig, level)

        with mock.patch.object(ppkz, "write_capture", corrupt):
            with self.assertRaises(ppk_archive.PackError) as cm:
                self.pack(csv)
        self.assertIn("data row 4 (line 5)", str(cm.exception))
        self.only_source_left("cap.csv")

    def test_sidecar_fields(self):
        csv = self.write_csv(make_rows(10))
        with open(self.pack(csv) + ".json") as f:
            meta = json.load(f)
        self.assertEqual(meta["source"]["header"], H_DIG)
        self.assertEqual(meta["source"]["bytes"], os.path.getsize(csv))
        self.assertEqual(len(meta["source"]["sha256"]), 64)


if __name__ == "__main__":
    unittest.main()
