#!/bin/bash
# Quick-compile one source file using the existing CMake build graph.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$1"
if [ -z "$SRC" ]; then
  echo "Usage: $0 <path-to-cpp>" >&2
  exit 1
fi
SRC="$(readlink -f "$SRC" 2>/dev/null || realpath "$SRC" 2>/dev/null || echo "$SRC")"
ROOT="$(readlink -f "$ROOT" 2>/dev/null || realpath "$ROOT" 2>/dev/null || echo "$ROOT")"
relpath="${SRC#"$ROOT"/}"
relpath="${relpath// /_}"
target="CMakeFiles/T4CServer.dir/${relpath}.o"
make -C "$ROOT/build" -j1 "$target"
