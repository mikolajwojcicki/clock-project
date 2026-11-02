#!/usr/bin/env bash
# Render PlantUML sources to SVG (plantuml-server over HTTP) and PDF (rsvg-convert).
# Usage: ./render.sh [file.puml ...]   (default: every *.puml in this directory)
# Needs: plantuml-server on $PLANTUML_URL, curl and rsvg-convert on PATH
#   (nix-shell -p librsvg curl --run ./render.sh). See README.md.
set -euo pipefail
cd "$(dirname "$0")"
url="${PLANTUML_URL:-http://127.0.0.1:18080}"
(($#)) || set -- *.puml
for src in "$@"; do
  base="${src%.puml}"
  # -f: the server answers PlantUML syntax errors with HTTP 400, so curl fails.
  curl -sS -f -X POST -H 'Content-Type: text/plain; charset=utf-8' \
    --data-binary @"$src" "$url/svg" -o "$base.svg"
  rsvg-convert -f pdf -o "$base.pdf" "$base.svg"
  echo "$src -> $base.svg $base.pdf"
done
