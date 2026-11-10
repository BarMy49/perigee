#!/usr/bin/env python3
"""Create a deterministic, reviewable working-tree snapshot with pinned submodules."""
import argparse
import gzip
import hashlib
import io
import json
from pathlib import Path
import subprocess
import tarfile


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--include-untracked', action='append', default=[])
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    version = (root / 'app/version.txt').read_text().strip()
    prefix = 'perigee-' + version
    epoch = int(git(root, 'show', '-s', '--format=%ct', 'HEAD'))
    files = {}
    revisions = {}

    def collect(directory, relative=Path()):
        revisions[relative.as_posix()] = git(directory, 'rev-parse', 'HEAD').decode().strip()
        for record in git(directory, 'ls-files', '--stage', '-z').split(b'\0'):
            if not record:
                continue
            metadata, name = record.split(b'\t', 1)
            mode, revision, stage = metadata.decode().split()
            if stage != '0':
                raise ValueError('Unmerged index entries are not package sources')
            path = Path(name.decode())
            if not relative.parts and path.parts[0] == '.idea':
                continue
            if mode == '160000':
                child = directory / path
                if git(child, 'rev-parse', 'HEAD').decode().strip() != revision:
                    raise ValueError(f'Submodule pin mismatch: {relative / path}')
                if git(child, 'status', '--porcelain').strip():
                    raise ValueError(f'Modified submodule: {relative / path}')
                collect(child, relative / path)
            else:
                files[(relative / path).as_posix()] = directory / path

    collect(root)
    untracked = set(git(root, 'ls-files', '--others', '--exclude-standard', '-z').decode().split('\0'))
    for name in args.include_untracked:
        path = Path(name)
        workflow = (len(path.parts) == 3 and path.parts[:2] == ('.github', 'workflows')
                    and not path.name.startswith('.') and path.suffix in ('.yml', '.yaml'))
        if name not in untracked or '..' in path.parts or (name.startswith('.') and not workflow):
            raise ValueError(f'Not an explicit untracked source path: {name}')
        files[name] = root / name
    contents = {}
    modes = {}
    for name, path in sorted(files.items()):
        if path.is_symlink() or not path.is_file():
            raise ValueError(f'Only existing regular source files are supported: {name}')
        contents[name] = path.read_bytes()
        modes[name] = 0o755 if path.stat().st_mode & 0o111 else 0o644
    provenance = dict(revisions=revisions, epoch=epoch, version=version,
                      working_tree_status=git(root, 'status', '--porcelain', '--untracked-files=no').decode(),
                      explicitly_included_untracked=sorted(args.include_untracked),
                      sha256={name: hashlib.sha256(data).hexdigest() for name, data in contents.items()})
    contents['SOURCE_SNAPSHOT.json'] = (json.dumps(provenance, indent=2, sort_keys=True) + '\n').encode()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('xb') as output:
        with gzip.GzipFile(filename='', fileobj=output, mode='wb', mtime=epoch) as compressed:
            with tarfile.open(fileobj=compressed, mode='w') as archive:
                for name, data in sorted(contents.items()):
                    info = tarfile.TarInfo(prefix + '/' + name)
                    info.size, info.mtime, info.mode = len(data), epoch, modes.get(name, 0o644)
                    archive.addfile(info, io.BytesIO(data))
    print(f'{args.output}: {len(contents)} files; SHA256 {hashlib.sha256(args.output.read_bytes()).hexdigest()}')


if __name__ == '__main__':
    main()
