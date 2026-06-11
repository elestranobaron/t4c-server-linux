#!/bin/bash
ROOT="/mnt/c/Users/tom/Documents/macgyver2/T4Serv3/Exe Server/Wda"
for f in "$ROOT"/*.cpp; do
  [ -f "$f" ] || continue
  echo "=== $(basename "$f") ==="
  bash /mnt/c/Users/tom/Documents/macgyver2/T4Serv3/tools/compile_one.sh "$f" 2>&1 | grep -E 'error:|fatal' | head -3
done | head -40
