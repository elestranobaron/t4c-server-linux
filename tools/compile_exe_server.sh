#!/bin/bash
# Compile all Exe Server .cpp files and list unique errors (fast feedback loop).
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EXE="$ROOT/Exe Server"
LOG=/tmp/t4c_exe_errors.log
: > "$LOG"
ok=0
fail=0
for f in "$EXE"/*.cpp; do
  base=$(basename "$f")
  if bash "$ROOT/tools/compile_one.sh" "$f" >/dev/null 2>>"$LOG"; then
    ok=$((ok + 1))
  else
    fail=$((fail + 1))
    echo "FAIL: $base" >> "$LOG"
  fi
done
echo "=== $ok ok, $fail failed ==="
grep -E 'error:|fatal error' "$LOG" | sed 's/: error:/: /' | sort -u | head -80
