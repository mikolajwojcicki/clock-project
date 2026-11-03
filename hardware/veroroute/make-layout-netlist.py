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
# List index = hole position from the lowest DK x (USB at the left), which is also the
# VeroRoute block pin number. Checked against the Nordic PCA10040 v3.0.0 files: sheet 5
# (Connectors) numbers P1 1..8 = VIO VIO RESET VIO 5V GND GND VIN, P2 1..6 = A0..A5,
# P3 1..8 = D0..D7, P4 1..10 = D8..D13 GND AREF SDA SCL; pick-and-place rotates P1/P2
# 0 deg and P3/P4 180 deg, so Nordic pin 1 is at the low-x end on the bottom headers and
# at the high-x end on the top headers (hence the reversed P3/P4 lists). D2..D5 =
# P0.13..P0.16 per the DK User Guide. Comment shows the Arduino / GPIO name of each position.
HEADERS = {
    "P1": [None, None, None, 1, 16, 2, None, None],  # NC IOREF RESET 3V3 5V GND GND VIN
    "P2": [3, 4, 8, 9, 10, 14],  # A0 P0.03, A1 P0.04, A2 P0.28, A3 P0.29, A4 P0.30, A5 P0.31
    "P3": [None, None, 20, 19, 18, 17, 7, 6],  # D7 P0.18, D6 P0.17, D5..D2 P0.16..P0.13, D1 P0.12, D0 P0.11
    "P4": [None, None, 5, 15, 13, 12, 11, 21, None, None],  # SCL SDA AREF/P0.02 GND D13..D10 D9 D8
}
# Types are VeroRoute import strings (Part Aliases dialog, "Valid Import Strings"), so no aliases are needed.
# SIPx = single-row block of x pins, one hole each (BLOCK_100MILx draws a 3-hole-tall pill per pin; PADSx imports as loose pads).
HEADER_TYPE = {"P1": "SIP8", "P2": "SIP6", "P3": "SIP8", "P4": "SIP10"}

# Modules whose pin field is cut into SIP strips: ref -> {strip ref: schematic pin per position}.
# None = hole with no shield net.
SOCKET_ROWS = {
    # Adafruit LIS3DH #2809: two 6-pin rows, 6 holes (15.24 mm) apart, read with the
    # silkscreen upright (J3 pins: 1 VIN 2 3Vo 3 GND 4 SCL 5 SDA 6 SDO 7 CS 8 INT1 9 INT2).
    "J3": {
        "J3A": [1, 2, 3, 4, 5, 8],  # VIN 3Vo GND SCL SDA INT
        "J3B": [9, None, None, None, 6, 7],  # I2 A3 A2 A1 SDO CS
    },
    # Waveshare 2.13" HAT Rev2.1 plugs onto a 2x20 male header (RPi layout), rows 1 hole
    # apart: J2A = odd RPi pins 1..39, J2B = even pins 2..40, position i = RPi pin 2i-1 / 2i.
    # Source: hardware/docs/waveshare-hat-40pin.md (Waveshare wiki + HAT schematic).
    # The HAT power input is RPi pin 2/4 (`RPI_5V` net, same as cable VCC); we feed 3V3 there.
    # Pins 1/17 (P_3V3) power nothing on the HAT; 12, 27, 28 and the rest stay open.
    # (J2 pins: 1 VCC 2 GND 3 DIN 4 CLK 5 CS 6 DC 7 RST 8 BUSY.)
    "J2": {
        "J2A": [None] * 5 + [7] + [None] * 3 + [3] + [None, 4] + [None] * 8,  # RST 11, DIN 19, CLK 23
        "J2B": [1, None, 2] + [None] * 5 + [8, None, 6, 5] + [None] * 8,  # VCC 2, GND 6, BUSY 18, DC 22, CS 24
    },
}

# ref -> Type for every other part kept in the layout.
TYPES = {
    "J4": "SIP4", "J5": "SIP2",
    "R_SCK": "RESISTOR4", "R1": "RESISTOR4",
    "C1": "CAP_CERAMIC", "C3": "CAP_CERAMIC", "C2": "CAP_ELECTRO_200",
    "Q1": "TO92", "D1": "DIODE",
    "BZ1": "SIP4",  # AP-1205V-P1 pins are 8.21 mm apart (measured): 3 holes = 7.62 mm, pins splayed 0.3 mm each; pins 1 and 4 used
    "SW1": "SWITCH_BUTTON_4PIN", "SW2": "SWITCH_BUTTON_4PIN", "SW3": "SWITCH_BUTTON_4PIN",
    "SW4": "SWITCH_BUTTON_4PIN",  # 2 of 4 pins used; check the internally shorted pairs in VeroRoute
    "SW5": "SIP3",  # MSS-2245 one pole as a 3-pin row; SWITCH_ST/DT fail import ("Part has no pins"). Second pole row stays unused: keep its holes free (row pitch to measure).
}
DROP = {"R2", "R3"}  # DNP
# Layout pin number for parts whose footprint skips holes: ref -> {schematic pin: block pin}.
PIN_REMAP = {"BZ1": {1: 1, 2: 4}}

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


def split(pins_of, layout):
    """Net per block pin for a connector cut into strips: layout[i] is the schematic pin at block pin i+1."""
    nets = {i + 1: clean(pins_of[j]) for i, j in enumerate(layout) if j is not None}
    return {n: net for n, net in nets.items() if not net.startswith("unconnected-")}


def build(parts):
    out = []
    j1 = parts["J1"][1]
    for hdr, pins in HEADERS.items():
        out.append((hdr, HEADER_TYPE[hdr], f"DK_{hdr}", split(j1, pins)))
    for ref, strips in SOCKET_ROWS.items():
        for sub, pins in strips.items():
            out.append((sub, f"SIP{len(pins)}", f"{parts[ref][0]}_{sub}", split(parts[ref][1], pins)))
    for ref, (value, pins) in parts.items():
        if ref == "J1" or ref in SOCKET_ROWS or ref in DROP:
            continue
        remap = PIN_REMAP.get(ref, {})
        nets = {remap.get(n, n): clean(net) for n, net in pins.items() if not net.startswith("unconnected-")}
        out.append((ref, TYPES[ref], value, nets))
    return out


def check(parts, built):
    j1 = parts["J1"][1]
    used = [j for pins in HEADERS.values() for j in pins if j is not None]
    assert sorted(used) == sorted(j1), f"J1 pins not covered exactly once: {sorted(set(j1) ^ set(used))}"
    for ref, strips in SOCKET_ROWS.items():
        got = [j for pins in strips.values() for j in pins if j is not None]
        assert sorted(got) == sorted(parts[ref][1]), f"{ref} pins not covered exactly once"
    assert set(parts) - {"J1"} - set(SOCKET_ROWS) - DROP == set(TYPES), "TYPES does not match parts in the netlist"
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
