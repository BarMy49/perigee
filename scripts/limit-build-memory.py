#!/usr/bin/env python3
"""Run a serial Linux build with 896 MiB RAM and 128 MiB swap limits."""
import argparse
import json
import os
from pathlib import Path
import signal
import sys
import subprocess
import time

PROPERTIES = ['MemoryHigh=640M', 'MemoryMax=896M', 'MemorySwapMax=128M']
BUILDER_PROPERTIES = ['MemoryHigh=96M', 'MemoryMax=256M', 'MemorySwapMax=0']


def process_tree(root):
    parents = {}
    for entry in Path('/proc').iterdir():
        if not entry.name.isdigit():
            continue
        try:
            fields = (entry / 'stat').read_text().rsplit(')', 1)[1].split()
            parents[int(entry.name)] = int(fields[1])
        except (OSError, ValueError, IndexError):
            continue
    owned = {root}
    previous = set()
    while previous != owned:
        previous = owned.copy()
        owned.update(pid for pid, parent in parents.items() if parent in owned)
    return owned


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--flatpak', action='store_true', help='Use the smaller Flatpak builder-parent budget')
    parser.add_argument('command', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    command = args.command[1:] if args.command[:1] == ['--'] else args.command
    if not command or args.report.exists():
        parser.error('A build command and a new report path are required')
    unit = 'perigee-limited-build-' + str(os.getpid())
    launch = ['systemd-run', '--user', '--scope', '--quiet', '--unit=' + unit]
    parent_properties = BUILDER_PROPERTIES if args.flatpak else PROPERTIES
    for value in parent_properties:
        launch += ['--property=' + value]
    process = subprocess.Popen(launch + command, start_new_session=True)
    scopes = {unit + '.scope'}
    samples = {}
    interrupted = False
    combined_peak = 0
    failure = None
    try:
        while process.poll() is None:
            for pid in process_tree(process.pid):
                try:
                    group = (Path('/proc') / str(pid) / 'cgroup').read_text().strip().split('/')[-1]
                except OSError:
                    continue
                if not (group.startswith('app-flatpak-io.github.BarMy49.Perigee-') and group.endswith('.scope')):
                    continue
                if group not in scopes:
                    result = subprocess.run(['systemctl', '--user', 'set-property', '--runtime', group] + PROPERTIES, capture_output=True, text=True)
                    if result.returncode:
                        try:
                            still_present = group in (Path('/proc') / str(pid) / 'cgroup').read_text()
                        except OSError:
                            still_present = False
                        if still_present:
                            scopes.add(group)
                            raise subprocess.CalledProcessError(result.returncode, result.args)
                        continue
                    scopes.add(group)
            combined = 0
            for scope in scopes:
                result = subprocess.run(['systemctl', '--user', 'show', scope, '--property=MemoryCurrent', '--property=MemorySwapCurrent'], capture_output=True, text=True)
                values = {}
                for line in result.stdout.splitlines():
                    key, _, value = line.partition('=')
                    if value.isdigit():
                        values[key] = int(value)
                combined += sum(values.values())
                record = samples.setdefault(scope, {'ram_sample_peak': 0, 'swap_sample_peak': 0})
                record['ram_sample_peak'] = max(record['ram_sample_peak'], values.get('MemoryCurrent', 0))
                record['swap_sample_peak'] = max(record['swap_sample_peak'], values.get('MemorySwapCurrent', 0))
            combined_peak = max(combined_peak, combined)
            if combined > 896 * 1024 * 1024:
                raise MemoryError('Combined build memory reached the safety threshold')
            time.sleep(0.2)
    except (KeyboardInterrupt, subprocess.CalledProcessError, MemoryError) as error:
        interrupted = True
        failure = str(error)
        print(failure, file=sys.stderr)
    finally:
        if interrupted or process.poll() is None:
            for scope in scopes:
                subprocess.run(['systemctl', '--user', 'stop', scope], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            try:
                os.killpg(process.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
        returncode = process.wait()
        args.report.parent.mkdir(parents=True, exist_ok=True)
        with args.report.open('x') as output:
            json.dump({'returncode': returncode, 'interrupted': interrupted, 'failure': failure, 'combined_sample_peak': combined_peak, 'properties': PROPERTIES, 'builder_properties': parent_properties, 'scopes': samples}, output, indent=2)
            output.write('\n')
    raise SystemExit(1 if interrupted else returncode)


if __name__ == '__main__':
    main()
