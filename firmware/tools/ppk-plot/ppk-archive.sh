#!/usr/bin/env bash
# Pack/unpack PPK2 captures: runs ppk_archive.py through the ppk-plot.sh venv wrapper.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PPK_SCRIPT=ppk_archive.py exec "$here/ppk-plot.sh" "$@"
