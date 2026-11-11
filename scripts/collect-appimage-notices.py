#!/usr/bin/env python3
"""Collect exact Git-tracked dependency notices, or verify and install a bundle."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
from urllib.parse import urlsplit


def digest(data):
    return hashlib.sha256(data).hexdigest()


def git(directory, *arguments):
    return subprocess.check_output(['git', '-c', 'safe.directory=' + str(directory),
                                    '-C', str(directory), *arguments],
                                   env={**os.environ, 'LC_ALL': 'C'})


def relative_path(name):
    path = Path(name)
    if not name or path.is_absolute() or '..' in path.parts or path.as_posix() != name:
        raise ValueError('Unsafe notice path')
    return path


def collect(sources):
    files = {}
    components = {}
    for specification in sources:
        name, separator, directory = specification.partition('=')
        if (not separator or not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9_.-]*', name)
                or name == 'manifest.json' or name in components):
            raise ValueError('Unique component=source-directory arguments are required')
        source = Path(directory).resolve(strict=True)
        if not source.is_dir() or Path(git(source, 'rev-parse', '--show-toplevel').decode().strip()).resolve() != source:
            raise ValueError('Each component must name its Git source root')
        revision = git(source, 'rev-parse', 'HEAD').decode().strip()
        origin = git(source, 'remote', 'get-url', 'origin').decode().strip()
        url = urlsplit(origin)
        if (url.scheme != 'https' or not url.netloc or url.username or url.password
                or url.query or url.fragment):
            raise ValueError('Dependency origins must be credential-free HTTPS URLs')
        tracked = git(source, 'ls-files', '-z').decode().split('\0')
        candidates = []
        for filename in tracked:
            if not filename:
                continue
            path = relative_path(filename)
            root_notice = len(path.parts) == 1 and re.fullmatch(r'(LICENSE|LICENCE|COPYING|COPYRIGHT|NOTICE|AUTHORS)([._-].*)?', path.name, re.IGNORECASE)
            license_directory = len(path.parts) > 1 and path.parts[0].upper() in ('LICENSES', 'LICENCES', 'COPYING')
            freetype_notice = filename in ('docs/FTL.TXT', 'docs/GPLv2.TXT')
            if root_notice or license_directory or freetype_notice:
                candidates.append(filename)
        if not any(not Path(filename).name.upper().startswith('AUTHORS') for filename in candidates):
            raise ValueError('Component has no retained license candidate: ' + name)
        for filename in sorted(candidates):
            path = source / filename
            if path.is_symlink() or not path.is_file() or path.stat().st_size > 4 * 1024 * 1024:
                raise ValueError('Only bounded regular notice files are supported')
            if sum(len(data) for data in files.values()) + path.stat().st_size > 64 * 1024 * 1024:
                raise ValueError('Notice collection exceeds size bounds')
            data = path.read_bytes()
            if data != git(source, 'show', 'HEAD:' + filename):
                raise ValueError('Notice differs from its pinned Git source: ' + name + '/' + filename)
            files[name + '/' + filename] = data
        components[name] = {'revision': revision, 'origin': origin, 'notice_paths': sorted(candidates)}
    if any(name.endswith('/docs/FTL.TXT') for name in files):
        files['FreeType-NOTICE'] = b'This package uses FreeType, copyright The FreeType Project (https://freetype.org). See the included FreeType license texts.\n'
    manifest = {'format': 1, 'components': components,
                'files': {name: digest(data) for name, data in sorted(files.items())},
                'limitations': 'Pinned source notice candidates and attribution, not complete legal classification or a static/dynamic binary ownership audit.'}
    files['manifest.json'] = (json.dumps(manifest, indent=2, sort_keys=True) + '\n').encode()
    return files


def verify_bundle(directory):
    if directory.is_symlink() or not directory.is_dir():
        raise ValueError('Notice bundle must be a real directory')
    manifest_path = directory / 'manifest.json'
    if manifest_path.is_symlink() or not manifest_path.is_file() or manifest_path.stat().st_size > 1024 * 1024:
        raise ValueError('Regular notice manifest is required')
    manifest = json.loads(manifest_path.read_text())
    if manifest.get('format') != 1 or not isinstance(manifest.get('files'), dict):
        raise ValueError('Unsupported notice manifest')
    records = manifest['files']
    if 'manifest.json' in records:
        raise ValueError('Manifest cannot replace itself')
    for name, checksum in records.items():
        relative_path(name)
        if not re.fullmatch(r'[0-9a-f]{64}', checksum):
            raise ValueError('Invalid notice digest')
    actual = set()
    for path in directory.rglob('*'):
        if path.is_symlink():
            raise ValueError('Symlinked notice bundle entries are unsupported')
        if path.is_file():
            actual.add(path.relative_to(directory).as_posix())
    if actual != set(records) | {'manifest.json'}:
        raise ValueError('Notice bundle contains missing or unrecorded files')
    files = {'manifest.json': manifest_path.read_bytes()}
    total_size = 0
    for name, checksum in records.items():
        path = directory / name
        size = path.stat().st_size
        total_size += size
        if size > 4 * 1024 * 1024 or total_size > 64 * 1024 * 1024:
            raise ValueError('Notice bundle exceeds size bounds')
        data = path.read_bytes()
        if digest(data) != checksum:
            raise ValueError('Notice bundle digest mismatch')
        files[name] = data
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', action='append', default=[])
    parser.add_argument('--install', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if bool(args.source) == bool(args.install) or args.output.exists() or args.output.is_symlink():
        parser.error('Choose sources or a bundle, and a new output directory')
    files = verify_bundle(args.install) if args.install else collect(args.source)
    args.output.mkdir(parents=True)
    for name, data in sorted(files.items()):
        target = args.output / relative_path(name)
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open('xb') as output:
            output.write(data)
        target.chmod(0o644)
    print(f'{len(files) - 1} dependency notice/attribution files retained')


if __name__ == '__main__':
    main()
