# Timed streaming notifications

This independent Qt Core/Qt Test + SDL2/SDL2_ttf target checks the existing status
overlay's five-second notification layer. It reads the repository's ModeSeven.ttf
through a test-only Path reader and renders CPU text surfaces into a fake renderer.
It does not open a stream, read settings/credentials or touch a clipboard.

```bash
mkdir -p ../perigee-build/tests/overlay-notifications
cd ../perigee-build/tests/overlay-notifications
qmake6 ../../../perigee/tests/overlay-notifications/overlay-notifications.pro
make -j2
./tst_overlaynotifications -o results.txt,txt
```

Coverage: duration and replacement, unchanged-text refresh without rerendering,
connection/gamepad status priority, preserving warnings after expiry, returning to
an unexpired notice, font surface creation and SDL tick wraparound. Expiry tests
advance explicit timestamps instead of sleeping. No app build depends on this target.

Optional lifetime check:

```bash
valgrind --tool=memcheck --leak-check=full \
  --show-leak-kinds=definite,indirect \
  --errors-for-leak-kinds=definite,indirect --error-exitcode=99 \
  --log-file=valgrind.log ./tst_overlaynotifications -o results-valgrind.txt,txt
```

Actual placement and readability on a streaming window remain a live UI check.
See [clipboard feedback evidence](../../docs/IMPLEMENTATION_PLAN.md#clipboard-confirmation-follow-up-2026-10-25).
