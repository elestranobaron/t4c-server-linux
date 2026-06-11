#!/bin/bash
set -euo pipefail

SRC="/mnt/c/Users/tom/Documents/macgyver2/T4Serv3"
DEST="$HOME/Server_FinalStep/T4Serv3"

mkdir -p "$HOME/Server_FinalStep"

if [ -f "$DEST/CMakeLists.txt" ]; then
    echo "Already present: $DEST"
else
    echo "Copying $SRC -> $DEST ..."
    rsync -a --exclude=build --exclude=.git "$SRC/" "$DEST/"
    echo "Copy done."
fi

mkdir -p "$DEST/build"
cd "$DEST/build"
cmake ..
echo "CMake OK. Starting make -j4 ..."
make -j4 2>&1 | tee "$HOME/Server_FinalStep/build.log"
