#!/usr/bin/env bash
set -euo pipefail

fail() { echo "$1" >&2; exit 1; }
SOURCE_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_ROOT=${PERIGEE_APPIMAGE_BUILD_ROOT:-$SOURCE_ROOT/build}
BUILD_FOLDER=$BUILD_ROOT/build-release
DEPLOY_FOLDER=$BUILD_ROOT/deploy-release
INSTALLER_FOLDER=$BUILD_ROOT/installer-release
LINUXDEPLOY=${LINUXDEPLOY:-linuxdeploy-$(uname -m).AppImage}
VERSION=${CI_VERSION:-$(cat "$SOURCE_ROOT/app/version.txt")}
BUILD_JOBS=${PERIGEE_BUILD_JOBS:-1}
[[ "$VERSION" =~ ^[A-Za-z0-9][A-Za-z0-9.+_-]*$ ]] || fail 'Invalid artifact version'
[[ "$BUILD_JOBS" =~ ^[1-9][0-9]*$ ]] || fail 'Invalid build job count'
[[ "$BUILD_ROOT" = /* ]] || fail 'PERIGEE_APPIMAGE_BUILD_ROOT must be absolute'
command -v qmake6 >/dev/null || fail 'Unable to find qmake6'
command -v convert >/dev/null || fail 'Unable to find ImageMagick convert'
command -v "$LINUXDEPLOY" >/dev/null || fail 'Unable to find linuxdeploy'
export LDAI_RUNTIME_FILE=${LDAI_RUNTIME_FILE:-$(dirname "$(command -v "$LINUXDEPLOY")")/runtime-$(uname -m)}
[[ -s "$LDAI_RUNTIME_FILE" ]] || fail 'Unable to find the reviewed AppImage runtime'
for directory in "$BUILD_FOLDER" "$DEPLOY_FOLDER" "$INSTALLER_FOLDER"; do
    [[ ! -e "$directory" && ! -L "$directory" ]] || fail "Output already exists: $directory"
done
mkdir -p "$BUILD_FOLDER" "$DEPLOY_FOLDER" "$INSTALLER_FOLDER"
export CFLAGS="${CFLAGS:-} -flto=$BUILD_JOBS"
export CXXFLAGS="${CXXFLAGS:-} -flto=$BUILD_JOBS"
export LDFLAGS="${LDFLAGS:-} -flto=$BUILD_JOBS"
cd "$BUILD_FOLDER"
qmake6 "$SOURCE_ROOT/moonlight-qt.pro" CONFIG+=disable-wayland CONFIG+=disable-libdrm PREFIX=/usr DEFINES+=APP_IMAGE
make -j"$BUILD_JOBS" release
make -C app -f Makefile.Release INSTALL_ROOT="$DEPLOY_FOLDER" install
python3 "$SOURCE_ROOT/scripts/check-linux-stage.py" "$DEPLOY_FOLDER"
for entry in LICENSE packaging/rpm/ModeSeven-README \
    moonlight-common-c/moonlight-common-c/LICENSE.txt \
    moonlight-common-c/moonlight-common-c/enet/LICENSE \
    moonlight-common-c/moonlight-common-c/nanors/LICENSE \
    qmdnsengine/qmdnsengine/LICENSE.txt h264bitstream/LICENSE app/SDL_GameControllerDB/LICENSE; do
    install -Dm644 "$SOURCE_ROOT/$entry" "$DEPLOY_FOLDER/usr/share/licenses/perigee/$entry"
done
cp -a "$SOURCE_ROOT/packaging/appimage/runtime" \
    "$DEPLOY_FOLDER/usr/share/licenses/perigee/appimage-runtime"

RASTER_ICON=usr/share/icons/hicolor/256x256/apps/io.github.BarMy49.Perigee.png
mkdir -p "$DEPLOY_FOLDER/$(dirname "$RASTER_ICON")"
convert -background none "$SOURCE_ROOT/app/res/io.github.BarMy49.Perigee.svg" \
    -resize 256x256 "$DEPLOY_FOLDER/$RASTER_ICON"
ln -s "$RASTER_ICON" "$DEPLOY_FOLDER/io.github.BarMy49.Perigee.png"
ln -s io.github.BarMy49.Perigee.png "$DEPLOY_FOLDER/.DirIcon"
export QML_SOURCES_PATHS=$SOURCE_ROOT/app/gui
export QMAKE=qmake6
LIBVA_FALLBACK_DIR=$DEPLOY_FOLDER/opt/libva-fallback
SYSTEM_LIBVA=$(ldconfig -p 2>/dev/null | awk '/libva\.so\.2/{if (!found++) print $NF}')
[[ -n "$SYSTEM_LIBVA" ]] || fail 'Unable to locate libva for the fallback probe'
mkdir -p "$LIBVA_FALLBACK_DIR"
cp -a "$(dirname "$SYSTEM_LIBVA")"/libva*.so* "$LIBVA_FALLBACK_DIR/"
cc -O2 -L"$(dirname "$SYSTEM_LIBVA")" -Wl,--no-as-needed \
    -o "$LIBVA_FALLBACK_DIR/libva-probe" "$SOURCE_ROOT/app/deploy/linux/libva-probe.c" -lva -lva-x11
va_nodes() { LC_ALL=C readelf -V "$1" 2>/dev/null | { grep -oE 'VA_API_[0-9]+\.[0-9]+\.[0-9]+' || true; } | sort -u; }
NEEDED_NODES=$(for binary in "$DEPLOY_FOLDER/usr/bin/perigee" \
    /usr/local/lib*/libav*.so* /usr/local/lib*/libsw*.so* \
    /usr/lib64/libav*.so* /usr/lib64/libsw*.so* \
    /usr/lib/x86_64-linux-gnu/libav*.so* /usr/lib/x86_64-linux-gnu/libsw*.so*; do
    if [[ -f "$binary" ]]; then va_nodes "$binary"; fi
done | sort -u)
PROBE_NODES=$(va_nodes "$LIBVA_FALLBACK_DIR/libva-probe")
MISSING_NODES=$(comm -13 <(echo "$PROBE_NODES") <(echo "$NEEDED_NODES"))
[[ -z "$MISSING_NODES" ]] || fail "libva-probe is missing version nodes: $MISSING_NODES"
EXCLUDES=('--exclude-library=libva.so*' '--exclude-library=libva-drm.so*' \
    '--exclude-library=libva-wayland.so*' '--exclude-library=libva-x11.so*')
SDL3=$(ldconfig -p 2>/dev/null | awk '/libSDL3\.so\.0/{if (!found++) print $NF}')
EXTRA_LIBRARIES=()
if [[ -n "$SDL3" ]]; then EXTRA_LIBRARIES+=("--library=$SDL3"); fi
"$LINUXDEPLOY" --appdir "$DEPLOY_FOLDER" "${EXTRA_LIBRARIES[@]}" --plugin qt "${EXCLUDES[@]}"
[[ -e "$DEPLOY_FOLDER/AppRun" ]] || fail 'linuxdeploy did not generate AppRun'
cp -a "$DEPLOY_FOLDER/AppRun" "$DEPLOY_FOLDER/AppRun.perigee"
rm "$DEPLOY_FOLDER/AppRun"
cd "$INSTALLER_FOLDER"
export VERSION
export LDAI_OUTPUT=Perigee-$VERSION-$(uname -m).AppImage
"$LINUXDEPLOY" --appdir "$DEPLOY_FOLDER" \
    --custom-apprun "$SOURCE_ROOT/app/deploy/linux/appimage/AppRun" \
    "${EXCLUDES[@]}" --output appimage
[[ -s "$LDAI_OUTPUT" ]] || fail "Expected artifact missing: $LDAI_OUTPUT"
sha256sum "$LDAI_OUTPUT" > "$LDAI_OUTPUT.sha256"
echo "Build successful: $INSTALLER_FOLDER/$LDAI_OUTPUT"
