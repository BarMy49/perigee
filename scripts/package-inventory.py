#!/usr/bin/env python3
"""Record an installed package tree and its actual notice/source metadata."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess


def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            value.update(chunk)
    return value.hexdigest()


def elf_metadata(path):
    with path.open('rb') as source:
        if source.read(4) != b'\x7fELF':
            return None
    environment = {**os.environ, 'LC_ALL': 'C'}
    dynamic = subprocess.run(['readelf', '-d', str(path)], check=True, capture_output=True, text=True, env=environment)
    notes = subprocess.run(['readelf', '-n', str(path)], check=True, capture_output=True, text=True, env=environment)
    soname = re.findall(r'\(SONAME\).*?\[(.*?)\]', dynamic.stdout)
    return {'needed': sorted(re.findall(r'\(NEEDED\).*?\[(.*?)\]', dynamic.stdout)),
            'soname': soname[0] if soname else None,
            'build_ids': sorted(set(re.findall(r'Build ID: (\S+)', notes.stdout)))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tree', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    tree = args.tree.resolve(strict=True)
    if not tree.is_dir() or tree == Path('/') or args.output.exists() or args.output.resolve().is_relative_to(tree):
        parser.error('A package tree and a new output path are required')
    records = {}
    notices = []
    snapshots = {}
    elf_files = {}
    for path in sorted(tree.rglob('*')):
        name = path.relative_to(tree).as_posix()
        if path.is_symlink():
            records[name] = {'symlink': str(path.readlink())}
        elif path.is_file():
            records[name] = {'sha256': digest(path), 'size': path.stat().st_size}
            metadata = elf_metadata(path)
            if metadata is not None:
                elf_files[name] = metadata
            if '/licenses/' in name or '/doc/perigee/' in name:
                notices.append(name)
            if path.name == 'SOURCE_SNAPSHOT.json':
                snapshots[name] = json.loads(path.read_text())
    needed = {name: data['needed'] for name, data in elf_files.items()
              if name in ('bin/perigee', 'usr/bin/perigee')}
    providers = {}
    for name, data in elf_files.items():
        for library in {Path(name).name, data['soname']} - {None}:
            providers.setdefault(library, []).append(name)
    external = sorted({library for data in elf_files.values() for library in data['needed']
                       if library not in providers})
    report = {'files': records, 'notice_files': notices, 'source_snapshots': snapshots,
              'direct_elf_dependencies': needed, 'elf_files': elf_files,
              'bundled_elf_providers': providers, 'elf_requirements_outside_tree': external,
              'limitations': 'File/notice/ELF inventory only; SONAME matches are not loader resolution. Does not establish complete license review, corresponding-source publication, dlopen dependencies or runtime compatibility. Requirements outside the tree may be supplied by the host or Flatpak runtime.'}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as output:
        json.dump(report, output, indent=2, sort_keys=True)
        output.write('\n')
    print(f'{len(records)} files/symlinks; {len(notices)} notice/source records; {len(elf_files)} ELF files')


if __name__ == '__main__':
    main()
