#!/usr/bin/env python3
"""Fix Windows case-collapsed filenames for Linux (NTFS is case-insensitive).

1. Crypto: lowercase file = content, MixedCase.h = thin alias #include
2. Everywhere else: symlink missing include spellings to on-disk names
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

INCLUDE_RE = re.compile(r'#include\s+"([^"]+)"')
SOURCE_SUFFIXES = {".cpp", ".c", ".h", ".H", ".CPP"}

CMAKE_INCLUDE_DIRS = [
    "Exe Server",
    "Crypto",
    "Dll Gameops",
    "Dll Npcs",
    "Dll Npcs Addon",
    "Dll Npcs Arakas",
    "Dll Npcs RavensDust",
    "Dll Npcs Remort",
    "Dll Npcs Stoneheim",
    "Dll Npcs WindHowl",
]


def setup_crypto_aliases(crypto: Path) -> None:
    for canonical, alias in (("xorkey.h", "Xorkey.h"), ("crypt.h", "Crypt.h")):
        canon = crypto / canonical
        alias_path = crypto / alias
        if alias_path.is_file() and not alias_path.is_symlink() and not canon.exists():
            alias_path.rename(canon)
            print(f"rename: Crypto/{alias} -> Crypto/{canonical}")
        if not canon.is_file():
            continue
        if alias_path.exists():
            continue
        guard = alias.upper().replace(".", "_")
        alias_path.write_text(
            f"#ifndef T4C_{guard}_ALIAS\n"
            f"#define T4C_{guard}_ALIAS\n"
            f'#include "{canonical}"\n'
            f"#endif\n",
            encoding="utf-8",
        )
        print(f"alias: Crypto/{alias} -> {canonical}")


def find_file_insensitive(directory: Path, name: str) -> Path | None:
    if not directory.is_dir():
        return None
    exact = directory / name
    if exact.is_file():
        return exact
    lower = name.lower()
    for entry in directory.iterdir():
        if entry.is_file() and entry.name.lower() == lower:
            return entry
    return None


def ensure_symlink(link: Path, target: Path) -> bool:
    if link.exists() or link.is_symlink():
        return False
    link.symlink_to(target.name)
    print(f"symlink: {link} -> {target.name}")
    return True


def collect_includes(root: Path) -> set[str]:
    out: set[str] = set()
    for path in root.rglob("*"):
        if path.suffix not in SOURCE_SUFFIXES or not path.is_file():
            continue
        if "build" in path.parts:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except OSError:
            continue
        out.update(INCLUDE_RE.findall(text))
    return out


def include_dirs(root: Path) -> list[Path]:
    dirs: list[Path] = []
    for name in CMAKE_INCLUDE_DIRS:
        p = root / name
        if p.is_dir():
            dirs.append(p)
    exe = root / "Exe Server"
    if exe.is_dir():
        for sub in exe.iterdir():
            if sub.is_dir():
                dirs.append(sub)
    return dirs


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <project-root>", file=sys.stderr)
        return 2

    root = Path(sys.argv[1]).resolve()
    crypto = root / "Crypto"
    if crypto.is_dir():
        setup_crypto_aliases(crypto)

    includes = collect_includes(root)
    idirs = include_dirs(root)
    created = 0

    for inc in sorted(includes):
        inc_path = Path(inc)
        if inc_path.is_absolute():
            continue

        if len(inc_path.parts) == 1:
            name = inc_path.name
            for d in idirs:
                want = d / name
                if want.is_file():
                    continue
                hit = find_file_insensitive(d, name)
                if hit and hit.name != name and ensure_symlink(want, hit):
                    created += 1
        else:
            # Path-relative includes (e.g. ../Exe Server/Foo.h)
            target = (root / inc_path).resolve()
            try:
                target.relative_to(root)
            except ValueError:
                continue
            if target.is_file():
                continue
            hit = find_file_insensitive(target.parent, target.name)
            if hit and hit.name != target.name and ensure_symlink(target, hit):
                created += 1

            # Also try relative to each DLL folder (StdAfx pattern)
            for dll in idirs:
                if not dll.name.startswith("Dll"):
                    continue
                rel_target = (dll / inc_path).resolve()
                try:
                    rel_target.relative_to(root)
                except ValueError:
                    continue
                if rel_target.is_file():
                    continue
                hit = find_file_insensitive(rel_target.parent, rel_target.name)
                if hit and hit.name != rel_target.name and ensure_symlink(rel_target, hit):
                    created += 1

    print(f"Done. {created} symlink(s) created.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
