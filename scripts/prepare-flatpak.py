#!/usr/bin/env python3
"""Prepare a local Flatpak manifest from an explicit Perigee source snapshot."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import tarfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    source = args.source.resolve(strict=True)
    with tarfile.open(source) as archive:
        members = archive.getmembers()
        names = [member.name for member in members]
        if len(names) != len(set(names)) or any(not member.isfile() for member in members):
            parser.error('Source archive must contain unique regular files')
        if any(PurePosixPath(name).is_absolute() or '..' in PurePosixPath(name).parts
               or '\\' in name for name in names):
            parser.error('Unsafe source archive path')
        roots = {name.split('/')[0] for name in names}
        if len(roots) != 1:
            parser.error('Source archive must have exactly one root')
        root = next(iter(roots))
        snapshot = json.load(archive.extractfile(root + '/SOURCE_SNAPSHOT.json'))
        expected = {root + '/' + name for name in snapshot['sha256']}
        expected.add(root + '/SOURCE_SNAPSHOT.json')
        if set(names) != expected:
            parser.error('Source archive contains missing or unrecorded files')
        for name, digest in snapshot['sha256'].items():
            if hashlib.sha256(archive.extractfile(root + '/' + name).read()).hexdigest() != digest:
                parser.error('Source snapshot hash mismatch: ' + name)
    template = Path(__file__).resolve().parents[1] / 'packaging/flatpak/io.github.BarMy49.Perigee.json.in'
    manifest = json.loads(template.read_text())
    manifest['modules'][-1]['sources'][0].update(path=str(source), sha256=hashlib.sha256(source.read_bytes()).hexdigest())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as output:
        json.dump(manifest, output, indent=2)
        output.write('\n')
    print(args.output)


if __name__ == '__main__':
    main()
