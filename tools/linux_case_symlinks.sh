#!/bin/bash
# Case-sensitivity symlinks for Linux (NTFS is case-insensitive).
set -euo pipefail
EXE="$1"
if [ -z "$EXE" ]; then
  echo "Usage: $0 <path-to-Exe Server>" >&2
  exit 1
fi
cd "$EXE"

link_if_needed() {
  local target="$1"
  local alias="$2"
  if [ -f "$target" ] && [ ! -e "$alias" ]; then
    ln -s "$target" "$alias"
    echo "symlink: $alias -> $target"
  fi
}

# Known one-off aliases (Windows legacy names vs actual on-disk names).
link_if_needed NPCstructure.h NPCStructure.h
link_if_needed WorldMap.h Worldmap.h
link_if_needed Directions.h DIRections.h
link_if_needed StatModifierFlagsListing.h STATMODIFIERFLAGSLISTING.H
link_if_needed vdlist.h VDList.h
link_if_needed TFC_MAIN.h TFC_MAIN.H
link_if_needed Random.h random.h
link_if_needed PLAYERS.H Players.h

# ALLCAPS.H -> Titlecase.h (first char upper, rest lower).
for f in *.H; do
  [ -f "$f" ] || continue
  base="${f%.H}"
  title="$(printf '%s' "$base" | awk '{print toupper(substr($0,1,1)) tolower(substr($0,2))}').h"
  link_if_needed "$f" "$title"
done

# TitleCase.h / lowercase.h pairs.
for f in *.h; do
  [ -f "$f" ] || continue
  lower="$(printf '%s' "$f" | tr '[:upper:]' '[:lower:]')"
  if [ "$f" != "$lower" ]; then
    link_if_needed "$f" "$lower"
  fi
done
