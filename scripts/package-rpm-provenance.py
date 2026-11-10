#!/usr/bin/env python3
"""Match inventoried ELF build IDs to installed RPMs and retain their notices."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess


def command(arguments):
    return subprocess.run(arguments, capture_output=True, text=True,
                          env={**os.environ, 'LC_ALL': 'C'})


def digest(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--notices', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or args.output.is_symlink() or args.notices.exists() or args.notices.is_symlink():
        parser.error('New report and notice paths are required')
    inventory = json.loads(args.inventory.read_text())
    args.notices.mkdir(parents=True)
    packages = {}
    matches = {}
    unmatched = []
    for name, metadata in inventory['elf_files'].items():
        match = None
        for build_id in metadata['build_ids']:
            if not re.fullmatch(r'[0-9a-f]{32,128}', build_id):
                continue
            candidate = Path('/usr/lib/.build-id') / build_id[:2] / build_id[2:]
            if not candidate.is_file():
                continue
            installed = candidate.resolve(strict=True)
            notes = command(['readelf', '-n', str(installed)])
            if notes.returncode or build_id not in re.findall(r'Build ID: (\S+)', notes.stdout):
                continue
            query = command(['rpm', '-qf', '--qf', '%{NEVRA}\n%{SOURCERPM}\n%{LICENSE}\n%{URL}\n', str(installed)])
            values = query.stdout.splitlines()
            if query.returncode or len(values) != 4:
                continue
            package, source_rpm, license_expression, url = values
            if not re.fullmatch(r'[A-Za-z0-9+_.:^~-]+', package):
                raise ValueError('Unsafe RPM package name')
            if package not in packages:
                license_files = command(['rpm', '-q', '--licensefiles', package])
                license_files.check_returncode()
                retained = []
                skipped = []
                for filename in license_files.stdout.splitlines():
                    path = Path(filename)
                    allowed = any(path.is_relative_to(root) for root in
                                  [Path('/usr/share/licenses'), Path('/usr/share/doc')])
                    if (not path.is_absolute() or '..' in path.parts or not allowed
                            or path.is_symlink() or not path.is_file()):
                        skipped.append(filename)
                        continue
                    destination = args.notices / package / path.relative_to('/')
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(path, destination)
                    retained.append({'source': filename, 'sha256': digest(destination),
                                     'retained_path': destination.relative_to(args.notices).as_posix()})
                packages[package] = {'source_rpm': source_rpm, 'rpm_license': license_expression,
                                     'upstream_url': url, 'retained_notices': retained,
                                     'skipped_notice_entries': skipped}
            installed_hash = digest(installed)
            match = {'package': package, 'build_id': build_id, 'installed_path': str(installed),
                     'installed_sha256': installed_hash,
                     'byte_identical': inventory['files'][name]['sha256'] == installed_hash}
            break
        if match is None:
            unmatched.append(name)
        else:
            matches[name] = match
    sources = {}
    for package, data in packages.items():
        source = sources.setdefault(data['source_rpm'], {'packages': [], 'retained_notice_count': 0})
        source['packages'].append(package)
        source['retained_notice_count'] += len(data['retained_notices'])
    report = {'inventory_sha256': digest(args.inventory), 'elf_matches': matches,
              'unmatched_elf_files': unmatched, 'packages': packages,
              'source_rpms': sources,
              'source_rpms_without_retained_notices': sorted(name for name, data in sources.items()
                                                           if data['retained_notice_count'] == 0),
              'limitations': 'Build-ID correspondence to this installed RPM database, not proof of byte identity, source availability, ABI compatibility or complete license clearance. Patched/stripped libraries may retain their build ID. RPM license fields are recorded verbatim; static dependencies and all corresponding source still need review.'}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as output:
        json.dump(report, output, indent=2, sort_keys=True)
        output.write('\n')
    print(f'{len(matches)} ELF build-ID matches; {len(packages)} RPM packages; {len(unmatched)} unmatched ELF files')


if __name__ == '__main__':
    main()
