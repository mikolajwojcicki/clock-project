#!/usr/bin/env bash
# Rebuild pakiet.pdf (consultation bundle) from the tracked sources, in show order.
# Run from the repo root:
#   nix-shell -p poppler-utils img2pdf --run notes/consultation/first-consultation/build-pakiet.sh
# Rerun after firmware/docs/diagrams/render.sh or a new PPK2 plot; page list in sciagawka.md.
set -euo pipefail
out=notes/consultation/first-consultation/pakiet.pdf
d=firmware/docs/diagrams
m=notes/media/2026-11-02_refresh-on-demand_ppk2
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

img2pdf "$m-full.png" -o "$tmp/ppk-full.pdf"
img2pdf "$m-refresh-436s.png" -o "$tmp/ppk-refresh.pdf"

pdfunite \
  hardware/docs/hardware-block-diagram.pdf \
  hardware/kicad/breadboard-prototype-for-promotor.pdf \
  $d/l1-overview.pdf \
  $d/l2-modules-{a,b,c,d,e}.pdf \
  $d/l3-detail-{a,b,c,d,e,f}.pdf \
  $d/ble-overview-{a,b}.pdf \
  "$tmp/ppk-full.pdf" "$tmp/ppk-refresh.pdf" \
  $d/ble-sequence-{a,b}.pdf \
  "$out"
pdfinfo "$out" | grep Pages
