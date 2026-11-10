#!/usr/bin/env python3
"""Fetch reviewed AppImage tools with digest verification and no overwrites."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=Path)
    args = parser.parse_args()
    args.destination.mkdir(parents=True, exist_ok=True)
    manifest = Path(__file__).resolve().parents[1] / 'packaging/appimage/tools.json'
    for entry in json.loads(manifest.read_text()):
        target = args.destination / entry['name']
        if target.is_symlink():
            raise ValueError(f'Refusing symlink: {target}')
        if target.exists():
            if hashlib.sha256(target.read_bytes()).hexdigest() != entry['sha256']:
                raise ValueError(f'Existing tool digest mismatch: {target}')
            print(f'Verified existing tool: {target.name}')
            continue
        staged = None
        try:
            with tempfile.NamedTemporaryFile(dir=args.destination, delete=False) as output:
                staged = Path(output.name)
            subprocess.run(['curl', '--fail', '--location', '--proto', '=https',
                            '--output', str(staged), entry['url']], check=True)
            if hashlib.sha256(staged.read_bytes()).hexdigest() != entry['sha256']:
                raise ValueError(f'Downloaded tool digest mismatch: {entry["name"]}')
            staged.chmod(0o755)
            os.link(staged, target)
            print(f'Verified downloaded tool: {target.name}')
        finally:
            if staged is not None:
                staged.unlink(missing_ok=True)


if __name__ == '__main__':
    main()
