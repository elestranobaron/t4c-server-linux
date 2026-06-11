#!/bin/bash
# Compile T4CServer dans build/, puis synchronise res/ → build/ (données runtime).
#
# IMPORTANT : ce script ne supprime JAMAIS build/ par défaut.
# Tes WDA, T4CServer.ini, logs et binaires locaux restent en place.
# Nettoyage complet uniquement avec : ./build.sh --clean
#   (crée d'abord build_backup_YYYYMMDD_HHMMSS à la racine du projet)
set -euo pipefail
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
RES_DIR="$PROJECT_DIR/runtime"
BUILD_DIR="$PROJECT_DIR/build"

if [[ "${1:-}" == "--clean" ]]; then
    if [[ -d "$BUILD_DIR" ]] && [[ -n "$(ls -A "$BUILD_DIR" 2>/dev/null || true)" ]]; then
        STAMP="$(date +%Y%m%d_%H%M%S)"
        BACKUP="$PROJECT_DIR/build_backup_${STAMP}"
        echo "[build.sh] Sauvegarde build/ → $BACKUP"
        cp -a "$BUILD_DIR" "$BACKUP"
    fi
    echo "[build.sh] --clean : suppression de build/ (sauvegarde ci-dessus si elle existait)"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_CXX_STANDARD=17
make -j"$(nproc)"

# runtime/ → build/ : ajoute / met à jour, ne supprime pas les fichiers présents seulement dans build/
if [[ -d "$RES_DIR" ]]; then
    if command -v rsync >/dev/null 2>&1; then
        rsync -a --exclude='SOURCE.txt' "$RES_DIR/" "$BUILD_DIR/"
    else
        cp -a "$RES_DIR/." "$BUILD_DIR/"
    fi
    echo "[build.sh] runtime/ → build/ synchronisé (sans effacer le reste de build/)."
else
    echo "[build.sh] ATTENTION: pas de répertoire runtime/ — lancez: ./scripts/copy_server_runtime.sh" >&2
fi

echo "[build.sh] OK : $BUILD_DIR/T4CServer"
