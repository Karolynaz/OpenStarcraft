#!/usr/bin/env python3
"""Stage a user's classic StarCraft MPQs and one map, without overwriting files."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", required=True, type=Path, help="Your classic StarCraft installation/extracted files")
    parser.add_argument("--map", required=True, type=Path, help="A trusted 32..64 tile .scm/.scx map")
    parser.add_argument("--dest", type=Path, default=Path(__file__).resolve().parents[1] / "local-data/3ds/openbw")
    args = parser.parse_args()
    if not args.source.is_dir():
        parser.error("--source must be a directory")
    paths = {}
    for name in ("Patch_rt.mpq", "BrooDat.mpq", "StarDat.mpq"):
        matches = [p for p in args.source.rglob("*") if p.is_file() and p.name.casefold() == name.casefold()]
        if len(matches) != 1:
            parser.error(f"Expected one {name}, found {len(matches)}. CASC Data/ files cannot be renamed to MPQ.")
        paths[Path(name)] = matches[0]
    paths[Path("maps/probe.scx")] = args.map
    manifest = {}
    # Validate the whole input set before copying anything.
    for relative, source in paths.items():
        if not source.is_file():
            parser.error(f"Missing file: {source}")
        with source.open("rb") as f:
            if f.read(4) != b"MPQ\x1a":
                parser.error(f"Not an MPQ with a header at offset zero: {source}")
            f.seek(0)
            digest = hashlib.file_digest(f, "sha256").hexdigest()
        dest = args.dest / relative
        if dest.exists():
            with dest.open("rb") as f:
                if hashlib.file_digest(f, "sha256").hexdigest() != digest:
                    parser.error(f"Refusing to replace different existing file: {dest}")
        manifest[str(relative)] = {"sha256": digest, "bytes": source.stat().st_size}
    for relative, source in paths.items():
        dest = args.dest / relative
        dest.parent.mkdir(parents=True, exist_ok=True)
        if not dest.exists():
            shutil.copyfile(source, dest)
        print(f"Ready: {relative}")
    (args.dest / "logs").mkdir(exist_ok=True)
    (args.dest / "assets-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Copy this private directory to SD /3ds/openbw/: {args.dest}")
    print("Map dimensions and internal asset compatibility are checked when the probe loads them.")


if __name__ == "__main__":
    main()
