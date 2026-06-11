#!/bin/bash
SRC="/mnt/c/Users/tom/Documents/macgyver2/T4Serv3"
DEST="$HOME/Server_FinalStep/T4Serv3"

echo "=== copy check ==="
echo "SRC files:  $(find "$SRC" -type f 2>/dev/null | wc -l)"
echo "DEST files: $(find "$DEST" -type f 2>/dev/null | wc -l)"
test -f "$DEST/Exe Server/NPCStructure.h" && echo "NPCStructure.h: OK" || echo "NPCStructure.h: MISSING"
test -f "$DEST/build/Makefile" && echo "cmake: OK" || echo "cmake: NO"

echo "=== build ==="
if [ -f "$HOME/Server_FinalStep/build.log" ]; then
  grep -E '\[.*%\]' "$HOME/Server_FinalStep/build.log" | tail -1
  grep -E 'error:|fatal error' "$HOME/Server_FinalStep/build.log" | sort -u | head -15
fi
test -f "$DEST/build/T4CServer" && echo "BINARY: YES" || echo "BINARY: NO"

ps aux | grep make | grep -v grep | head -3 || echo "no make running"
