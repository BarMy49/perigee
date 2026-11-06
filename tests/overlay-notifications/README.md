# Timed streaming notifications

This independent Qt Core/Gui/Qt Test + SDL2/SDL2_ttf target checks the existing status
overlay's five-second notification layer. Qt font fallback/shaping renders notices;
the repository's ModeSeven.ttf remains the primary warning/debug font, read through
a test-only Path reader. CPU surfaces go to a fake renderer.
It does not open a stream, read settings/credentials or touch a clipboard.

```bash
mkdir -p ../perigee-build/tests/overlay-notifications
cd ../perigee-build/tests/overlay-notifications
qmake6 ../../../perigee/tests/overlay-notifications/overlay-notifications.pro
make -j2
QT_QPA_PLATFORM=offscreen ./tst_overlaynotifications -o results.txt,txt
```

Coverage: duration and replacement, unchanged-text refresh without rerendering,
Unicode Polish/Arabic-script/CJK/Tamil surfaces, connection/gamepad status priority, preserving warnings after expiry, returning to
an unexpired notice, font surface creation and SDL tick wraparound. Expiry tests
advance explicit timestamps instead of sleeping. No app build depends on this target.

Optional lifetime check:

```bash
QT_QPA_PLATFORM=offscreen valgrind --tool=memcheck --leak-check=full \
  --show-leak-kinds=definite,indirect \
  --errors-for-leak-kinds=definite,indirect --error-exitcode=99 \
  --log-file=valgrind.log ./tst_overlaynotifications -o results-valgrind.txt,txt
```

Actual placement and readability on a streaming window remain a live UI check.
See [clipboard feedback evidence](../../docs/IMPLEMENTATION_PLAN.md#clipboard-confirmation-follow-up-2026-10-25).

The command-panel cases additionally check persistent visibility, warning
preemption without reopening, notification coexistence, absent-renderer refusal,
and a complete eight-action panel in all 30 compiled locales. Glyph contrast
checks require light opaque pixels in each Unicode sample; text rendering uses
separate outline and fill passes. This catches black-only surfaces that a simple
nonempty-pixel check would miss. Panels are prepared before streaming and use the
existing status slot; the tests add no renderer or media readback path.
