# Appearance preference checks

This Fedora/Qt 6 Qt Test target links the current qmake debug objects with a
synthetic-profile entry point. Build the application first, then from the source root:

```bash
make -C ../perigee-build/debug/app -f Makefile.Debug -j2
make -C ../perigee-build/debug/app -f Makefile.Debug \
    -f "$PWD/tests/ui-theme/checks.mk" theme-check-build -j2
QT_QPA_PLATFORM=wayland ../perigee-build/debug/theme-tests/tst_uitheme \
    -o ../perigee-build/theme-results.txt,txt
```

An offscreen run can use `QT_QPA_PLATFORM=offscreen SDL_VIDEODRIVER=dummy`.
`PERIGEE_THEME_PREVIEW_DIR=/absolute/output/path` optionally saves light/dark
screenshots of the real application window. The target requires Qt 6 Test and
the existing application dependencies; it does not modify the application build.

Cases cover legacy default-dark settings, saved light/dark values, invalid-value
fallback, save/reload, change notification, an unchanged bitrate, the actual
Settings selector, all seven light SVG resources, and a popup whose colors update
while open. The test uses the real main QML window and preference object with
disposable XDG/TMPDIR state. Host discovery is disabled and no host is configured.
The `shortcutPreferencePersistence` case also checks actual preference
reload/save preservation and invalid-shortcut fallback. No pairing or stream is started. Inherited mapping/compatibility fetches use a
closed loopback proxy and can report expected proxy-refusal warnings. Existing
tooltip/shortcut and GTK initialization warnings remain separate from theme checks.

For user acceptance, select **Settings → UI Settings → Appearance → Light**,
inspect host/status icons, menus, dialogs, settings and app tiles, restart the
client and confirm the selection persists, then switch back to **Dark**. This
does not establish secondary-platform, compositor-policy or streaming performance
acceptance. There is no automatic system-theme mode in this increment.

## Shortcut editor and synthetic stream controls

The same application-linked target now checks Settings → Keyboard shortcuts,
remapping, reserved fields, conflict feedback, restore defaults and save/reload.
It verifies a 720-pixel window and keyboard focus after the responsive reflow.
The SDL dispatch cases use a hidden synthetic window and inactive Common C;
no stream is initialized and no clipboard is read. They cover English plus all
30 compiled catalogs, stats remapping, forwarding the former binding, AltGr
bypass, capture-off Super handling, physical apostrophe key releases, raising
remote keys on panel open, disabled clipboard commands, stats selection, Escape,
focus-loss dismissal and the reserved disconnect action. A fake overlay renderer
receives actual CPU surfaces. These are not live host/input/compositor-policy
or performance acceptance results.

```bash
QT_QPA_PLATFORM=offscreen SDL_VIDEODRIVER=dummy \
  ../perigee-build/debug/theme-tests/tst_uitheme
QT_QPA_PLATFORM=wayland SDL_VIDEODRIVER=wayland \
  ../perigee-build/debug/theme-tests/tst_uitheme
QT_QPA_PLATFORM=wayland SDL_VIDEODRIVER=wayland QT_SCALE_FACTOR=2 \
  ../perigee-build/debug/theme-tests/tst_uitheme \
  shortcutEditor streamControlDispatch:english applicationThemeAndPopup
```

Previews also include `shortcuts.png` and `stream-controls.png`. The fixture keeps
one QML engine alive and disables its gamepad navigator before constructing SDL
input, matching the normal UI-to-stream handoff. Repeater fields are found through
the visual item tree. Do not run it against a production profile.


## Milestone 6 verification (2026-11-10)

Four additional application-linked cases cover the profile editor and detached
Session preferences, SFTP dialogs, app UUID parser, and synthetic host persistence.
They click actual Save buttons, verify invalid drafts remain open, check opaque
light/dark backgrounds and keyboard scrolling, and exercise CLI profile bypass.
SFTP uses a fake QDesktopServices handler; no SSH or external file manager is opened.
Current results: 47 offscreen, 47 native Wayland, 6 selected scale2 cases pass.
These do not verify media startup, host acceptance or real transfers.


Current temporary-build commands (from the source root):

```bash
CCACHE_DISABLE=1 make -C /tmp/perigee-6a-build/app -f Makefile.Debug \
    -f "$PWD/tests/ui-theme/checks.mk" theme-check-build -j2
QT_QPA_PLATFORM=offscreen SDL_VIDEODRIVER=dummy \
    /tmp/perigee-6a-build/theme-tests/tst_uitheme
QT_QPA_PLATFORM=wayland SDL_VIDEODRIVER=wayland \
    /tmp/perigee-6a-build/theme-tests/tst_uitheme
QT_QPA_PLATFORM=wayland SDL_VIDEODRIVER=wayland QT_SCALE_FACTOR=2 \
    /tmp/perigee-6a-build/theme-tests/tst_uitheme \
    sessionProfileEditorAndSessionOwnership transferDialogPresentation \
    shortcutEditor applicationThemeAndPopup
```
