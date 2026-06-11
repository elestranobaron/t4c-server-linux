#!/bin/bash
ROOT="/mnt/c/Users/tom/Documents/macgyver2/T4Serv3/Exe Server"
for f in AsyncFuncQueue.cpp main.cpp Character.cpp TFC_MAIN.cpp AutoConfig.cpp CommCenter.cpp; do
  echo "=== $f ==="
  bash /mnt/c/Users/tom/Documents/macgyver2/T4Serv3/tools/compile_one.sh "$ROOT/$f" 2>&1 | grep -E 'error:|fatal' | head -8
  echo
done
