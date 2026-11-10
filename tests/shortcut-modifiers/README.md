# Local shortcut modifier checks

Synthetic modifier coverage for the predicate used by the actual SDL keyboard
handler. No window, host, clipboard or network access is needed.

```bash
mkdir -p /tmp/perigee-shortcut-modifiers
cd /tmp/perigee-shortcut-modifiers
qmake6 /home/barmy/Projects/perigee/tests/shortcut-modifiers/shortcut-modifiers.pro
CCACHE_DISABLE=1 make -j2
./tst_shortcutmodifiers
```

The modifier test covers all 2,048 combinations of left/right Ctrl, Alt, Shift and GUI,
Num Lock, Caps Lock and SDL's AltGr (`KMOD_MODE`) flag. Ctrl+Alt+Shift remains
eligible with either side and lock keys; AltGr-marked input is never eligible.
This verifies the local-command modifier gate, not remote key delivery or text
composition. Layouts/backends that do not report `KMOD_MODE` cannot be distinguished
by this predicate and still need real GNOME/KDE layout tests. Live key releases,
emergency ungrab, dead keys and IME acceptance remain pending.

# Persistent binding overrides

With Perigee closed, an advanced user can edit the independent native/portable
Perigee INI profile. Under `[localshortcuts]`, set an action to one lowercase ASCII
letter, or an empty value to disable it. Example:

```ini
[localshortcuts]
stats=t
mouse=
keyboardGrab=b
```

The chord remains Ctrl+Alt+Shift. Keys are applied at the next session startup;
logical-key matching precedes the physical letter-scancode fallback. Supported
editable action names are `fullscreen`, `stats`, `mouse`, `cursor`, `minimize`,
`paste`, `clipboardSend`, `clipboardReceive`, `pointerLock` and `keyboardGrab`.
Platform and clipboard-consent gates still apply. Disabled actions follow the
ordinary remote input path; no remote key remapping is added.

`quit`, `ungrab`, `quitAndExit` and `controls` are reserved at Q, Z, E and O. Do not put them in
the override group. Unknown names, invalid letters and conflicts reject the entire
override set for the session, using legacy defaults without rewriting the file.
The inherited default K overlap is retained only when neither clipboard receive
nor keyboard grab is overridden: clipboard receive has priority when enabled.
To resolve it explicitly, assign keyboard grab to a free letter as above.

Preference saves preserve this group; explicit Moonlight import excludes it.
Settings → Keyboard shortcuts now provides the editor, Save and Restore defaults. Tests cover defaults, empty overrides, reserved
keys, duplicate bindings, invalid names/letters and on-disk reload/fallback.
The application-linked preference test in `tests/ui-theme` additionally checks
save/reload preservation of this group using disposable settings.


Ctrl+Alt+Shift+O opens the keyboard-only stream panel. Press a displayed number
for Disconnect, Fullscreen, Release capture, Mouse mode, Send clipboard, Receive
clipboard, Statistics or Keyboard capture; unavailable actions are omitted.
Escape or the opening chord closes it. Existing clipboard consent and host grants
still apply. Local Q/Z/E recovery chords remain available; Z always releases capture and never recaptures it. Opening releases remote
keys; panel keys and their releases are consumed locally. Focus loss or a primary
connection warning dismisses the panel. The mouse/gamepad continue their ordinary
remote behavior; the panel has no clickable controls. Disconnect followed by an
ordinary launch/resume uses the existing new-session lifecycle, without automatic
reconnect or live display reconfiguration.

O became reserved for the panel in Task 5H. An older override assigning O is now
invalid and falls back as a unit; the saved group is preserved until explicitly
saved/reset in the editor. The modifier chord remains fixed. Consumed-key tests
also cover repeat suppression, release consumption and recovery after a missing
release. Actual dead-key/IME composition and host key delivery need live testing.
