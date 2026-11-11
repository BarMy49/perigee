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


def retain_notices(package, notice_root):
    license_files = command(['rpm', '-q', '--licensefiles', package])
    license_files.check_returncode()
    owned_files = command(['rpm', '-ql', package])
    owned_files.check_returncode()
    candidates = {filename: 'rpm-license' for filename in license_files.stdout.splitlines()}
    for filename in owned_files.stdout.splitlines():
        path = Path(filename)
        if re.fullmatch(r'(COPYING|LICENSE|LICENCE|NOTICE|COPYRIGHT)([._-].*)?', path.name, re.IGNORECASE):
            candidates.setdefault(filename, 'named-license')
        elif re.fullmatch(r'(AUTHORS|README)([._-].*)?', path.name, re.IGNORECASE):
            candidates.setdefault(filename, 'named-attribution')
    retained = []
    skipped = []
    for filename, basis in sorted(candidates.items()):
        path = Path(filename)
        allowed = any(path.is_relative_to(root) for root in
                      [Path('/usr/share/licenses'), Path('/usr/share/doc')])
        if (not path.is_absolute() or '..' in path.parts or not allowed
                or path.is_symlink() or not path.is_file()):
            skipped.append(filename)
            continue
        destination = notice_root / package / path.relative_to('/')
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, destination)
        retained.append({'source': filename, 'basis': basis, 'sha256': digest(destination),
                         'retained_path': destination.relative_to(notice_root).as_posix()})
    return retained, skipped


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--notices', type=Path, required=True)
    parser.add_argument('--notice-package', action='append', default=[],
                        help='Additional installed notice provider from an inventoried source RPM')
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
                retained, skipped = retain_notices(package, args.notices)
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
    notice_packages = {}
    source_names = {data['source_rpm'] for data in packages.values()}
    for requested in args.notice_package:
        query = command(['rpm', '-q', '--qf', '%{NEVRA}\n%{SOURCERPM}\n%{LICENSE}\n%{URL}\n', '--', requested])
        query.check_returncode()
        values = query.stdout.splitlines()
        if len(values) != 4:
            raise ValueError('A single installed notice provider is required')
        package, source_rpm, license_expression, url = values
        if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9+_.:^~-]*', package):
            raise ValueError('Unsafe RPM package name')
        if source_rpm not in source_names:
            raise ValueError('Notice provider is not from an inventoried source RPM')
        if package in packages or package in notice_packages:
            continue
        retained, skipped = retain_notices(package, args.notices)
        notice_packages[package] = {'source_rpm': source_rpm, 'rpm_license': license_expression,
                                    'upstream_url': url, 'retained_notices': retained,
                                    'skipped_notice_entries': skipped}
    sources = {}
    for package, data in list(packages.items()) + list(notice_packages.items()):
        source = sources.setdefault(data['source_rpm'], {'packages': [], 'notice_provider_packages': [], 'retained_notice_count': 0, 'license_candidate_count': 0})
        source['packages' if package in packages else 'notice_provider_packages'].append(package)
        source['retained_notice_count'] += len(data['retained_notices'])
        source['license_candidate_count'] += sum(n['basis'] != 'named-attribution' for n in data['retained_notices'])
    report = {'inventory_sha256': digest(args.inventory), 'elf_matches': matches,
              'unmatched_elf_files': unmatched, 'packages': packages,
              'notice_provider_packages': notice_packages,
              'source_rpms': sources,
              'source_rpms_without_retained_notices': sorted(name for name, data in sources.items()
                                                           if data['retained_notice_count'] == 0),
              'source_rpms_without_license_candidates': sorted(name for name, data in sources.items()
                                                             if data['license_candidate_count'] == 0),
              'limitations': 'Named license candidates and attribution documents are not legal classification. Build-ID correspondence to this installed RPM database, not proof of byte identity, source availability, ABI compatibility or complete license clearance. Patched/stripped libraries may retain their build ID. RPM license fields are recorded verbatim; static dependencies and all corresponding source still need review.'}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open('x') as output:
        json.dump(report, output, indent=2, sort_keys=True)
        output.write('\n')
    print(f'{len(matches)} ELF build-ID matches; {len(packages)} RPM packages; {len(unmatched)} unmatched ELF files')


if __name__ == '__main__':
    main()
