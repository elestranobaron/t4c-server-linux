#!/bin/bash
# Compile every Exe Server .cpp sequentially; print unique errors.
set -euo pipefail
ROOT="/mnt/c/Users/tom/Documents/macgyver2/T4Serv3"
EXE="$ROOT/Exe Server"
LOG=/tmp/t4c_exe_all.log
: > "$LOG"
ok=0
fail=0
exec 200>"$ROOT/build/.compile.lock"
for f in "$EXE"/*.cpp; do
  base=$(basename "$f")
  if flock -x 200 bash "$ROOT/tools/compile_one.sh" "$f" >>"$LOG" 2>&1; then
    ok=$((ok + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $base" >> "$LOG"
  fi
done
echo "=== $ok ok, $fail failed ==="
grep -E 'error:|fatal error' "$LOG" | sed 's|.*/Exe Server/||' | sort -u | head -80
