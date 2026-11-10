# Host and app grid keyboard checks

This Qt Quick Test fixture loads the application's shared `CenteredGridView`,
`NavigableItemDelegate` and menu components with synthetic entries. It opens no
connection and cannot launch, resume or quit a host application.

From the repository root, with Qt Quick Test installed:

```bash
QT_QPA_PLATFORM=offscreen /usr/lib64/qt6/bin/qmltestrunner \
    -input tests/grid-navigation -o ../perigee-build/grid-navigation.txt,txt
```

Use `QT_QPA_PLATFORM=wayland` instead to exercise native Wayland window/focus
handling in an available desktop session. No application rebuild is required for
the fixture; rebuild the application to update its embedded QML resources.

The 25 checks include setup/cleanup, Home/End, Page Up/Page Down, selection from
an initially unselected grid, clamping, empty grids, resized viewports, vertical
margins, modifier handling, arrows, toolbar focus, Shift+F10/Menu, Escape, menu
action activation and prevention of unintended delegate activation. Page size is
the number of whole rows and columns within the viewport margins, with a minimum
of one item. These shared-component checks do not establish physical gamepad,
screen-reader, full application or streaming acceptance.

For interactive acceptance in the application, select a host or app with arrows,
use Home/End and Page Up/Page Down, open its menu with Shift+F10 and close it with
Escape. Check that selection is retained and navigation continues. The existing
Menu key, right-click and gamepad paths remain available. Toolbar/text-field keys
should continue to act on the focused control.
