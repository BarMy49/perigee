# Fedora development, baseline, and packaging

Target selected by the user: Fedora 44 GNOME Wayland first, KDE Plasma Wayland
second; X11/XWayland compatibility where available. Use current packaged Qt 6 and
the pinned [source/dependency manifest](REPOSITORY_ANALYSIS.md#inspected-revisions).
These Bash commands are source-checked instructions, not an executed Fedora build.

## Verification status

| Check | Result as of 2026-10-25 |
| --- | --- |
| Source/build/submodule inspection | Executed; qmake/C++17/Qt Quick structure and exact pins recorded |
| Fedora/RPM Fusion package documentation | Inspected; DNF resolution/install not executed |
| Windows tools | `cmake.exe`, `wsl.exe`, `docker.exe` found; qmake/qmake6 and queried GNU/Clang compiler commands absent |
| Docker Linux engine | `docker version --format '{{json .Server}}'` could not connect to `dockerDesktopLinuxEngine`; no container build possible through that daemon |
| WSL distributions | `wsl --list --quiet` listed only `docker-desktop`; no usable Fedora desktop/toolchain established |
| Fedora compile/debug/release | Not executed; pending suitable environment |
| Pairing/streaming/input/audio/GPU tests | Not executed; real Windows Apollo and standard Sunshine hosts required |
| GNOME/KDE/X11, HDR/VRR, packaging/performance | Not executed |

Do not start application Milestone 1A until the
[baseline acceptance gate](IMPLEMENTATION_PLAN.md#milestone-0-baseline) is recorded
as passed. A later container compile can validate dependencies/builds but cannot
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

Recommended full development set, pending actual resolver validation:

```bash
sudo dnf install \
  git gcc-c++ make pkgconf-pkg-config \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel qt6-qtwayland \
  sdl2-compat-devel SDL2_ttf-devel openssl-devel opus-devel ffmpeg-devel \
  libva-devel libvdpau-devel libdrm-devel libplacebo-devel \
  wayland-devel wayland-protocols libxkbcommon-devel libX11-devel \
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

## Baseline collection helper

From this checkout on Fedora 44:

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
Current executable in that build directory is `app/moonlight`. Milestone 1A changes
the Linux output to `app/perigee`; until then that command does not exist.

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
configure qmake with `PREFIX=/usr`, then `make INSTALL_ROOT="$PWD/stage" install`.
Validate the Perigee `.desktop` with `desktop-file-validate` and the planned
`io.github.BarMy49.Perigee.metainfo.xml` with `appstreamcli validate`. Those tools
and packaging commands have not been run here. See [branding](BRANDING.md).

## Baseline result record

Status: **pending; no Fedora compilation or runtime acceptance has passed**.

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
