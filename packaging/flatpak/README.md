# Perigee development Flatpak

This is a development prototype. A local bundle, private install/remove/reinstall,
sandbox policy/import and native Wayland startup checks pass. The user accepts
Apollo video/audio/keyboard/mouse on the incremental prototype. A separate clean
corrected-manifest build and isolated runtime smoke checks now pass; live-host
acceptance for that separate artifact remains pending.
The independent application ID is io.github.BarMy49.Perigee;
settings remain inside its Flatpak namespace and are never shared with Moonlight.

## Source and dependencies

`io.github.BarMy49.Perigee.json.in` uses KDE Platform/SDK 6.11. All additional Git
sources have full immutable commit IDs. SDL3, SDL2-compat, SDL_ttf, libplacebo and
upstream FFmpeg versions follow the existing AppImage build choices. dav1d is
explicitly pinned to 1.5.4 for software AV1 decoding. Dependency root notices are
copied to /app/share/licenses/perigee/dependencies; application/static dependency
and font notices are retained separately. Full source/license release inventory
remains pending. No Common C/submodule pin or media source is changed.

The packaging review consulted the upstream Flathub Moonlight manifest at
[50797df452b4044a8ee66371ab60b3be378bbfdc](https://github.com/flathub/com.moonlight_stream.Moonlight/blob/50797df452b4044a8ee66371ab60b3be378bbfdc/com.moonlight_stream.Moonlight.json).
No gamescope layer, host-OS access, device=all permission or custom FFmpeg fork
from that manifest was imported. Build descriptions are composed for Perigee.

Create a complete source snapshot with scripts/create-source-archive.py, explicitly
including required untracked sources as documented in the RPM workflow. Then:

```bash
scripts/build-flatpak.sh /absolute/perigee-source.tar.gz /absolute/new-output-root
```

The output root must be new. Use a cache directory outside the IDE project tree:
SDK/source caches contain many files and must not be indexed as project sources.
The builder requires the KDE SDK to be available and never installs an application.
`prepare-flatpak.py` validates all snapshot hashes and exact archive membership,
rejects nonregular/unsafe/duplicate paths, and refuses to overwrite its output.
Builds use the standard Flatpak sandbox; missing required media/Wayland pkg-config
interfaces fail explicitly instead of silently reducing qmake features.

## Permission contract

Permissions are limited to network, Wayland/fallback-X11, IPC, PulseAudio, DRI and
input devices, read-only /run/udev and the specific ScreenSaver D-Bus name.
Runtime permissions are not inherited. Flatpak 1.17+ is required. There is no home,
host, host-os, general configuration/cache sharing, SSH agent or unrestricted
session/system bus access. Graphics/input/network/audio functionality still needs
actual sandbox acceptance.

There is no GUI import chooser in the current product. A future explicit file
forwarding/document-portal check must prove that a selected INI can be imported
without broad filesystem access. Pairing stays excluded by default. Do not weaken
permissions or copy production credentials merely to make an acceptance check pass.
Browser/SFTP dispatch through desktop portals and user-consented file forwarding
also remain unverified. No host is paired/launched by this prototype.

## Interrupted local build, 2026-11-10

A private SDK installation was downloaded without touching system/normal-user
Flatpak installations. SDK commit:
f77a65229fcb9dc1468944cd6a2b5aa327d1877725d681467805dae6ed68aebe.
Runtime commit:
fb23e178d082b117df7924f9a84710992ec233ebc007385feb85d349c4c81415.
Manifest parsing and source-integrity/no-overwrite/permission checks pass.
The sandbox compiled libplacebo, then reached SDL3 object 164/369 before the IDE
crash terminated build processes. No compiler remains running and no bundle exists.

The machine had about 630 MiB available and 6.6 GiB swap used when inspected.
CLion logs mention the generated Flatpak project directory repeatedly, but neither
kernel/systemd-oomd logs nor targeted JVM markers proved an OOM crash. Do not state
a confirmed crash cause. No IDE settings or heap limits were changed.
Generated files were moved by same-filesystem rename, without bulk copying, to
`/home/barmy/.cache/perigee-build/flatpak-20261110` to avoid project indexing.
Sources, SDK, cache and build log are retained. A relocated-source manifest is
`io.github.BarMy49.Perigee-resume.json` in that directory.

After memory pressure has subsided, resume with one build job:

```bash
flatpak_root=/home/barmy/.cache/perigee-build/flatpak-20261110
FLATPAK_USER_DIR="$flatpak_root/installation" GIT_TERMINAL_PROMPT=0 \
  flatpak-builder --sandbox --disable-rofiles-fuse --disable-updates --jobs=1 \
  --state-dir="$flatpak_root/state" --repo="$flatpak_root/repository" \
  "$flatpak_root/app-resume" "$flatpak_root/io.github.BarMy49.Perigee-resume.json"
```

The command was subsequently run inside the resource-limited scope below. Use a fresh app directory if an attempt already
exists. Application installation, native rendering, GPU/audio/input/streaming,
portal/file forwarding, Moonlight coexistence and cleanup remain pending.


## Memory-limited resumption

After the user freed memory, the cached build was resumed with one job under
`systemd-run --user --scope` with `MemoryHigh=640M`, `MemoryMax=896M` and
`MemorySwapMax=128M`. The combined RAM/swap limit is 1 GiB. The scope name is
`perigee-flatpak-resume-20261110`; `systemctl --user show` provides live accounting.
This is a temporary build scope, not an installed service or a global IDE setting.
The new log is `resume-build.log` in the external build root. No other compilation
runs concurrently. The application make-job fallback is now also one.

The notice audit identified statically linked vendored FreeType at SDL_ttf's
submodule pin `535d2993d2cab67cc8b270132b1b1bcb62f269aa`. The corrected template
copies LICENSE.TXT, FTL.TXT, GPLv2.TXT and a FreeType attribution. These same files
were staged manually into the running prototype before export; details are in
`freetype-notices-staging.txt`. This notice-only adjustment does not change media
code. A fresh build of the final corrected manifest is not yet claimed.


## Completed prototype checks

The one-job dependency build completed, then stopped before application compilation
because KDE's SDK names its Qt 6 executable qmake, not Fedora's qmake6. The template
now checks the Qt major version and uses qmake. The retry did not reuse dependency
cache as expected, so it was stopped. The application was built directly against
the completed SDK/dependency root with flatpak build, then finalized/exported with
flatpak-builder --finish-only. This is incremental prototype validation, not a clean
build of the final corrected manifest. Logs retain each attempt.

Application-only commands are in state/build/perigee-1/build-perigee-prototype.sh;
application-only.log and finish.log contain compile/install/export results.
The enclosing systemd scope did not follow the direct Flatpak invocation when it
created a new scope. The same limit was applied to the verified compiler scope,
app-flatpak-io.github.BarMy49.Perigee-3797339081.scope. Observed application build
peak: 671576064 bytes (about 640 MiB), with about 2 MiB swap. Do not assume that an
enclosing scope automatically limits a separately scoped Flatpak process.

Bundle: /home/barmy/.cache/perigee-build/flatpak-20261110/Perigee-6.2.0-x86_64.flatpak
SHA256: 4ca4108b57c945f6e9f5130da6c9eb1080bce91c3e278d3d732892840e2f3f28
Local app commit: 0a21bcbb9939d1208a2bc38e4691442d95c94203392844fe273b5855b70ffdd5.
No normal-user/system application installation or public publication occurred.
The app remains installed only in the private SDK/test installation for acceptance.

Runtime checks pass: shared libraries resolve; native Perigee/Moonlight settings
are hidden; direct access to a host-side synthetic INI is denied; explicit document
portal file forwarding imports its four preferences (1600×900, 60 FPS, 15000 kbps).
The fixture export is revoked afterward. Both production profile hashes remain
unchanged. This tests explicit CLI file selection, not a GUI file chooser or
pairing import. Results/logs: runtime-policy-results.json, denied-import.log and
forwarded-import.log; script: /tmp/perigee-flatpak-runtime-checks.py.

Native Wayland startup maps the expected app ID and receives compositor configure,
with no QML load failure and unchanged production profiles. The GUI test is closed;
results/logs are in gui-27f1gjcp and the script is /tmp/perigee-flatpak-gui-check.py.
It applies the memory limit before starting Qt. GUI peak: 493420544 bytes (about
471 MiB), zero swap. No visual rendering, video decode, audible audio, physical
input or streaming acceptance is inferred from window configuration alone.
Private remove/reinstall also passes, preserving app data and the same app commit.
No native Moonlight package files or pairing state are changed.

For the user-supervised live check:

```bash
FLATPAK_USER_DIR=/home/barmy/.cache/perigee-build/flatpak-20261110/installation \
  flatpak --user run --branch=devel --env=QT_QPA_PLATFORM=wayland \
  --env=SDL_VIDEODRIVER=wayland io.github.BarMy49.Perigee
```

This uses the Flatpak's separate profile. Video/audio/input and representative
GPU/portal/coexistence coverage still need user/live evidence. RPM upgrade, clean
Fedora 44 builds, full source/license inventory and remote CI remain release gates.


The user now confirms video, audible audio and keyboard/mouse input work in this
native Wayland Flatpak against Apollo. That primary-host check is accepted by user
report for this prototype. It does not close ordinary Sunshine, clean-manifest,
Fedora 44 or controlled performance coverage.


A clean build of the corrected manifest passed with the
[resource guard](../BUILD_RESOURCES.md), a fresh build/cache output and only reused
downloaded source mirrors. There is no manual notice staging or application-only
build command in this verification. The initial guarded attempt stopped before
completion at its conservative aggregate threshold; the identical-manifest retry
used a smaller builder-parent cache budget and completed compile/install/export.
Its highest aggregate sample was 909049856 bytes (about 867 MiB). Compilation
used one job; optimization and media code were unchanged.

Clean output root: /home/barmy/.cache/perigee-build/flatpak-clean-20261110.
Logs: build.log (guarded stop), build-retry.log (pass), memory-retry.json and
bundle-memory.json. Bundle: Perigee-6.2.0-x86_64.flatpak; SHA256:
18f844ba2ed2ecc8e8764efc7d7af9865f293518243f1317cc52ab3c4b3391fb.
The source snapshot SHA256 is
7653b343396e20587db205aca1fc668a7fcdbb12cbd022f09c0556907250d77d.
The installed-tree inventory records 2108 files/symlinks and 39 notice/source
records, including all four automatically installed FreeType texts/attribution.
This inventory is not complete license clearance or corresponding-source publication.

Application source/resource files match the accepted prototype, but the application
ELF differs. Its prior live pass does not automatically validate this new artifact.
The accepted private installation and its app data remain intact.

The clean bundle installs into its own private installation and passes the same
permission contract, shared-library closure, hidden-native-profile and selected-file
portal import checks. Its app commit is
079814343d48c38d7046a4f1c9c11561854d507f5c0d5364ea1f97871fae6d6f.
The synthetic export is confirmed revoked. Native Wayland startup receives the
correct app ID/configure without QML load failure; peak 463527936 bytes (about
442 MiB), zero swap. Test processes close, and hashes of native Perigee, Moonlight
and the user's existing Flatpak profile are unchanged. No live host is contacted.
Results: runtime-policy-results.json, permissions-comparison.json and
gui-_pffjho0/result.json in the clean output root. Scripts are
/tmp/perigee-flatpak-clean-runtime-checks.py and
/tmp/perigee-flatpak-clean-gui-check.py.

This check caught a harness issue: Flatpak resets XDG_CONFIG_HOME supplied through
flatpak run --env. Two import attempts safely refused the existing destination.
The corrected harness exports disposable XDG paths inside the sandbox shell,
before starting Qt. The prior prototype's GUI-profile-isolation claim is therefore
withdrawn; native-profile isolation still passed. Do not copy that older harness
as a disposable-profile example. The corrected clean checks preserve all three
profiles and use no credentials. Window configuration is a startup smoke check,
not visual rendering or live video/audio/input acceptance for this artifact.


The user now also confirms Apollo video, audible audio and keyboard/mouse input
work in the clean-built bundle launched from its separate private installation.
This accepts the primary-host live check for bundle SHA256
18f844ba2ed2ecc8e8764efc7d7af9865f293518243f1317cc52ab3c4b3391fb.
The capped launch command was provided with report path
/tmp/perigee-clean-flatpak-live-memory.json. Decoder-specific, Sunshine, KDE,
HDR/controller and controlled performance coverage remain separate follow-ups.


## Approved-icon refresh and wrapper fix

The user approved the original orbit icon. A branded refresh is building at
/home/barmy/.cache/perigee-build/flatpak-branding-20261111 with unchanged SDK/runtime
pins, one job/CPU and the aggregate memory guard. The interrupted attempt stopped
with SIGTERM during FFmpeg preparation; resume results are recorded separately.

The build wrapper now copies the verified input archive into output-root/sources/
before preparing its manifest. Flatpak's --sandbox requires the source to remain
inside that directory tree. The separate sources directory prevents input filenames
from colliding with io.github.BarMy49.Perigee.json or sdk-commit.txt. Updates stay
disabled. Real-archive fixtures cover spaces, both metadata-name collisions,
unchanged input bytes, existing-output rejection and builder failure propagation.


The branded refresh now passes build/export/bundle and isolated runtime smoke
checks. Highest combined memory sample: 934137856 bytes (about 891 MiB), one job.
Bundle: /home/barmy/.cache/perigee-build/flatpak-branding-20261111/Perigee-6.2.0-orbit-x86_64.flatpak.
SHA256: 0263075f954addc5f14f03b5a18d7fa98b3c0a9f864fe1ce76693cc72fd16746.
Approved icon equality, library closure, native-profile isolation, selected-file
portal import and native Wayland startup pass with unchanged production profiles.
It is installed only in its separate private test installation; no accepted app
installation was replaced. Live streaming of this branded binary is unreported.
