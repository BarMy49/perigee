#!/usr/bin/env bash
set -euo pipefail
if [[ $# != 2 ]]; then
    echo "Usage: $0 DEBUG_BUILD_DIRECTORY RESULT_DIRECTORY" >&2
    exit 2
fi
SOURCE_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
DEBUG_BUILD=$(realpath -- "$1")
RESULT_ROOT=$(realpath -m -- "$2")
[[ -f "$DEBUG_BUILD/app/debug/main.o" && -x "$DEBUG_BUILD/app/perigee" ]]
[[ ! -e "$RESULT_ROOT" ]] || { echo 'Result directory already exists' >&2; exit 1; }
mkdir -p "$RESULT_ROOT"
export CCACHE_DISABLE=1 QT_QPA_PLATFORM=offscreen SDL_VIDEODRIVER=dummy
export QT_QUICK_BACKEND=${QT_QUICK_BACKEND:-software}
export PERIGEE_TEST_BINARY=$DEBUG_BUILD/app/perigee
while read -r suite target; do
    mkdir -p "$RESULT_ROOT/$suite"
    cd "$RESULT_ROOT/$suite"
    qmake6 "$SOURCE_ROOT/tests/$suite/$suite.pro"
    make -j"${PERIGEE_BUILD_JOBS:-1}"
    "./$target" -o "$RESULT_ROOT/$suite.txt",txt
done <<'SUITES'
apollo-capabilities tst_apollocapabilities
session-services tst_sessionservices
display-launch tst_displaylaunch
identity-import tst_identityimport
session-profiles tst_sessionprofiles
milestone6-options tst_milestone6options
shortcut-modifiers tst_shortcutmodifiers
translations tst_translations
SUITES
cd "$SOURCE_ROOT"
python3 scripts/check-translations.py --lupdate "${PERIGEE_LUPDATE:-/usr/lib64/qt6/bin/lupdate}"
/usr/lib64/qt6/bin/qmltestrunner -input tests/grid-navigation -o "$RESULT_ROOT/grid-navigation.txt",txt
make -C "$DEBUG_BUILD/app" -f Makefile.Debug -f "$SOURCE_ROOT/tests/ui-theme/checks.mk" \
    theme-check-build -j"${PERIGEE_BUILD_JOBS:-1}"
"$DEBUG_BUILD/theme-tests/tst_uitheme" -o "$RESULT_ROOT/ui-theme.txt",txt
make -C "$DEBUG_BUILD/app" -f Makefile.Debug -f "$SOURCE_ROOT/tests/app-favorites/checks.mk" \
    favorites-check-build -j"${PERIGEE_BUILD_JOBS:-1}"
"$DEBUG_BUILD/favorites-tests/tst_appfavorites" -o "$RESULT_ROOT/app-favorites.txt",txt
