# App favorites checks

This Fedora/Qt 6 Qt Test target links the current qmake debug application objects
with a test entry point. It exercises the real app model, host record/save path
and AppView menu without duplicating production logic or modifying the app build.
Build the application first, then from the repository root:

```bash
make -C ../perigee-build/debug/app -f Makefile.Debug -j2
make -C ../perigee-build/debug/app -f Makefile.Debug \
    -f "$PWD/tests/app-favorites/checks.mk" favorites-check-build -j2
QT_QPA_PLATFORM=offscreen ../perigee-build/debug/favorites-tests/tst_appfavorites \
    -o ../perigee-build/favorites-results.txt,txt
```

`checks.mk` uses the debug build's compiler definitions, dependencies and objects,
excluding its startup entry point. Test output is in `debug/favorites-tests/`.
Qt 6 Test development files and `/usr/lib64/qt6/libexec/moc` are required. Run with
`QT_QPA_PLATFORM=wayland` for native Wayland window/selection coverage as well.

The test entry point sets disposable XDG/TMPDIR roots and synthetic host records
before initializing the application. It never starts host polling or creates a
stream, pairs a host or reads real credentials. Synthetic box art is cached locally.
The inherited compatibility fetch is routed to a closed loopback proxy; its
expected `ProxyConnectionRefusedError` warnings are not host failures.

Coverage includes legacy default-off records, favorites-first/alphabetical order,
valid Qt model row changes, persistent index and QML tile selection, repeated
favorite/unfavorite actions, stale IDs, host-list refresh and rename/removal,
pending box-art callbacks, hidden/direct-launch/running state, separate hosts,
save/reload of both true and false, and the actual AppView Favorite menu action.
The importer regression independently asserts that a source `favorite` field is
excluded from Moonlight import, including explicit pairing import.

For interactive acceptance, mark an app Favorite from its menu, confirm its star
and position, reopen the host and restart the client, then uncheck Favorite.
Verify selection stays on that app and normal launch/resume, hidden apps and
Direct Launch still behave as expected. This target does not establish real
streaming, physical gamepad, performance or screen-reader acceptance.
