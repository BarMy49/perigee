#!/usr/bin/env bash
set -euo pipefail
if [[ $# != 2 || "$2" != /* ]]; then
    echo "Usage: $0 SOURCE_ARCHIVE ABSOLUTE_OUTPUT_DIRECTORY" >&2
    exit 2
fi
SOURCE_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
SOURCE_ARCHIVE=$(realpath -- "$1")
OUTPUT_ROOT=$2
[[ ! -e "$OUTPUT_ROOT" && ! -L "$OUTPUT_ROOT" ]] || { echo 'Output already exists' >&2; exit 1; }
flatpak info org.kde.Sdk//6.11 --show-commit >/dev/null
mkdir -p "$OUTPUT_ROOT/sources"
SOURCE_COPY=$OUTPUT_ROOT/sources/$(basename -- "$SOURCE_ARCHIVE")
cp -- "$SOURCE_ARCHIVE" "$SOURCE_COPY"
python3 "$SOURCE_ROOT/scripts/prepare-flatpak.py" --source "$SOURCE_COPY" \
    --output "$OUTPUT_ROOT/io.github.BarMy49.Perigee.json"
flatpak info org.kde.Sdk//6.11 --show-commit > "$OUTPUT_ROOT/sdk-commit.txt"
flatpak info org.kde.Platform//6.11 --show-commit > "$OUTPUT_ROOT/runtime-commit.txt"
flatpak-builder --sandbox --disable-rofiles-fuse --disable-updates --jobs="${PERIGEE_BUILD_JOBS:-1}" \
    --state-dir="$OUTPUT_ROOT/state" --repo="$OUTPUT_ROOT/repository" \
    "$OUTPUT_ROOT/app" "$OUTPUT_ROOT/io.github.BarMy49.Perigee.json"
flatpak build-bundle --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo \
    "$OUTPUT_ROOT/repository" "$OUTPUT_ROOT/Perigee-6.2.0-$(uname -m).flatpak" \
    io.github.BarMy49.Perigee devel
sha256sum "$OUTPUT_ROOT/Perigee-6.2.0-$(uname -m).flatpak" > "$OUTPUT_ROOT/bundle.sha256"
