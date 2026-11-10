# Perigee development AppImage

`scripts/build-appimage.sh` adapts the inherited qmake/linuxdeploy flow. It uses
`perigee`, the independent Linux desktop/icon/AppStream identity and explicit
`Perigee-<version>-<arch>.AppImage` artifact names. The upstream artifact update
feed remains disabled; no Perigee update service or public release is configured.

The first increment deliberately retains upstream's disabled native Wayland/DRM
build flags and libva probe/fallback strategy. This is an X11/XWayland artifact,
not primary-target native Wayland acceptance. The fallback wrapper delegates to
linuxdeploy's generated AppRun so Qt plugin hooks and library setup are preserved.
A supported host libva is preferred; fallback libraries are isolated under opt/
and enter the loader path only when the probe fails. No streaming source or core
pin is changed. The explicit SDL3 dependency is located using the loader cache
instead of assuming /usr/local/lib.

## Local build

Build dependencies remain those of the inherited AppImage workflow, plus ImageMagick
for a deterministic PNG rendering of the attributed SVG. The qmake
Linux stage is validated before bundling. Source/static dependency/font notices
are copied into the AppDir. A complete inventory and corresponding source for
bundled dynamic libraries/runtime are still required before distribution.

```bash
python3 scripts/fetch-appimage-tools.py /tmp/perigee-appimage-tools
PATH=/tmp/perigee-appimage-tools:$PATH \
  APPIMAGE_EXTRACT_AND_RUN=1 CCACHE_DISABLE=1 \
  PERIGEE_APPIMAGE_BUILD_ROOT=/tmp/perigee-appimage-build \
PERIGEE_BUILD_JOBS=1 scripts/build-appimage.sh
```

The build root must be absolute. The script refuses existing build-release,
deploy-release or installer-release outputs and never deletes prior build trees.
Choose a fresh root for each attempt. `CI_VERSION` may override the inherited
version with a filename-safe label. Arguments and paths with spaces are preserved.
`INSTALL_ROOT` stages /usr payloads without system installation.
The default compiler/LTO job count is now one. PERIGEE_BUILD_JOBS controls both;
invalid values fail before build outputs are created. This job-control adjustment
does not change optimization or rebuild the already accepted development image.

`tools.json` records official linuxdeploy/plugin/runtime release revisions, asset IDs and
SHA256 digests checked on 2026-11-10. URLs use the upstream continuous release;
if upstream replaces an asset, download verification fails closed. Cache the
reviewed tools for reproducible use, or explicitly review new tools and update
this manifest. Digest pinning does not guarantee future asset availability.
The downloader requires curl, reuses only matching tools and refuses overwrites.
No credentials are needed. CI now uses this same downloader and Perigee artifact
names; its existing dependency revisions have not been changed. No workflow was
run remotely, committed or published.

## Acceptance limits

Shell syntax and launcher fixtures verify argument preservation, host-libva choice
and fallback behavior. Downloader checks cover matching files, digest mismatches
and symlink refusal. These fixtures do not prove GPU decoding, artifact startup,
portable relocation, Qt plugin availability or compatibility with older Fedora.
Record actual build/runtime outcomes in Task 7B of the implementation plan.


## Executed local checks, 2026-11-10

Fresh release compilation and native staging pass. Bundling on Fedora 45 initially
failed because the tools' bundled strip cannot parse newer RELR sections. Retrying
only bundling/output with `NO_STRIP=1` passes; use that supported environment override
on this machine. The reviewed runtime is passed with `LDAI_RUNTIME_FILE`, avoiding
an implicit unrecorded network download. A PNG rendering and .DirIcon are generated
from the attributed SVG; no artwork was redesigned. Appimagetool still emits an
older .appdata.xml naming hint even though the installed .metainfo.xml passes
AppStream validation. No fake release entry or duplicate component is added.

Artifact: `/tmp/perigee-m7-appimage1/installer-release/Perigee-6.2.0-x86_64.AppImage`.
SHA256: `dc84dd8ce9767e958eaa428f19c739fe7f97448877a1bf969820dc1baa8e1dd6`.
Its extracted runtime `--help` passes with QT_QPA_PLATFORM=xcb; offscreen is not
bundled and an offscreen attempt fails as expected. Native Wayland is intentionally
not bundled. Actual GUI launch on XWayland maps the independent WM_CLASS, renders
the Computers view, and leaves both production profiles unchanged. Screenshot and
result JSON are in `/tmp/perigee-appimage-gui-78sleawp`; the check script is
`/tmp/perigee-m7-appimage-gui.py`. Initial capture was taken before painting; later
rendering inspection passes after a bounded settling wait. Failed retries during
temporary quota exhaustion are not passes. Completed extraction caches and the
intermediate object tree were removed to reclaim space; artifact/AppDir/logs remain.

These are development startup checks on Fedora 45, not codec/streaming, audible
audio, input, portable marker, relocation across distributions, or performance
acceptance. A live host test and Fedora 44 build/runtime matrix still need evidence.
No install, update-channel publication or AppImage release was performed.


The user subsequently confirms video, audible audio and keyboard/mouse input work
against Apollo with this development image. This closes that primary-host live
check for XWayland. Native Wayland, Fedora 44/older systems, Sunshine, portable and
controlled performance acceptance remain separate pending coverage.

Portable-mode acceptance now passes on this exact development artifact: explicit
preferences import, startup/restart, relocation through paths with spaces, identity
persistence, and ignoring Moonlight's portable.dat. Moonlight sentinel fixtures
and all three production profiles remain unchanged. Only marked test processes
are closed. The one-job resource guard records a 683802624-byte combined sample
peak (about 652 MiB), with about 8 MiB maximum sampled swap. Results/logs are in
/home/barmy/.cache/perigee-build/appimage-portable-20261110; the check is
/tmp/perigee-appimage-portable-check.py. This uses a selected working directory
containing perigee-portable.dat; merely placing a marker beside a binary does not
select portable mode from another working directory. It does not establish older
distribution or portable streaming compatibility.

The installed-tree inventory in /tmp/perigee-appimage-inventory-20261110.json
records 1873 files/symlinks and 10 notice/source records, plus the application's
direct ELF requirements. It confirms the existing app/static/font notices;
complete bundled dynamic-library/runtime notices and corresponding source remain
a release gate. Generate comparable reports with scripts/package-inventory.py;
the tool reads ELF metadata without running packaged binaries and does not certify
license clearance or discover every dynamically loaded dependency.
