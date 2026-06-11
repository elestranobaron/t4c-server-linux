#!/bin/bash
set -euo pipefail
SRC="/mnt/c/Users/tom/Documents/macgyver2/T4Serv3"
DEST="$HOME/Server_FinalStep/T4Serv3"
LOG="$HOME/Server_FinalStep/build.log"

sync_one() {
  rsync -a "$SRC/$1" "$DEST/$1"
}

echo "Syncing patched files..."
for rel in \
  "CMakeLists.txt" \
  "PORT_LINUX_CHANGELOG.md" \
  "Dll Gameops/StdAfx.h" \
  "Dll Npcs/StdAfx.h" \
  "Dll Npcs Addon/StdAfx.h" \
  "Dll Npcs Arakas/StdAfx.h" \
  "Dll Npcs RavensDust/StdAfx.h" \
  "Dll Npcs Remort/StdAfx.h" \
  "Dll Npcs Stoneheim/StdAfx.h" \
  "Dll Npcs WindHowl/StdAfx.h" \
  "Exe Server/PacketManager.h" \
  "Exe Server/PacketManager.cpp" \
  "Exe Server/Character.cpp" \
  "Exe Server/Unit.cpp" \
  "Exe Server/PlayerManager.cpp" \
  "Exe Server/TFCMessagesHandler.cpp" \
  "Exe Server/main.cpp" \
  "Exe Server/Win32Compat.h" \
  "Exe Server/Portability.h" \
  "Exe Server/TFCServerGP.h" \
  "Exe Server/TFCServerGP.cpp" \
  "Exe Server/T4CLog.h" \
  "Exe Server/DebugLogger.h" \
  "Exe Server/TFC Server.h" \
  "Exe Server/ChatterChannels.h" \
  "Exe Server/DevTools/PerformUtil.h" \
  "Exe Server/DevTools/PerformUtil.cpp" \
  "Exe Server/PLAYERS.H" \
  "Exe Server/PLAYERS.CPP" \
  "Exe Server/Unit.h" \
  "Exe Server/UDP/Socket.h" \
  "Exe Server/UDP/Socket.cpp" \
  "Exe Server/UDP/UDPClient.cpp" \
  "Exe Server/UDP/NMNetwork.h" \
  "Exe Server/UDP/NMPacketManager.cpp" \
  "Exe Server/UDP/NMPacketManager.h" \
  "tools/normalize_linux_case.py" \
  "tools/continue_finalstep.sh"
do
  sync_one "$rel"
done

python3 "$DEST/tools/normalize_linux_case.py" "$DEST"

CACHE="$DEST/build/CMakeCache.txt"
if [ ! -f "$CACHE" ] || grep -q '/mnt/c/' "$CACHE" 2>/dev/null; then
  echo "Fresh cmake (bad or missing cache)..."
  rm -rf "$DEST/build"
  mkdir -p "$DEST/build"
  cd "$DEST/build"
  cmake .. 2>&1 | tee "$LOG"
else
  echo "Reconfigure cmake (network pivot)..."
  cd "$DEST/build"
  cmake .. 2>&1 | tee "$LOG"
fi

echo "make -j4..."
make -j4 2>&1 | tee -a "$LOG"
