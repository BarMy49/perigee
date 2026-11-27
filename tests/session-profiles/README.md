# Session profiles

This target compiles real StreamingPreferences with deterministic non-Wayland/
non-slow-GPU WMUtils stubs and disposable INI state. Ten cases cover snapshots,
strict preset validation, loading a profile on selection, saving changes into the
selected profile, switching between profiles and back to Current settings, editing
the selected profile, stored profiles winning over older settings, independent
sessions, all writable properties, shortcut bindings/packet size, separate-process
restart, corrupt stored profiles and failed-write rollback. It opens no host or window.

```bash
mkdir -p /tmp/perigee-profile-tests
cd /tmp/perigee-profile-tests
qmake6 /home/barmy/Projects/perigee/tests/session-profiles/session-profiles.pro
CCACHE_DISABLE=1 make -j2
./tst_sessionprofiles
valgrind --error-exitcode=99 --leak-check=full ./tst_sessionprofiles
```

Actual QML editor clicks and Session ownership/CLI bypass are covered by
[application-linked checks](../ui-theme/README.md). Live media behavior and
controlled performance remain unverified.
