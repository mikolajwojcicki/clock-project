#!/usr/bin/env python3
"""Derive the VeroRoute import netlist from the KiCad OrcadPCB2 export.

KiCad draws the DK as one symbol (J1) and leaves every footprint empty
($noname), which VeroRoute cannot import. This script reads
hardware/kicad/dk-perfboard-shield.net and writes
hardware/veroroute/dk-perfboard-shield-layout.net with:
  - J1 split into the four DK Arduino headers P1..P4 (pin order below),
  - a footprint (Type) per part so VeroRoute Part Aliases can be defined,
  - DNP parts (R2, R3) and `unconnected-*` pins dropped.
Nets are never edited by hand: every pin net comes from the KiCad netlist.

Usage: python3 make-layout-netlist.py   (run from any directory)
"""
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
SRC = HERE.parent / "kicad" / "dk-perfboard-shield.net"
DST = HERE / "dk-perfboard-shield-layout.net"

# DK header pin -> J1 pin number in the schematic symbol. None = no shield net.
# Pin order is the Arduino R3 order seen from the top of the DK, USB at the
# left (header pin 1 at the lowest x). [INFERENCE] from the Arduino standard
# and the Nordic GPIO table; verify against the DK silkscreen before soldering.
# Comment shows the Arduino / GPIO name of each position.
HEADERS = {
    "P1": [None, None, None, 1, 16, 2, None, None],  # NC IOREF RESET 3V3 5V GND GND VIN
    "P2": [3, 4, 8, 9, 10, 14],  # A0 P0.03, A1 P0.04, A2 P0.28, A3 P0.29, A4 P0.30, A5 P0.31
    "P3": [None, None, 20, 19, 18, 17, 7, 6],  # D7 P0.18, D6 P0.17, D5..D2 P0.16..P0.13, D1 P0.12, D0 P0.11
    "P4": [None, None, 5, 15, 13, 12, 11, 21, None, None],  # SCL SDA AREF/P0.02 GND D13..D10 D9 D8
}
HEADER_TYPE = {"P1": "HDR_F_1x8", "P2": "HDR_F_1x6", "P3": "HDR_F_1x8", "P4": "HDR_F_1x10"}

# ref -> Type for every other part kept in the layout.
TYPES = {
    "J2": "HDR_F_1x8", "J3": "HDR_F_1x9", "J4": "HDR_F_1x4", "J5": "HDR_M_1x2",
    "R_SCK": "RES_AXIAL", "R1": "RES_AXIAL",
    "C1": "CAP_CERAMIC", "C3": "CAP_CERAMIC", "C2": "CAP_ELECTRO",
    "Q1": "TO92", "D1": "DIODE_AXIAL", "BZ1": "BUZZER_2P",
    "SW1": "SW_TACT", "SW2": "SW_TACT", "SW3": "SW_TACT", "SW4": "SW_TACT",
    "SW5": "SW_SLIDE_3P",
}
DROP = {"R2", "R3"}  # DNP

BLOCK = re.compile(r"\( (\S+) (\S+)\s+(\S+) (\S+)\n(.*?)\n \)", re.S)
PIN = re.compile(r"\(\s*(\d+) (\S+) \)")


def parse(text):
    parts = {}
    for m in BLOCK.finditer(text):
        _, _, ref, value, body = m.groups()
        parts[ref] = (value, {int(n): net for n, net in PIN.findall(body)})
    return parts


def clean(net):
    # Parentheses in KiCad auto net names ("Net-(J1-P0.16_SW4)") break the Orcad parser.
    return net.replace("(", "").replace(")", "")


def build(parts):
    out = []
    j1 = parts["J1"][1]
    for hdr, pins in HEADERS.items():
        nets = {i + 1: clean(j1[j]) for i, j in enumerate(pins) if j is not None}
        out.append((hdr, HEADER_TYPE[hdr], f"DK_{hdr}", nets))
    for ref, (value, pins) in parts.items():
        if ref == "J1" or ref in DROP:
            continue
        nets = {n: clean(net) for n, net in pins.items() if not net.startswith("unconnected-")}
        out.append((ref, TYPES[ref], value, nets))
    return out


def check(parts, built):
    j1 = parts["J1"][1]
    used = [j for pins in HEADERS.values() for j in pins if j is not None]
    assert sorted(used) == sorted(j1), f"J1 pins not covered exactly once: {sorted(set(j1) ^ set(used))}"
    assert set(parts) - {"J1"} - DROP == set(TYPES), "TYPES does not match parts in the netlist"
    for ref, _t, _v, nets in built:
        for n, net in nets.items():
            assert net.startswith(("/", "Net-")) and "(" not in net, (ref, n, net)
    assert len({b[0] for b in built}) == len(built), "duplicate refs"


def main():
    parts = parse(SRC.read_text())
    built = build(parts)
    check(parts, built)
    lines = ["( { EESchema Netlist Version 1.1 derived by make-layout-netlist.py from dk-perfboard-shield.net }"]
    for ref, typ, value, nets in built:
        lines.append(f" ( /{ref} {typ} {ref} {value}")
        for n in sorted(nets):
            lines.append(f"  ( {n:>4} {nets[n]} )")
        lines.append(" )")
    lines.append(")")
    lines.append("*")
    DST.write_text("\n".join(lines) + "\n")
    print(f"wrote {DST} ({len(built)} parts)")


if __name__ == "__main__":
    sys.exit(main())
