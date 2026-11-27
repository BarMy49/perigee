# Build Perigee on Fedora

This guide covers building on a regular Fedora installation. Ready-to-install
packages and the tested runtime combinations are in the
[prerelease notes](https://github.com/BarMy49/perigee/releases/tag/v6.2.0-perigee.1). Use the RPM built for your Fedora version so that
its library requirements match the installed FFmpeg ABI.

## Dependencies

The [Fedora CI workflow](../.github/workflows/ci-fedora.yml) records the
dependency set tested in a Fedora 44 container. Install the matching development
providers for your Fedora version. The pkg-config requirements let DNF select
providers compatible with your installed SDL and FFmpeg packages.

```bash
sudo dnf install \
  git gcc-c++ make python3 findutils which openssl \
  qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel qt6-qtwayland qt6-linguist \
  'pkgconfig(openssl)' 'pkgconfig(sdl2)' 'pkgconfig(SDL2_ttf)' 'pkgconfig(opus)' \
  'pkgconfig(libavcodec)' 'pkgconfig(libavformat)' 'pkgconfig(libavutil)' 'pkgconfig(libswscale)' \
  'pkgconfig(libva)' 'pkgconfig(libva-x11)' 'pkgconfig(libva-wayland)' 'pkgconfig(libva-drm)' \
  'pkgconfig(vdpau)' 'pkgconfig(libdrm)' 'pkgconfig(libplacebo)' \
  'pkgconfig(wayland-client)' 'pkgconfig(x11)' desktop-file-utils appstream
```

Review the DNF transaction before accepting it if your machine uses RPM Fusion
FFmpeg. Keep a consistent multimedia package set. The SDL2 API is available
through Fedora's SDL2 compatibility library.

## Debug and release builds

From the source root, initialize the exact [submodule pins](REPOSITORY_ANALYSIS.md#dependency-relationships).
Use a separate directory for each build mode.

```bash
git submodule update --init --recursive
source_root=$(pwd -P)
build_root="$(dirname "$source_root")/perigee-build"
for mode in debug release; do
    mkdir -p "$build_root/$mode"
    (
        cd "$build_root/$mode"
        qmake6 "$source_root/moonlight-qt.pro" PREFIX=/usr
        make -j1 "$mode"
    ) || exit
done
```

The binary is `app/perigee` in each build directory. Check qmake's feature output
for FFmpeg, VA-API/EGL, libplacebo/Vulkan and Wayland. For tests, use debug objects
with the [focused Linux runner](../tests/README.md). Compare streaming performance
with release builds. See [resource limits](../packaging/BUILD_RESOURCES.md) for
memory-constrained builds.

## Run with a disposable profile

Set `build_root` to the absolute build directory parent from the previous example.
Leave `XDG_RUNTIME_DIR` pointing at your desktop session so the app can connect
to the compositor.

```bash
profile=$(mktemp -d)
mkdir -p "$profile"/{config,cache,data,tmp}
cd "$profile"
env XDG_CONFIG_HOME="$profile/config" \
    XDG_CACHE_HOME="$profile/cache" \
    XDG_DATA_HOME="$profile/data" \
    TMPDIR="$profile/tmp" \
    QT_QPA_PLATFORM=wayland SDL_VIDEODRIVER=wayland \
    "$build_root/debug/app/perigee"
```

For X11 or XWayland, use
`QT_QPA_PLATFORM=xcb SDL_VIDEODRIVER=x11`. To inspect unresolved libraries, run
`ldd app/perigee`. For decoder and renderer issues, record the actual session
statistics and GPU, driver and compositor versions. `vainfo` and
`vulkaninfo --summary` can help diagnose the local driver. Keep credentials,
host secrets and private clipboard text out of logs shared for review.

## Stage and package

Run from the source root with `build_root` set as above. Stage the release
payload without installing it into the system.

```bash
stage=$(mktemp -d)
make -C "$build_root/release/app" -f Makefile.Release INSTALL_ROOT="$stage" install
python3 scripts/check-linux-stage.py "$stage"
```

See the [RPM](../packaging/rpm/README.md), [Flatpak](../packaging/flatpak/README.md)
and [AppImage](../packaging/appimage/README.md) recipes for artifact checks and
matching source material. Flatpak uses sandboxed profile storage and file portal
selection. AppImage uses X11 or XWayland.

Historical baseline measurements and test transcripts are available in
[the build history](https://github.com/BarMy49/perigee/blob/18c34bf1e44c5fd3c9fa1fe3f735fc96a8a2539b/docs/FEDORA_BUILD.md).
