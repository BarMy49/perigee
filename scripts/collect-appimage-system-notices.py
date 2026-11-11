#!/usr/bin/env python3
"""Retain dpkg copyright/source identities for the actual AppImage ELF payload."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import struct
import subprocess
from urllib.parse import urlsplit


CUSTOM_LIBRARIES = (
    (r'libSDL3\.so(?:\..*)?', 'SDL3'),
    (r'libSDL2(?:-2\.0)?\.so(?:\..*)?', 'SDL2-compat'),
    (r'libSDL2_ttf(?:-2\.0)?\.so(?:\..*)?', 'SDL2_ttf'),
    (r'lib(?:avcodec|avutil|avformat|swscale)\.so(?:\..*)?', 'FFmpeg'),
    (r'libva(?:-x11)?\.so(?:\..*)?', 'libva'),
    (r'libplacebo\.so(?:\..*)?', 'libplacebo'),
)
PROJECT_ELFS = {'usr/bin/perigee', 'opt/libva-fallback/libva-probe'}


def command(*arguments):
    return subprocess.run(arguments, capture_output=True, text=True,
                          env={**os.environ, 'LC_ALL': 'C'})


def checked(*arguments):
    result = command(*arguments)
    result.check_returncode()
    return result.stdout


def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as source:
        for data in iter(lambda: source.read(1024 * 1024), b''):
            value.update(data)
    return value.hexdigest()


def build_id(path):
    with path.open('rb') as source:
        header = source.read(16)
        if header[:4] != b'\x7fELF':
            return None
        if len(header) != 16 or header[5] not in (1, 2):
            raise ValueError('Unsupported ELF header: ' + str(path))
        sections = checked('readelf', '-SW', str(path))
        match = re.search(r'\.note\.gnu\.build-id\s+NOTE\s+\S+\s+([0-9a-f]+)\s+([0-9a-f]+)', sections)
        if not match:
            raise ValueError('GNU build ID is required: ' + str(path))
        offset, size = (int(value, 16) for value in match.groups())
        if size < 16 or size > 4096:
            raise ValueError('Invalid build-ID section size')
        source.seek(offset)
        note = source.read(size)
    if len(note) != size:
        raise ValueError('Truncated build-ID section')
    name_size, description_size, note_type = struct.unpack(('<' if header[5] == 1 else '>') + 'III', note[:12])
    start = 12 + (name_size + 3) // 4 * 4
    if (note_type != 3 or note[12:12 + name_size] != b'GNU\0'
            or not 16 <= description_size <= 64 or start + description_size > len(note)):
        raise ValueError('Invalid GNU build-ID note')
    return note[start:start + description_size].hex()


def sdk_packages():
    result = checked('dpkg-query', '-W', '-f=${binary:Package}\t${Version}\t${source:Package}\t${source:Version}\t${Architecture}\n')
    packages = {}
    for line in result.splitlines():
        fields = line.split('\t')
        if len(fields) != 5:
            raise ValueError('Unexpected dpkg package metadata')
        name, version, source, source_version, architecture = fields
        if not re.fullmatch(r'[a-z0-9][a-z0-9+.-]*(?::[a-z0-9][a-z0-9-]*)?', name):
            raise ValueError('Unsafe binary package name')
        packages[name] = {'version': version, 'source_package': source or name.split(':')[0],
                          'source_version': source_version or version, 'architecture': architecture}
    return packages


def sdk_matches(names, packages):
    candidates = set()
    owner_paths = set()
    for filename in checked('dpkg-query', '-L', *packages).splitlines():
        path = Path(filename)
        if path.name in names and path.is_file():
            candidates.add(path.resolve())
            owner_paths.update((path, path.resolve()))
    for directory in ('/usr/local/lib', '/usr/local/lib64'):
        for path in Path(directory).rglob('*'):
            if path.name in names and path.is_file():
                candidates.add(path.resolve())
    owners = {}
    query_paths = sorted(owner_paths | candidates)
    for index in range(0, len(query_paths), 40):
        result = command('dpkg-query', '-S', *(str(path) for path in query_paths[index:index + 40]))
        if result.returncode not in (0, 1):
            result.check_returncode()
        for line in result.stdout.splitlines():
            if ': /' not in line or line.startswith(('diversion by', 'local diversion')):
                continue
            package_names, filename = line.split(': ', 1)
            for name in package_names.split(', '):
                if name in packages:
                    owners.setdefault(str(Path(filename).resolve()), set()).add(name)
    matches = {}
    for path in sorted(candidates):
        identity = build_id(path)
        if identity is not None:
            matches.setdefault(identity, []).append(path)
    return matches, owners


def collect(tree):
    notice_tools = runpy.run_path(str(Path(__file__).with_name('collect-appimage-notices.py')))
    custom_files = notice_tools['verify_bundle'](tree / 'usr/share/licenses/perigee/dependencies')
    custom_manifest = json.loads(custom_files['manifest.json'])
    components = custom_manifest.get('components')
    if not isinstance(components, dict):
        raise ValueError('Verified custom-source notice components are required')
    for name, metadata in components.items():
        if (not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9_.-]*', name) or not isinstance(metadata, dict)
                or not re.fullmatch(r'[0-9a-f]{40}', metadata.get('revision', ''))
                or not isinstance(metadata.get('notice_paths'), list) or not metadata['notice_paths']):
            raise ValueError('Invalid custom-source notice identity')
        origin = urlsplit(metadata.get('origin', ''))
        if (origin.scheme != 'https' or not origin.netloc or origin.username or origin.password
                or origin.query or origin.fragment):
            raise ValueError('Custom-source origins must be credential-free HTTPS URLs')
        for notice in metadata['notice_paths']:
            notice_tools['relative_path'](notice)
            if name + '/' + notice not in custom_files:
                raise ValueError('Custom source has an unretained notice')
    elfs = {}
    for path in sorted(tree.rglob('*')):
        if path.is_symlink():
            if not path.exists() or not path.resolve().is_relative_to(tree):
                raise ValueError('Package symlinks must stay inside the AppDir')
        elif path.is_file():
            identity = build_id(path)
            if identity is not None:
                elfs[path.relative_to(tree).as_posix()] = (path, identity)
    if 'usr/bin/perigee' not in elfs:
        raise ValueError('A staged Perigee application is required')
    packages = sdk_packages()
    matches, owners = sdk_matches({Path(name).name for name in elfs}, packages)
    records = {}
    used = set()
    for name, (path, identity) in elfs.items():
        record = {'sha256': digest(path), 'build_id': identity}
        if name in PROJECT_ELFS:
            record['kind'] = 'project'
        else:
            counterparts = matches.get(identity, [])
            package_names = sorted({package for candidate in counterparts
                                    for package in owners.get(str(candidate), set())})
            record['sdk_counterparts'] = [{'path': str(candidate), 'sha256': digest(candidate)}
                                          for candidate in counterparts]
            if package_names:
                record.update(kind='dpkg', packages=package_names)
                used.update(package_names)
            else:
                component = next((component for pattern, component in CUSTOM_LIBRARIES
                                  if re.fullmatch(pattern, Path(name).name)), None)
                if (component not in components or not counterparts
                        or not all(any(candidate.is_relative_to(root) for root in
                                       (Path('/usr/local/lib'), Path('/usr/local/lib64')))
                                   for candidate in counterparts)):
                    raise ValueError('Unattributed packaged ELF: ' + name)
                record.update(kind='custom-source', component=component)
        records[name] = record
    files = {}
    retained_packages = {}

    def retain(path, name, allowed_root):
        resolved = path.resolve(strict=True)
        if not resolved.is_relative_to(allowed_root) or not resolved.is_file() or resolved.stat().st_size > 4 * 1024 * 1024:
            raise ValueError('Unsupported installed notice file')
        if sum(map(len, files.values())) + resolved.stat().st_size > 64 * 1024 * 1024:
            raise ValueError('System notice collection exceeds size bounds')
        files[name] = resolved.read_bytes()
        return {'sdk_path': str(path), 'resolved_path': str(resolved), 'retained_path': name,
                'sha256': hashlib.sha256(files[name]).hexdigest()}

    for name in sorted(used):
        metadata = packages[name].copy()
        path = Path('/usr/share/doc') / name.split(':')[0] / 'copyright'
        metadata['copyright'] = retain(path, 'packages/' + name.replace(':', '_') + '/copyright', Path('/usr/share/doc'))
        retained_packages[name] = metadata
    for path in sorted(Path('/usr/share/common-licenses').iterdir()):
        if path.is_file():
            retain(path, 'common-licenses/' + path.name, Path('/usr/share/common-licenses'))
    manifest = {'format': 1, 'packages': retained_packages, 'elfs': records,
                'custom_notice_manifest_sha256': hashlib.sha256(custom_files['manifest.json']).hexdigest(),
                'custom_source_components': components,
                'files': {name: hashlib.sha256(data).hexdigest() for name, data in sorted(files.items())},
                'limitations': 'Build-ID correspondence and installed dpkg copyright/source metadata, not binary reproducibility or complete legal classification. Stripping or patching can preserve build IDs. Custom sources/static libraries, individual uploader signatures and corresponding-source distribution remain separately reviewed.'}
    files['manifest.json'] = (json.dumps(manifest, indent=2, sort_keys=True) + '\n').encode()
    return files, manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tree', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    tree = args.tree.resolve(strict=True)
    if not tree.is_dir() or tree == Path('/') or args.output.exists() or args.output.is_symlink():
        parser.error('An AppDir and a new system notice output path are required')
    files, manifest = collect(tree)
    args.output.mkdir(parents=True)
    for name, data in sorted(files.items()):
        target = args.output / name
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open('xb') as output:
            output.write(data)
        target.chmod(0o644)
    print(f"{len(manifest['packages'])} dpkg packages; {len(files) - 1} system notice files; {len(manifest['elfs'])} attributed ELFs")


if __name__ == '__main__':
    main()
