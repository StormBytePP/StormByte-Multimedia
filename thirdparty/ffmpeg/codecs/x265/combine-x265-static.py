#!/usr/bin/env python3
"""
combine-x265-static.py - combine multiple static libraries into one archive

This script extracts object members from input static archives, renames them
with a numeric prefix to avoid name collisions, and repacks them into a
single output static archive. It supports GNU `ar`/`ranlib` and MSVC `lib`.

Usage:
  combine-x265-static.py -o OUT.a IN1.a IN2.a [...]

The script is intentionally conservative: it extracts each input archive into
its own temporary directory, prefixes member filenames with the input index,
then repacks all prefixed members into the output archive.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def which_lib_tool() -> str:
    # Prefer GNU ar if available, otherwise MSVC lib
    if shutil.which("ar"):
        return "ar"
    if shutil.which("lib") or shutil.which("lib.exe"):
        return "lib"
    raise SystemExit("Neither 'ar' nor 'lib' found in PATH")


def list_ar_members(ar: str) -> list[str]:
    out = subprocess.check_output(["ar", "t", ar], text=True, stderr=subprocess.DEVNULL)
    return [l.strip() for l in out.splitlines() if l.strip()]


def extract_ar(ar: str, dest: Path) -> None:
    # Extract members one-by-one using `ar p` to avoid tool-specific extraction
    members = list_ar_members(ar)
    for m in members:
        outp = dest / m
        # ensure parent dir exists
        outp.parent.mkdir(parents=True, exist_ok=True)
        with open(outp, "wb") as fh:
            data = subprocess.check_output(["ar", "p", ar, m])
            if isinstance(data, str):
                data = data.encode()
            fh.write(data)


def list_lib_members(lib: str) -> list[str]:
    # lib /LIST <lib> prints members on Windows
    cmd = shutil.which("lib") or shutil.which("lib.exe")
    out = subprocess.check_output([cmd, "/LIST", lib], text=True, stderr=subprocess.DEVNULL)
    members = []
    for line in out.splitlines():
        line = line.strip()
        if not line:
            continue
        # lib prints like: "  member.obj"
        members.append(os.path.basename(line))
    return members


def extract_lib(lib: str, dest: Path) -> None:
    cmd = shutil.which("lib") or shutil.which("lib.exe")
    members = list_lib_members(lib)
    for m in members:
        # /EXTRACT:member extracts into cwd
        subprocess.check_call([cmd, "/EXTRACT:%s" % m, lib], cwd=str(dest))


def repack_ar(output: str, files: list[Path]) -> None:
    cmd = ["ar", "rcs", output] + [str(p) for p in files]
    subprocess.check_call(cmd)
    if shutil.which("ranlib"):
        subprocess.check_call(["ranlib", output])


def repack_lib(output: str, files: list[Path]) -> None:
    cmd = shutil.which("lib") or shutil.which("lib.exe")
    args = [cmd, "/OUT:%s" % output] + [str(p) for p in files]
    subprocess.check_call(args)


def main() -> int:
    p = argparse.ArgumentParser(description="Combine static archives into one (avoid member name collisions)")
    p.add_argument("-o", "--output", required=True, help="Output archive path")
    p.add_argument("inputs", nargs="+", help="Input static archives to combine")
    args = p.parse_args()

    outputs = []
    tool = which_lib_tool()

    out_path = Path(args.output).resolve()
    # resolve inputs early and guard against including the output as an input
    resolved_inputs = [Path(i).resolve() for i in args.inputs]
    if out_path in resolved_inputs:
        print(f"Error: output '{out_path}' is listed among inputs; refusing to self-include")
        return 2
    if len(set(resolved_inputs)) != len(resolved_inputs):
        print("Error: duplicate input paths detected; please provide unique archives")
        return 2

    if out_path.exists():
        print(f"Warning: {out_path} exists and will be replaced")

    with tempfile.TemporaryDirectory(prefix="combine-x265-") as tmpdir:
        tmp = Path(tmpdir)
        staged: list[Path] = []

        for idx, inp in enumerate(args.inputs, start=1):
            inp_path = Path(inp).resolve()
            if not inp_path.exists():
                print(f"Input not found: {inp}")
                return 2

            # extract into its own subdir to avoid clashes
            sub = tmp / f"in{idx:02d}"
            sub.mkdir()

            print(f"Extracting {inp_path} into {sub}")
            if tool == "ar":
                extract_ar(str(inp_path), sub)
            else:
                extract_lib(str(inp_path), sub)

            # copy extracted members into staging with prefix (don't move originals)
            for member in sorted(sub.iterdir()):
                if not member.is_file():
                    continue
                newname = f"p{idx:02d}_{member.name}"
                dst = tmp / newname
                shutil.copy2(member, dst)
                staged.append(dst)

        if not staged:
            print("No members extracted; nothing to do.")
            return 0

        # ensure all staged members carry a pNN_ prefix; rename any stray files
        normalized: list[Path] = []
        next_idx = len(resolved_inputs) + 1
        for p in staged:
            name = p.name
            if not name.startswith("p") or not (len(name) > 3 and name[1:3].isdigit() and name[3] == '_'):
                # assign a new prefix to any unprefixed or oddly-named member
                newname = f"p{next_idx:02d}_{name}"
                newpath = tmp / newname
                p.rename(newpath)
                normalized.append(newpath)
                next_idx += 1
            else:
                normalized.append(p)

        staged = normalized

        print(f"Repacking {len(staged)} objects into {out_path}")
        # ensure parent exists
        out_path.parent.mkdir(parents=True, exist_ok=True)

        # Remove existing output archive to avoid appending to an old file
        if out_path.exists():
            out_path.unlink()

        if tool == "ar":
            repack_ar(str(out_path), staged)
        else:
            repack_lib(str(out_path), staged)

        print(f"Created {out_path} (combined {len(args.inputs)} input archives)")

    return 0


if __name__ == "__main__":
    sys.exit(main())
