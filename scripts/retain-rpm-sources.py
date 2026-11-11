#!/usr/bin/env python3
"""Retain exact Fedora/RPM Fusion source RPMs for an installed-package inventory."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import uuid


ENVIRONMENT = {**os.environ, 'LC_ALL': 'C'}


def digest(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest()


def command(arguments):
    return subprocess.run(arguments, capture_output=True, text=True, env=ENVIRONMENT)


def write_new(path, data):
    with path.open('x') as output:
        json.dump(data, output, indent=2, sort_keys=True)
        output.write('\n')


def source_identity(filename):
    match = re.fullmatch(r'([A-Za-z0-9][A-Za-z0-9+_.-]*)-([A-Za-z0-9][A-Za-z0-9+_.~^]*)-([A-Za-z0-9][A-Za-z0-9+_.~^]*)\.src\.rpm', filename)
    if match is None:
        raise ValueError('Invalid source RPM filename')
    return match.groups()


def source_urls(filename, vendor):
    name, version, release = source_identity(filename)
    if vendor == 'Fedora Project':
        return [f'https://kojipkgs.fedoraproject.org/packages/{name}/{version}/{release}/src/{filename}']
    if vendor == 'RPM Fusion':
        distribution = re.search(r'\.fc([0-9]+)(?:\.|$)', release)
        if distribution is None:
            raise ValueError('RPM Fusion source needs an explicit Fedora release')
        number = distribution.group(1)
        root = 'https://download1.rpmfusion.org/free/fedora'
        return [f'{root}/releases/{number}/Everything/source/SRPMS/{name[0].lower()}/{filename}',
                f'{root}/updates/{number}/SRPMS/{name[0].lower()}/{filename}']
    raise ValueError('Unreviewed source-package vendor')


def make_plan(provenance):
    data = json.loads(provenance.read_text())
    sources = {}
    for package, record in sorted(data['packages'].items()):
        if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9+_.:^~-]*', package):
            raise ValueError('Invalid installed package name')
        result = command(['rpm', '-q', '--qf', '%{NEVRA}\n%{SOURCERPM}\n%{VENDOR}\n', '--', package])
        result.check_returncode()
        values = result.stdout.splitlines()
        if len(values) != 3 or values[:2] != [package, record['source_rpm']]:
            raise ValueError('Installed package differs from recorded provenance')
        filename, vendor = values[1:]
        urls = source_urls(filename, vendor)
        source = sources.setdefault(filename, {'vendor': vendor, 'urls': urls, 'binary_packages': []})
        if source['vendor'] != vendor:
            raise ValueError('Conflicting vendors for a source RPM')
        source['binary_packages'].append(package)
    if set(sources) != set(data['source_rpms']):
        raise ValueError('Source groups do not match inventoried packages')
    return {'provenance_sha256': digest(provenance), 'sources': sources,
            'limitations': 'Exact source RPM retention and digest inspection, not complete corresponding-source, static-dependency, build-environment or license clearance. Publisher signatures are recorded independently. No source script is executed.'}


def inspect_source(path, filename):
    if path.is_symlink() or not path.is_file():
        raise ValueError('Source must be a regular file')
    name, version, release = source_identity(filename)
    query = command(['rpm', '-qp', '--qf', '%{NAME}\n%{VERSION}\n%{RELEASE}\n%{SOURCEPACKAGE}\n', '--', str(path)])
    query.check_returncode()
    if query.stdout.splitlines() != [name, version, release, '1']:
        raise ValueError('Downloaded source RPM identity does not match the inventory')
    check = command(['rpmkeys', '--checksig', '--verbose', str(path)])
    output = check.stdout + check.stderr
    for kind in ['Header', 'Payload']:
        if not re.search(r'\b' + kind + r' SHA(?:256|512) digest: OK\b', output):
            raise ValueError('Source RPM header/payload digest check failed')
    if re.search(r'\b(?:BAD|NOT OK)\b', output):
        raise ValueError('Source RPM verification reported invalid material')
    signatures = [line.strip() for line in output.splitlines() if 'signature:' in line.lower()]
    return {'sha256': digest(path), 'size': path.stat().st_size,
            'header_identity': [name, version, release], 'source_package': True,
            'header_payload_digests_ok': True, 'rpmkeys_returncode': check.returncode,
            'signature_results': signatures,
            'publisher_signature_verified': check.returncode == 0 and any(line.endswith(': OK') for line in signatures)}


def retain_source(root, filename, source):
    target = root / 'sources' / filename
    receipt = root / 'receipts' / (filename + '.json')
    if target.exists() or target.is_symlink() or receipt.exists() or receipt.is_symlink():
        if target.is_symlink() or receipt.is_symlink() or not target.is_file() or not receipt.is_file():
            raise ValueError('Incomplete or symlinked cached source/receipt')
        old = json.loads(receipt.read_text())
        inspection = inspect_source(target, filename)
        if old.get('sha256') != inspection['sha256'] or old.get('url') not in source['urls']:
            raise ValueError('Cached source differs from its receipt or source plan')
        return {**inspection, 'url': old['url'], 'effective_url': old['effective_url'], 'reused': True}
    failures = []
    for url in source['urls']:
        staged = None
        try:
            with tempfile.NamedTemporaryFile(dir=root / 'sources', delete=False) as output:
                staged = Path(output.name)
            download = command(['curl', '--disable', '--silent', '--show-error', '--fail', '--location',
                                '--proto', '=https', '--proto-redir', '=https', '--max-redirs', '3',
                                '--connect-timeout', '20', '--max-time', '300', '--max-filesize', str(2 * 1024**3),
                                '--output', str(staged), '--write-out', '%{url_effective}', url])
            if download.returncode:
                failures.append({'url': url, 'curl_returncode': download.returncode})
                continue
            inspection = inspect_source(staged, filename)
            result = {**inspection, 'url': url, 'effective_url': download.stdout.strip(), 'reused': False}
            os.link(staged, target)
            write_new(receipt, result)
            return result
        finally:
            if staged is not None:
                staged.unlink(missing_ok=True)
    raise ValueError('Source download failed: ' + json.dumps(failures))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--provenance', type=Path, required=True)
    parser.add_argument('--directory', type=Path, required=True)
    parser.add_argument('--fetch', action='store_true', help='Fetch serially; otherwise write/verify the source plan only')
    args = parser.parse_args()
    plan = make_plan(args.provenance)
    root = args.directory.absolute()
    if root.is_symlink():
        parser.error('Source directory must not be a symlink')
    if root.exists():
        manifest = root / 'plan.json'
        if manifest.is_symlink() or not manifest.is_file() or json.loads(manifest.read_text()) != plan:
            parser.error('Existing directory has a different or missing source plan')
        for name in ['sources', 'receipts', 'attempts']:
            if (root / name).is_symlink() or not (root / name).is_dir():
                parser.error('Existing source subdirectories must be real directories')
    else:
        root.mkdir(mode=0o700, parents=True)
        for name in ['sources', 'receipts', 'attempts']:
            (root / name).mkdir(mode=0o700)
        write_new(root / 'plan.json', plan)
    print(f'{len(plan["sources"])} exact source RPMs planned', flush=True)
    if not args.fetch:
        return
    results = {}
    failures = {}
    for filename, source in sorted(plan['sources'].items()):
        try:
            results[filename] = retain_source(root, filename, source)
            print(f'Retained {filename}', flush=True)
        except (ValueError, OSError, subprocess.CalledProcessError) as error:
            failures[filename] = {'error': str(error)}
            print(f'Pending {filename}: {error}', flush=True)
    summary = root / 'attempts' / (uuid.uuid4().hex + '.json')
    write_new(summary, {'retained': results, 'pending': failures, 'plan_sha256': digest(root / 'plan.json')})
    print(f'{len(results)} retained; {len(failures)} pending; report {summary}')
    raise SystemExit(1 if failures else 0)


if __name__ == '__main__':
    main()
