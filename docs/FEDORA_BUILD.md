# Fedora development, baseline, and packaging

Target selected by the user: Fedora 44 GNOME Wayland first, KDE Plasma Wayland
second; X11/XWayland compatibility where available. Use current packaged Qt 6 and
the pinned [source/dependency manifest](REPOSITORY_ANALYSIS.md#inspected-revisions).
These Bash commands are source-checked instructions. Fedora preflight has run;
compilation and session acceptance remain pending.

## Verification status

| Check | Result as of 2026-10-25 |
| --- | --- |
| Source/build/submodule inspection | Executed; qmake/C++17/Qt Quick structure and exact pins recorded |
| Fedora/RPM Fusion packages | Corrected dependency set installed by user; RPM and pkg-config checks passed |
| Windows tools | `cmake.exe`, `wsl.exe`, `docker.exe` found; qmake/qmake6 and queried GNU/Clang compiler commands absent |
| Docker Linux engine | `docker version --format '{{json .Server}}'` could not connect to `dockerDesktopLinuxEngine`; no container build possible through that daemon |
| WSL distributions | `wsl --list --quiet` listed only `docker-desktop`; no usable Fedora desktop/toolchain established |
| Fedora preflight | Fedora 44 GNOME Wayland available; required development packages now installed; see result record |
| Fedora compile/debug/release | Debug and release qmake/make passed; both binaries have debug symbols and no missing linked libraries |
| Isolated Wayland startup/restart | Two 15-second release runs; exit 0 after SIGTERM; native identity fields persisted; checked Moonlight native files unchanged |
| Pairing/streaming/input/audio/GPU session tests | Not executed; real Windows Apollo and standard Sunshine hosts required |
| GNOME/KDE/X11, HDR/VRR, packaging/performance | Not executed |

The user authorized starting Milestone 1 on 2026-10-25 with pairing tests
skipped; see the [recorded exception](DECISIONS.md#adr-008-user-directed-implementation-gate-exception-2026-10-25).
Baseline session/performance acceptance remains unverified. Task 1A native
identity and Task 1B portable/desktop evidence are in the
[plan](IMPLEMENTATION_PLAN.md#task-1b-complete-desktop-and-portable-identity). A later container compile can validate dependencies/builds but cannot
replace physical GPU/compositor/host acceptance. Do not start Docker Desktop or
provision a replacement system merely to hide an unavailable baseline.

## Source and build requirements

- Root `moonlight-qt.pro`: subdir qmake projects and `qtCompileTest(SL/EGL)` probes.
- `app/app.pro`: C++17; Qt core, quick, network, quickcontrols2, svg; pkg-config
  SDL/OpenSSL/Opus, FFmpeg and optional Linux render backends.
- `globaldefs.pri`: debug/release targets, debug symbols and optional sanitizer flags.
- FFmpeg 4.0+ is documented for the base client. Upstream documents FFmpeg 6.1+
  and libplacebo 7.349.0+ for Vulkan rendering; verify actual feature compilation.
- Linux README recommends Qt 6 and advertises Qt 5.12+ compatibility; Perigee's
  initial validation is Qt 6. No Qt 5 build has been verified here.
- Recursive submodules include Common C with ENet/nanors, qmdnsengine, and the SDL
  controller database. Windows/macOS dependency setup scripts are not Linux setup.

Primary evidence: [upstream README](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/README.md),
[app.pro](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/app.pro#L64),
[global flags](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/globaldefs.pri).

Fedora 44 provides the SDL2 development API through
[`sdl2-compat-devel`](https://packages.fedoraproject.org/pkgs/sdl2-compat/sdl2-compat-devel/),
backed by SDL3. Do not interpret that as a reason to rewrite the client's SDL API.
[`qt6-qtdeclarative`](https://packages.fedoraproject.org/pkgs/qt6-qtdeclarative/qt6-qtdeclarative/fedora-44.html)
provides the relevant QtQuick Controls/Material/Templates/Layouts/Window and QtQml
modules; avoid stale separate Qt6 Controls package instructions.

## Package installation

Use a normal Fedora 44 development environment. Atomic variants need their own
container/layering workflow and are not validated by these host DNF instructions.
Inspect existing repositories/package choices first:

```bash
cat /etc/fedora-release
dnf repolist
```

If RPM Fusion Free is not already configured, its source-documented setup is:

```bash
sudo dnf install \
  "https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm"
```

Reference: [Fedora Magazine setup](https://fedoramagazine.org/migration-guide-from-windows-to-fedora-linux/).
The accessible [Fedora 44 RPM Fusion index](https://download1.rpmfusion.org/free/fedora/updates/44/x86_64/repoview/ffmpeg-devel.html)
lists `ffmpeg-devel`. The main RPM Fusion configuration page and Fedora Quick Docs
were blocked by access protection during research. Nonfree is not needed simply
to obtain FFmpeg development files; proprietary driver setup is separate.

Full development set; dependency resolution and installation checked on 2026-10-25:

```bash
sudo dnf install \
  git gcc-c++ make pkgconf-pkg-config \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel qt6-qtwayland \
  sdl2-compat-devel SDL2_ttf-devel openssl-devel opus-devel ffmpeg-devel \
  libva-devel libvdpau-devel libdrm-devel libplacebo-devel \
  wayland-devel wayland-protocols-devel libxkbcommon-devel libX11-devel \
  'pkgconfig(egl)' 'pkgconfig(gl)' \
  pulseaudio-libs-devel alsa-lib-devel \
  gdb valgrind perf libva-utils vulkan-tools
```

The pkg-config providers avoid assuming a particular EGL/OpenGL development
subpackage name. The set includes optional rendering/diagnostic support, not only
the minimal link requirements. Use `dnf repoquery --whatprovides 'pkgconfig(sdl2)'`
and analogous queries if package names/providers change. Record that resolution.

Fedora FFmpeg-free/full RPM Fusion choices may conflict. Inspect the transaction
and installed packages rather than silently adding `--allowerasing` or replacing
the user's multimedia stack. A conflict is an environment finding to document.

### Dependency transaction preflight, 2026-10-25

`dnf repolist` confirmed Fedora/updates and RPM Fusion Free/updates are already
enabled; no repository setup is needed. `rpm -qa 'ffmpeg*' 'libav*-free*'
'rpmfusion*'` found full RPM Fusion `ffmpeg` and `ffmpeg-libs` version
`8.1.3-1.fc44`, so no multimedia-stack replacement is proposed.

The initial full-list `dnf install --assumeno` exited 1 because
`wayland-protocols` has no match. `dnf repoquery --available 'wayland-protocols*'`
exited 0 and identified `wayland-protocols-devel`. The corrected dry-run was:

```bash
dnf install --assumeno \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel \
  SDL2_ttf-devel openssl-devel opus-devel ffmpeg-devel \
  libva-devel libvdpau-devel libplacebo-devel wayland-protocols-devel \
  pulseaudio-libs-devel alsa-lib-devel valgrind perf
```

It resolved 30 new packages including dependencies, 27 MiB download and 145 MiB
installed size, with no removals or upgrades, then exited 1 as expected for
`--assumeno`. The resolver selected Qt development packages 6.11.2, FFmpeg
8.1.3, libplacebo 7.360.1 and libva 2.23.0. This is dependency resolution evidence,
not an installation or compilation pass.

**Historical installation blockers (resolved by user):** `sudo -n true` exited 1 (`a password is required`).
DNF also warned that this transaction would invalidate a pending offline
transaction initiated by `dnf5daemon-server`. No privileged installation was
attempted and the offline transaction was not changed. The user must complete or
resolve that offline transaction before installing the above packages with
`sudo dnf install` in their terminal. Recheck the proposed transaction at that
time; do not add `--allowerasing`. Then resume the documented debug/release builds.

## Baseline collection helper

**Checkout discrepancy (2026-10-25):** `scripts/collect-fedora-baseline.sh` is
absent and `git ls-files scripts/collect-fedora-baseline.sh` returns no entry.
The helper description below records the earlier handoff, not an available command.
Use the manual build/run instructions until the helper is restored and validated.

From a checkout containing the helper on Fedora 44:

```bash
bash scripts/collect-fedora-baseline.sh
```

The collector writes a fresh owner-only report/profile directory beneath
`${XDG_STATE_HOME:-$HOME/.local/state}/perigee-baseline/`. It records source/submodule
revisions, working-tree state, desktop/session type, package/tool versions and
bounded VA-API/Vulkan probes. Each diagnostic has its exit status; a missing tool
does not become a passing check. No credentials or streaming logs are collected.
Successful collection means only that the report was written.

After installing dependencies and initializing the recorded submodules, use:

```bash
bash scripts/collect-fedora-baseline.sh --build
```

This opt-in verifies tracked app/build/dependency content against the frozen
reference commit and requires matching recursive submodule revisions. Documentation-
only commits are allowed; the report records both actual HEAD and reference SHA.
It builds debug and
release into fresh report-local directories, saves qmake/make logs, and checks for
the expected upstream binary. It never fetches, installs, deletes old build trees,
or launches a session. The generated `baseline.md` contains a manually runnable
release command using fresh XDG/TMPDIR state and a checklist for actual acceptance.
Run it from the reported private working directory, as that command specifies.

`--help` works on other platforms; collection/build intentionally reject non-Fedora
44 environments. Bash syntax and command/error handling can be checked from Git
Bash on Windows, but those checks do not verify Fedora collection or compilation.

## Reproducible debug and release builds

From the Perigee source directory, verify the baseline and initialize recorded pins:

```bash
git status --short --branch
git rev-parse HEAD
git submodule update --init --recursive
git submodule status --recursive
qmake6 -query QT_VERSION
pkg-config --modversion sdl2 SDL2_ttf openssl opus libavcodec libavutil libswscale
pkg-config --modversion libva libplacebo wayland-client
```

Do not use `git submodule update --remote`. Keep unmodified upstream baseline and
later Perigee measurements in separate build/configuration directories.

Debug build, starting at the source root (its directory name is `perigee`):

```bash
mkdir -p ../perigee-build/debug
cd ../perigee-build/debug
set -o pipefail
qmake6 ../../perigee/moonlight-qt.pro 2>&1 | tee qmake.log
make -j"$(nproc)" debug 2>&1 | tee build.log
```

Inspect qmake output for FFmpeg, VA-API/EGL, libplacebo/Vulkan and Wayland features;
optional dependencies can silently remove a path even when the build succeeds.
Current executable is `app/perigee`. The frozen baseline builds below
produced `app/moonlight`; preserve them for later comparisons. Do not rebuild the
frozen release directory with changed source when retaining baseline evidence.

For release, start again at the source root:

```bash
mkdir -p ../perigee-build/release
cd ../perigee-build/release
set -o pipefail
qmake6 ../../perigee/moonlight-qt.pro 2>&1 | tee qmake.log
make -j"$(nproc)" release 2>&1 | tee build.log
```

Use release builds for performance comparisons. Upstream requests debug information
for both build modes. Do not modify `globaldefs.pri` to obtain ordinary debug symbols.

## Isolated baseline run

Before identity isolation, do not run against Moonlight's real configuration. From
the debug/release build directory, create a private disposable baseline profile:

```bash
baseline_root="$HOME/.local/state/perigee-baseline"
umask 077
mkdir -p "$baseline_root"/{config,cache,data,tmp}

env \
  XDG_CONFIG_HOME="$baseline_root/config" \
  XDG_CACHE_HOME="$baseline_root/cache" \
  XDG_DATA_HOME="$baseline_root/data" \
  TMPDIR="$baseline_root/tmp" \
  QT_QPA_PLATFORM=wayland \
  SDL_VIDEODRIVER=wayland \
  ./app/moonlight
```

Do not run this from a directory containing upstream `portable.dat`. Do not change
`XDG_RUNTIME_DIR`: it identifies the real compositor/runtime sockets. Isolating
TMPDIR also separates upstream's Moonlight-prefixed temporary log cleanup.

Fresh baseline credentials identify a new Apollo client. Arrange the needed host
permissions in its admin interface; never unpair or overwrite the production
Moonlight client to complete a test. Pairing presence alone is not input permission.
Do not capture keys/PINs/passphrases or sensitive clipboard text in shared logs.

After Milestone 1A use `./app/perigee` with disposable data until coexistence passes.
Do not run `sudo make install` for the upstream-branded baseline.

## Debugging and runtime dependencies

In the same isolated environment, use the real streaming-capable build:

```bash
env \
  XDG_CONFIG_HOME="$baseline_root/config" \
  XDG_CACHE_HOME="$baseline_root/cache" \
  XDG_DATA_HOME="$baseline_root/data" \
  TMPDIR="$baseline_root/tmp" \
  QT_QPA_PLATFORM=wayland \
  SDL_VIDEODRIVER=wayland \
  gdb --args ./app/moonlight
```

Useful checks: `vainfo`, `vulkaninfo --summary`, `ldd ./app/moonlight`,
`rpm -q qt6-qtbase qt6-qtdeclarative sdl2-compat ffmpeg-libs libva libplacebo`, and
recorded runtime decoder/renderer statistics. `ffmpeg -hwaccels` describes compiled
capabilities, not successful acceleration on the actual GPU.

Runtime requires matching Qt GUI/QML/Wayland plugins, SDL compatibility/runtime,
SDL_ttf, OpenSSL, Opus, selected FFmpeg libraries, and the renderer/driver stack
actually compiled/selected. VA-API needs an appropriate GPU/codec driver; Vulkan
needs a working loader/ICD and supported decode/presentation capabilities. Do not
install an Intel/AMD/NVIDIA driver based only on a guessed GPU. Collect vendor,
device, driver, monitor and session information first.

Diagnostic-only environment settings:

| Variable | Use and caution |
| --- | --- |
| `QT_QPA_PLATFORM=wayland`, `SDL_VIDEODRIVER=wayland` | Coordinated native-Wayland test; confirm actual selected backends |
| `QT_QPA_PLATFORM=xcb`, `SDL_VIDEODRIVER=x11` | Separate X11/XWayland comparison, not an automatic Wayland fix |
| `QT_DEBUG_PLUGINS=1`, `QT_LOGGING_RULES='qt.qpa.*=true'` | Plugin/backend diagnosis; logs need review/redaction |
| `PREFER_VULKAN=1` | Existing upstream renderer-preference diagnostic; not an initial product default |
| `LIBVA_DRIVER_NAME`, `LIBVA_DRIVERS_PATH` | Only for a documented driver issue; do not force names globally |
| `QSG_RENDER_LOOP` | Upstream sets basic rendering for interoperability; do not force threaded rendering |

GDB stacks/core dumps, `perf`, and Valgrind can investigate failures/resource use.
Sanitizer builds are a separate diagnostic configuration, not performance baselines;
record any GPU-driver limitations rather than treating all reports as app defects.
Do not alter the application to work around an unexamined environment failure.

## Wayland, X11, and portals

- Test GNOME and KDE separately. Keyboard-shortcut inhibition, reserved compositor
  shortcuts, relative pointer/capture, focus, decoration and scaling differ.
- Preserve an emergency local ungrab/exit action. Release keys on transitions; do
  not claim every Windows shortcut can bypass compositor/security policy.
- The focused SDL window and Qt UI can use separate Wayland connections. Use the
  [owner-thread/session design](ARCHITECTURE.md#session-lifecycle-and-threading).
- Clipboard portal support requires an authorized compatible session and actual
  compositor/backend support. It does not grant generic background clipboard access.
- PipeWire can support SDL audio/portal workflows; no Linux screen capture is
  required to receive GameStream. Do not add a capture service to solve presentation.
- HDR/VRR need hardware, monitor, driver and compositor evidence. X11 success does
  not imply Wayland success; a container build proves neither.

## Packaging strategy

1. Native uninstalled build for baseline; no installed-file collision.
2. After complete identity metadata, stage a `perigee` RPM manifest using qmake's
   `PREFIX`, `BINDIR`, `DATADIR`, and `INSTALL_ROOT` support. Do not add Moonlight
   conflicts/obsoletes. No native RPM spec was found in the inspected client tree.
3. Adapt existing `.github/workflows/build-appimage.yml` and
   `scripts/build-appimage.sh` for Perigee identity. The workflow builds SDL3 plus
   sdl2-compat and graphics dependencies; it is not evidence that Fedora drivers
   or all host hardware work inside an AppImage.
4. Create an independent Flatpak manifest/ID after reviewing the then-current
   upstream packaging source and runtime. No client Flatpak manifest was found in
   this tree. Verify graphics extensions, Wayland/X11, audio, gamepad/network
   permissions, file selection and configuration isolation; do not copy a stale
   Artemis manifest or broadly expose the home directory.

Future staging example, only after Milestone 1B, from a dedicated build directory:
configure the app subproject explicitly with `PREFIX=/usr` (the top-level
invocation did not propagate this variable in the observed build), then use its
selected build-mode Makefile with `INSTALL_ROOT` as shown below.
Validate the Perigee `.desktop` with `desktop-file-validate` and the planned
`io.github.BarMy49.Perigee.metainfo.xml` with `appstreamcli validate`. Task 1B staging and metadata checks have now run; distributable packages remain
unimplemented. See [branding](BRANDING.md).

## Current Perigee staging and portable checks

Task 1B is implemented; detailed verification is in the
[Task 1B result](IMPLEMENTATION_PLAN.md#task-1b-complete-desktop-and-portable-identity).
The test uses a debug artifact; the frozen baseline release build remains intact.
No system installation was performed. From `../perigee-build/debug/app`, the
executed staging commands were:

```bash
qmake6 ../../../perigee/app/app.pro PREFIX=/usr
make -f Makefile.Debug -j2
make -f Makefile.Debug INSTALL_ROOT=/home/barmy/Projects/perigee-build/task1b-stage-4ce4q2ot install
```

`desktop-file-validate` and `appstreamcli validate-tree --no-net <stage>/usr`
passed. Six staged files have no path collisions with installed `moonlight-qt`.
AppStream advisory notes cover the selected mixed-case ID and missing release
history. Temporary icon attribution/license material is staged with the binary.
Original artwork, distributable packages, system launcher/icon-cache acceptance
and visual shell grouping remain pending.

Linux portable mode requires `perigee-portable.dat` in the launch working directory
and writes only its own `perigee-data/{config,cache,boxart,qmlcache,logs}` tree.
Settings are `perigee-data/config/io.github.BarMy49/Perigee.ini`. Moonlight's
`portable.dat` is ignored; no settings or pairing identity is imported. Linux
continues logging to stderr rather than enabling persistent file logs.

Reopen either disposable tested profile with:

```bash
/home/barmy/.local/state/perigee-baseline/task1b-20261025-7jcnowps/launch-native.sh
/home/barmy/.local/state/perigee-baseline/task1b-20261025-7jcnowps/launch-portable.sh
```

Native/portable restart, separate identities, untouched Moonlight state, process
coexistence, Wayland app ID and XWayland properties passed. Linux artifact checks
are disabled even for AppImage/Steam Link builds; mapping/compatibility data,
connectivity/STUN and upstream help endpoints are retained. No live stream,
permission, audio/input or performance acceptance follows from these checks.

## Explicit import verification

Task 1C adds the Linux command:

```bash
perigee import-moonlight --source PATH
perigee import-moonlight --source PATH --include-pairing
```

These are alternatives for a fresh destination, not sequential operations on the
same profile. Source must be an explicit Moonlight INI settings file. Default import
excludes the client tuple and first startup creates new credentials. Optional
identity import shares host-side permissions/identity and unpairing effects with
Moonlight; no host authentication is implied. Existing destination files (including
empty ones), unsafe directories and in-use profiles are refused.

Use the current debug `app/perigee` for Task 1C; the previous Task 1B staged binary
and its launchers are historical artifacts and do not include the importer/profile
lock. Nothing was installed system-wide. For isolated checks, use fresh XDG/TMPDIR
roots; keep the real `XDG_RUNTIME_DIR` for supervised Wayland startup. Native and
portable destinations follow normal marker/identity selection.

The focused Qt target is documented in
[tests/identity-import](../tests/identity-import/README.md). qmake/make and all
**32 fixture checks passed**. CLI tests deliberately have no valid GUI platform.
Atomic publication requires Linux anonymous files and `/proc/self/fd`; unsupported
filesystems fail without publishing settings. The test target is independent of
the application build and uses synthetic credentials only.

Three disposable imports and six eight-second Wayland startup/restart runs passed.
Optional certificate/key byte values and client IDs were verified with QSettings;
Qt reformats INI representation, so raw line comparisons are not a credential-byte
comparison. Default import produced a new identity instead. Active-profile import
and second startup were refused; source bytes/mode and 512 checked production
files stayed unchanged. Private result:
`/home/barmy/.local/state/perigee-baseline/task1c-20261025-01q1rd_5/result.json`.
Detailed build commands, fixture results and limits are in
[Task 1C](IMPLEMENTATION_PLAN.md#task-1c-explicit-import). Real host pairing and
imported authentication tests remain skipped by user direction.

## Baseline result record

Status: **Milestone 0 done by explicit user acceptance, 2026-10-25**. Debug/release
builds, isolated startup and later real-host streaming are recorded below. Pairing
remains skipped by user instruction; unrun performance, secondary-platform and
host-matrix checks remain coverage follow-ups, not asserted passes. Earlier
pending-status statements below describe their historical checkpoints. See
[closure](IMPLEMENTATION_PLAN.md#milestone-0-and-task-3b-closure-2026-10-25).

### Isolated startup evidence, 2026-10-25

A Python subprocess harness launched the release binary twice in a newly created
owner-only profile at
`/home/barmy/.local/state/perigee-baseline/runtime-20261025-4cvsh130`.
Each run remained alive for 15 seconds, then received SIGTERM and exited 0 within
the five-second termination allowance. The harness did not need SIGKILL. This
verifies bounded startup/restart, not interactive UI or streaming acceptance.

The harness ran the binary with an empty profile-local `work` directory as its
current directory (no `portable.dat`), `umask 077`, separate `config`, `cache`,
`data` and `tmp` directories, and these overrides:

```text
XDG_CONFIG_HOME=<profile>/config
XDG_CACHE_HOME=<profile>/cache
XDG_DATA_HOME=<profile>/data
TMPDIR=<profile>/tmp
QML_DISK_CACHE_PATH=<profile>/cache/qml
QT_QPA_PLATFORM=wayland
SDL_VIDEODRIVER=wayland
```

`XDG_RUNTIME_DIR` retained its existing compositor socket location. SDL logs
reported Wayland, SDL compatibility 2.32.74 and SDL3 3.4.18. Neither run reported
QML component-load failure; no visual inspection or keyboard/mouse interaction
was performed. Both runs warned about the QML ApplicationWindow ToolTip attached
property and GTK initialization order.

The application generated an isolated
`config/Moonlight Game Streaming Project/Moonlight.conf`. The `certificate`,
`key` and `uniqueid` fields were present and their in-memory SHA-256 comparisons
matched across restart; no values or hashes were printed. This proves local
identity persistence only, not host pairing persistence. Content hashes of 512
existing regular files in the native Moonlight organization directories under
the original XDG config/cache/data roots matched before and after. This check
does not cover other profiles, portable installations, or every global side effect.

Startup invoked the upstream asynchronous `SystemProperties` decoder query
(`app/backend/systemproperties.cpp`, `Session::getDecoderInfo()`). Its logs include
Intel i965 VA-API initialization, Wayland VA-API rendering and Vulkan selection,
alongside failed HEVC/AV1 decoder attempts, unsupported Vulkan video extensions,
iHD/hybrid-driver diagnostics and the earlier `wl_drm_interface` warning.
These are startup capability probes; no live stream decoder/renderer or codec
acceptance has been verified. No application or driver configuration was changed
to suppress failures.

`timeout 8s avahi-browse --resolve --terminate _nvstream._tcp` exited 0 with no
advertised hosts returned. This is a local discovery snapshot, not proof that
hosts are absent or unreachable elsewhere. Apollo/standard Sunshine host versions,
addresses, access and permissions remain unavailable; session tests are pending.

Private artifacts: `startup-1.log`, `startup-2.log`, `result.json`, isolated settings
and `launch.sh`. Do not publish raw settings or logs without review. To reopen the
same isolated baseline for supervised pairing/session tests:

```bash
/home/barmy/.local/state/perigee-baseline/runtime-20261025-4cvsh130/launch.sh
```

The launcher runs the uninstalled upstream-branded release binary with the above
isolated environment. No production client was unpaired or imported. Next:
obtain test-host versions/addresses, explicitly pair this new client, and record
real SDR H.264 session behavior first; verify permissions, audio/input, recovery
and repeated release measurements on Apollo and standard Sunshine separately.

### Native build evidence, 2026-10-25

The user supplied a completed DNF transaction for the corrected 30-package set.
Its output confirms the pending offline transaction was invalidated. Subsequent
`rpm -q` checks exited 0 for all 15 requested packages; no further privileged
transaction was run by the agent. These results supersede the earlier installation
blockers below.

`qmake6 -query QT_VERSION` exited 0: `6.11.2`. The following command exited 0:

```bash
pkg-config --modversion sdl2 SDL2_ttf openssl opus libavcodec libavutil libswscale libva libplacebo wayland-client
```

Versions in that order: `2.32.74`, `2.24.0`, `3.5.9`, `1.6`, `62.28.103`,
`60.26.103`, `9.5.103`, `1.23.0`, `7.360.1`, `1.26.0`.

Application/build/dependency comparison against the frozen upstream SHA again
showed no differences, including `h264bitstream`; recursive pins match the recorded
manifest and recursive submodule working trees remain clean. Source HEAD remains
`fb132c67e7e78bd493134f63e3eeb7c9e27fa5a0`. Existing user and documentation changes
were preserved.

From the source root, created `../perigee-build/debug` and
`../perigee-build/release`. In each directory ran:

```bash
qmake6 ../../perigee/moonlight-qt.pro >qmake.log 2>&1
make -j2 debug >build.log 2>&1    # debug directory
# Release directory uses: make -j2 release >build.log 2>&1
```

The make command only runs after successful qmake. Two jobs limit memory pressure
on this 7.6 GiB/four-CPU client. Both qmake and make exited 0 in each directory and produced
`/home/barmy/Projects/perigee-build/debug/app/moonlight` and
`/home/barmy/Projects/perigee-build/release/app/moonlight`. `file` confirms both
are x86-64 ELF executables with debug information, unstripped. Both `ldd` checks
exited 0; saved `ldd.log` files have no `not found` entries.

Configuration reports FFmpeg, VA-API X11/Wayland/DRM, VDPAU, DRM/master hooks,
Vulkan via libplacebo, EGL and Wayland extensions enabled. EGL probe passed;
Steam Link probe did not match this platform. Both build logs contain GCC 16
`QChar` SFINAE completeness diagnostics in Qt headers and deprecated FFmpeg
`AVVulkanDeviceContext::lock_queue`/`unlock_queue` callbacks. No build failure or
source workaround occurred.

Logs are outside the source tree under the respective build directories:
`qmake.log`, `build.log`, and `ldd.log`. At the build stage no application was launched or installed; the subsequent
isolated launches are recorded above. Actual stream decoding/presentation, audio/input, pairing, recovery and
performance on both host types remain unverified; compilation does not pass
the Milestone 0 runtime gate.

### Fedora preflight, 2026-10-25

Completed read-only environment/source inventory; no packages installed, application
launched, or credentials accessed. This is partial Milestone 0 evidence only.

| Executed command | Observed result |
| --- | --- |
| `git status --short` | Pre-existing staged deletion of `TODO.md` and untracked `.idea/`; preserved |
| `git rev-parse HEAD` | `fb132c67e7e78bd493134f63e3eeb7c9e27fa5a0` |
| `git diff ac296864ace96f51ad75f78b90644bdbca76c332 --stat -- app moonlight-qt.pro globaldefs.pri .gitmodules moonlight-common-c qmdnsengine` | Exit 0, no differences in these application/build/dependency paths |
| `git submodule status --recursive` | Exit 0; all five recursive revisions match the [manifest](REPOSITORY_ANALYSIS.md#dependency-relationships) |
| `git submodule foreach --quiet --recursive 'git status --short'` | Exit 0, no submodule changes |
| `cat /etc/os-release`; `uname -r` | Fedora 44 Workstation; kernel `7.2.11-200.fc44.x86_64` |
| `printenv XDG_SESSION_TYPE XDG_CURRENT_DESKTOP WAYLAND_DISPLAY` | `wayland`, `GNOME`, `wayland-0` |
| `rpm -q` for the development packages below plus GNOME/Mutter | GNOME Shell/Mutter `50.6-1.fc44`; Qt Wayland `6.11.2-1.fc44`; SDL compatibility development `2.32.74-1.fc44`; listed missing packages below |
| `command -v qmake6 make g++ pkg-config vainfo vulkaninfo` | `qmake6` absent; remaining commands found |
| `pkg-config --modversion sdl2 wayland-client` | Exit 0; `2.32.74`, `1.26.0` |
| `pkg-config --modversion sdl2 SDL2_ttf openssl opus libavcodec libavutil libswscale libva libplacebo wayland-client` | Exit 1; missing development metadata for SDL2_ttf, OpenSSL, Opus, FFmpeg, VA-API and libplacebo |
| `lspci -nnk -d ::03xx` | Intel Haswell-ULT `8086:0a16`, kernel driver `i915` |
| `timeout 15s vainfo` | Reports Intel i965 Haswell driver `2.4.0.pre1`, VA-API 1.23 / libva 2.23.0 and H.264 VLD profiles; also iHD initialization, `wl_drm_interface` and hybrid-driver diagnostics; standalone exit not captured |
| `timeout 15s vulkaninfo --summary` | Exit 0; Intel HD Graphics 4400, Mesa 26.2.5; llvmpipe also enumerated; warns Haswell Vulkan support is incomplete and skips a failing dzn ICD |
| `git ls-files scripts/collect-fedora-baseline.sh` | Exit 0 with no entry; file also absent on disk |

Missing queried RPMs: `qt6-qtbase-devel`, `qt6-qtdeclarative-devel`,
`qt6-qtsvg-devel`, `SDL2_ttf-devel`, `openssl-devel`, `opus-devel`, `ffmpeg-devel`,
`libva-devel`, `libplacebo-devel`. The subsequent dependency transaction preflight above resolves these requirements;
installation remains pending.
At this initial preflight no debug/release build was attempted without qmake;
the subsequent successful builds are recorded above. GPU capability enumeration
proves neither actual application hardware decoding nor presentation. Investigate
the reported driver diagnostics if baseline playback fails; no driver override
or architecture change has been applied.

Next: use the successfully built release binary with an isolated profile to
collect real Apollo and standard Sunshine session,
permissions, recovery and performance evidence. Host versions/access and monitor
configuration were not established at that preflight. The later user-directed
gate exception and Task 1A/1B results supersede that implementation blocker.

Record the following here when real tests run, without credential material:

| Evidence | Required record |
| --- | --- |
| Reproduction | Date, source SHA, submodule SHAs, clean/user changes, exact build/run commands and exit results |
| Client | Fedora/kernel, GNOME/KDE/compositor/session backend, Qt/SDL/FFmpeg/libva/libplacebo versions |
| Hardware | Client GPU/driver, host GPU/NVENC availability, monitors/scaling/refresh, codec/chroma/depth support |
| Hosts | Apollo Windows version/build and permissions; separate standard Sunshine version/build |
| Session | Resolution/FPS/bitrate/codec/chroma/HDR, actual hardware decoder and renderer, audio, mouse/keyboard checks |
| Performance | Release build, repeated identical workloads/network, decode/render times, drops, CPU/GPU/RAM and variance |
| Recovery | Pairing persistence, launch/resume, disconnect/reconnect/network interruption and resource release |
| Evidence files | Safe log/stat/screenshot locations, observed failures, limits and next investigation |

Start with reproducible SDR desktop and gaming workloads, then supported high
resolution/refresh/4:4:4/HDR combinations. Use three comparable runs where practical;
derive numerical regression gates from measured variation rather than inventing
latency targets. A successful native build does not complete Milestone 0 alone.

## Milestone 2 local check record

The read-only capability foundation builds in the existing debug app directory;
57 standalone parser/state checks and 32 import regressions pass on Fedora 44.
Two isolated Wayland GUI runs against local HTTP fixtures use only serverinfo
requests and leave 512 checked production files unchanged. These are fixtures,
not real Sunshine/Apollo, TLS/permission or streaming acceptance. Exact commands,
private evidence paths and pending UI checks are recorded in
[Milestone 2](IMPLEMENTATION_PLAN.md#milestone-2-implementation-and-local-evidence-2026-10-25).
The frozen upstream release baseline was not rebuilt.

## Task 3A local check record

The disabled session service bridge builds in the existing debug directory. Its
standalone Qt/Network/SDL target passes 15 lifecycle cases with temporary synthetic
TLS identities, 200 restarts and 30 pending-reply cancellations. Isolated Wayland
startup checks retain ordinary serverinfo-only polling and preserve 512 checked
production files. A live feature-disabled stream and performance comparison remain
pending; the frozen upstream release build was not changed. Exact commands and
limits are recorded in [Task 3A](IMPLEMENTATION_PLAN.md#task-3a-implementation-and-local-evidence-2026-10-25).

## User-provided real-host check record

After the user configured an isolated connection and authorized testing, paired
pinned-HTTPS metadata and app-list checks passed. Initial launch denial (403)
matched the reported mask; a later launch grant was observed without re-pairing.
Three 12-second native Wayland Desktop streams at 1920×1080 H.264 chose VA-API
and EGL/GLES, initialized stereo audio and disconnected with exit 0. Three
worker-only pinned GET/stop cycles also completed. All 512 checked production
files stayed unchanged. Host release, audible audio/input and controlled release
performance remain unverified; this does not close the full baseline gate.

Exact commands, private artifacts, observed rates and the worker's bare-serverinfo
authentication limitation are recorded in
[supervised host checks](IMPLEMENTATION_PLAN.md#supervised-real-host-checks-after-task-3a-2026-10-25).
No production identity was used or host administrative setting changed by the agent.

## Task 3B local check record

Opt-in manual text clipboard and its state controller build on Fedora 44/Qt 6.11.2.
The standalone suite reports 16 passing protocol/lifecycle cases and clean Memcheck
results; 32 importer regressions also pass. Isolated Wayland startup retains
serverinfo-only traffic and preserves 512 checked production files. Automatic SDL
retrieval/poll wiring remains gated on measured platform access/latency. The test host initially lacked clipboard grants; subsequent paired metadata now
reports both grants. Saved local opt-in and live transfer/GNOME/KDE focus and
clipboard acceptance remain pending. Exact commands and limits are
in [Task 3B](IMPLEMENTATION_PLAN.md#task-3b-implementation-and-local-evidence-2026-10-25).
