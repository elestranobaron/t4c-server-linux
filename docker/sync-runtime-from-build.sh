#!/bin/bash
# Copie les assets strategiques depuis un repertoire build/ local vers docker/runtime/.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="${1:-$ROOT/build}"
DST="$ROOT/docker/runtime"

if [ ! -d "$SRC" ]; then
    echo "Source introuvable: $SRC" >&2
    echo "Usage: $0 [/chemin/vers/build]" >&2
    exit 1
fi

mkdir -p "$DST/WDA"

for wda in "T4C Worlds.WDA" "T4C Edit.WDA" "NPCs.WDA"; do
    if [ -f "$SRC/WDA/$wda" ]; then
        cp -a "$SRC/WDA/$wda" "$DST/WDA/"
        echo "[sync] WDA/$wda"
    else
        echo "[sync] WARN: absent — $SRC/WDA/$wda" >&2
    fi
done

for elng in "$SRC"/*.elng; do
    [ -e "$elng" ] || continue
    cp -a "$elng" "$DST/"
    echo "[sync] $(basename "$elng")"
done

if [ -f "$SRC/T4CServer.ini" ] && [ "${T4C_SYNC_INI:-0}" = "1" ]; then
    cp -a "$SRC/T4CServer.ini" "$DST/T4CServer.ini"
    echo "[sync] T4CServer.ini (T4C_SYNC_INI=1)"
fi

echo "[sync] OK -> $DST"
echo "Verifier MD5 Worlds/Edit (LP64) puis: docker compose build"
