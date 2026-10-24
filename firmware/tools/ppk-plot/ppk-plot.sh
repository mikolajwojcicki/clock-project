#!/usr/bin/env bash
# Run ppk_plot.py (or $PPK_SCRIPT, see ppk-archive.sh) inside a project venv
# (created on first use).
# NixOS: manylinux wheels need libstdc++/zlib, so the venv is built and used
# inside nix-shell unless already there.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
venv="$here/.venv"

if [[ -z "${IN_NIX_SHELL:-}" ]] && command -v nix-shell >/dev/null; then
    exec nix-shell -p python3 stdenv.cc.cc.lib zlib --run \
        "LD_LIBRARY_PATH=\$(echo \$NIX_LDFLAGS | tr ' ' '\n' | sed -n 's/^-L//p' | paste -sd:):\${LD_LIBRARY_PATH:-} $(printf '%q ' "$here/ppk-plot.sh" "$@")"
fi

[[ -d "$venv" ]] || python3 -m venv "$venv"
# shellcheck disable=SC1091
source "$venv/bin/activate"
if ! cmp -s "$here/requirements.txt" "$venv/.requirements.installed"; then
    pip install -q -r "$here/requirements.txt"
    cp "$here/requirements.txt" "$venv/.requirements.installed"
fi
exec python "$here/${PPK_SCRIPT:-ppk_plot.py}" "$@"
