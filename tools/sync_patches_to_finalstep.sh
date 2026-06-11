#!/bin/bash
# Copie les fichiers modifiés (liste) depuis le miroir Cursor vers Server_FinalStep.
set -euo pipefail
D=~/Server_FinalStep/T4Serv3
W=/mnt/c/Users/tom/Documents/macgyver2/T4Serv3
while IFS= read -r rel; do
  [ -z "$rel" ] && continue
  mkdir -p "$(dirname "$D/$rel")"
  cp "$W/$rel" "$D/$rel"
done <<'FILES'
Exe Server/UDP/NMPacketManager.cpp
Exe Server/UDP/UDPClient.cpp
Exe Server/NMWda/WDAStruct.h
Exe Server/AsyncFuncQueue.cpp
Exe Server/AutoConfig.cpp
Exe Server/DeadlockDetector.cpp
Exe Server/GuildMaster.cpp
Exe Server/MainConsole.h
Exe Server/MainConsole.cpp
Exe Server/MD5/MD5Checksum.cpp
Exe Server/Win32Compat.h
Exe Server/Portability.h
FILES
echo "sync_patches: OK"
