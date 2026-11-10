#!/usr/bin/env python3
"""Validate the exact Perigee-only Linux staged payload without installing it."""
import argparse
import re
from pathlib import Path
import subprocess
import xml.etree.ElementTree as ET

APP_ID = 'io.github.BarMy49.Perigee'
BASE = {
    'usr/bin/perigee',
    f'usr/share/applications/{APP_ID}.desktop',
    f'usr/share/icons/hicolor/scalable/apps/{APP_ID}.svg',
    f'usr/share/metainfo/{APP_ID}.metainfo.xml',
    'usr/share/doc/perigee/LICENSE',
    'usr/share/doc/perigee/ASSET_NOTICES.md',
}
LICENSES = (
    'LICENSE', 'ModeSeven-README', 'moonlight-common-c/moonlight-common-c/LICENSE.txt',
    'moonlight-common-c/moonlight-common-c/enet/LICENSE',
    'moonlight-common-c/moonlight-common-c/nanors/LICENSE',
    'qmdnsengine/qmdnsengine/LICENSE.txt', 'h264bitstream/LICENSE',
    'app/SDL_GameControllerDB/LICENSE',
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('stage', type=Path)
    parser.add_argument('--rpm', action='store_true')
    args = parser.parse_args()
    stage = args.stage.resolve(strict=True)
    if stage == Path('/'):
        parser.error('A staging directory is required')
    expected = set(BASE)
    if args.rpm:
        expected.update('usr/share/licenses/perigee/' + name for name in LICENSES)
        expected.update('usr/share/doc/perigee/' + name for name in ('THIRD_PARTY.md', 'SOURCE_SNAPSHOT.json'))
    entries = list(stage.rglob('*'))
    for path in entries:
        if path.is_symlink():
            name = path.relative_to(stage).as_posix()
            if not (args.rpm and re.fullmatch(r'usr/lib/\.build-id/[0-9a-f]{2}/[0-9a-f]{38}', name)
                    and path.resolve() == stage / 'usr/bin/perigee'):
                parser.error('Unexpected symlink in payload')
    actual = {path.relative_to(stage).as_posix() for path in entries if path.is_file() and not path.is_symlink()}
    if actual != expected:
        parser.error(f'Unexpected files: {sorted(actual - expected)}; missing: {sorted(expected - actual)}')
    binary = stage / 'usr/bin/perigee'
    if not binary.stat().st_mode & 0o111 or binary.read_bytes()[:4] != b'\x7fELF':
        parser.error('Expected an executable ELF client')
    desktop = stage / f'usr/share/applications/{APP_ID}.desktop'
    fields = dict(line.split('=', 1) for line in desktop.read_text().splitlines() if '=' in line)
    for name, value in dict(Name='Perigee', Exec='perigee', Icon=APP_ID, StartupWMClass=APP_ID).items():
        if fields.get(name) != value:
            parser.error(f'Incorrect desktop {name}')
    component = ET.parse(stage / f'usr/share/metainfo/{APP_ID}.metainfo.xml').getroot()
    if component.findtext('id') != APP_ID or component.findtext('launchable') != APP_ID + '.desktop':
        parser.error('AppStream identity mismatch')
    subprocess.run(['desktop-file-validate', str(desktop)], check=True)
    subprocess.run(['appstreamcli', 'validate', '--no-net', str(stage / f'usr/share/metainfo/{APP_ID}.metainfo.xml')], check=True)
    print(f'PASS: {len(actual)} Perigee-only staged files; no configuration, credentials, or Moonlight payload')


if __name__ == '__main__':
    main()
