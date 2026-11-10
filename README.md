# Perigee

Perigee is a productivity-first, low-latency remote desktop and game-streaming
client based on [Moonlight Qt](https://github.com/moonlight-stream/moonlight-qt).
The primary client target is **Fedora 44 GNOME Wayland**, followed by KDE Plasma
Wayland. The primary host is **Apollo on Windows**; ordinary Sunshine connections
retain the standard Moonlight protocol path.

Perigee is in development. There are no Perigee release packages advertised here.
Upstream Moonlight downloads are separate products and do not include these changes.

## Current features

- Independent Linux identity, configuration and cache: executable `perigee`,
  application ID `io.github.BarMy49.Perigee`. Existing Moonlight state is preserved.
- Explicit Linux Moonlight preference import, with pairing credentials included
  only when requested. See [identity and import](docs/BRANDING.md).
- Read-only authenticated Apollo-compatible capability information and an optional
  user-configured HTTPS administration link.
- Per-host opt-in session text clipboard. GNOME native Wayland supports focused
  automatic synchronization; other desktops use manual transfer. Slow or unavailable
  reads switch the session to manual mode. Transfers require paired trust and host
  clipboard permissions and are limited to 1 MiB of UTF-8. Text is neither logged
  nor saved.
- Default manual shortcuts: **Ctrl+Alt+Shift+J** sends local text to the host;
  **Ctrl+Alt+Shift+K** receives host text. Five-second notices report the result.
- Per-host **Display Launch Options** can request an Apollo virtual display for
  a new launch, gated by authenticated permissions and host-reported driver
  readiness. Resuming an existing application requires disabling that request.
  Primary-host tests are user-accepted; detailed host-mode and compatibility
  coverage is recorded in the implementation plan.
- Host and app grids support **Home/End**, **Page Up/Page Down** and **Shift+F10**
  to open the selected tile's context menu. **Escape** closes the menu.
- Mark an app **Favorite** in its context menu to show a star and place it first.
  Favorites are saved per host; each group keeps alphabetical order. Removing
  the favorite restores its normal position while keeping the same tile selected.
- Settings uses two columns in wide windows and one column in narrower windows,
  keeping the focused control visible while resizing or navigating by keyboard.
- Choose **Dark** or **Light** under **Settings → UI Settings → Appearance**.
  Changes apply immediately and persist across restart; dark is the default.
- Edit local action letters or disable optional shortcuts in **Settings → Keyboard
  shortcuts**. Recovery keys stay fixed; changes apply to the next stream.
- **Ctrl+Alt+Shift+O** opens keyboard-only stream controls. Press a displayed
  number for an available action or **Escape** to close. Clipboard actions retain
  their consent and permission checks. Disconnect, then launch/resume normally
  for a new session. See [shortcut behavior](tests/shortcut-modifiers/README.md).
- Editable **Programming/Gaming** session profiles preserve global preferences;
  changes apply to new GUI sessions. Explicit CLI stream options remain independent.
- Configure a per-host **SFTP** destination to open it in a desktop handler. No SSH
  service or credentials are provisioned. Optional **Use application UUID** requires
  authenticated host support and fresh pinned identity checks.
- Inherited hardware decoding, codec, audio, gamepad and mouse/keyboard features.
  Actual availability depends on the host, GPU, drivers and build.

Enable **Session Text Clipboard** from the PC tile context menu before connecting.
Automatic synchronization establishes initial baselines without copying existing
clipboard contents and operates only while the stream is focused. Text only:
images, file transfer and background Wayland access are outside this feature.

Milestone 0, Task 3B and the primary-host display tests are user-accepted.
Milestone 5 adds desktop keyboard navigation, app favorites, responsive settings
light/dark appearance, shortcut editing and a keyboard-only stream panel.
Milestone 6 profiles, SFTP and gated UUID launch are implemented locally; the user
deferred commands and authorized Milestone 7. Live input, panel readability and
controlled streaming performance acceptance remain pending.
KDE, real Sunshine and broader controlled
performance/hardware coverage remain documented follow-ups. See the
[implementation plan](docs/IMPLEMENTATION_PLAN.md) for evidence and current scope.

## Build on Fedora

Install the dependencies documented in [Fedora development](docs/FEDORA_BUILD.md)
(Qt 6, SDL2 compatibility development files, FFmpeg and the listed libraries).
From a checkout named `perigee`:

```bash
git submodule update --init --recursive
mkdir -p ../perigee-build/debug
cd ../perigee-build/debug
qmake6 ../../perigee/moonlight-qt.pro
make -j2 debug
./app/perigee
```

Use the isolated XDG/TMPDIR launch environment in the Fedora guide when testing
with a disposable profile. Normal launch uses independent Perigee state; it does
not automatically import a Moonlight connection or pairing identity. Portable
mode uses `perigee-portable.dat` and `perigee-data/`.

Build release in a separate directory with `make -j2 release`. The qmake project
retains its upstream filename. Windows/macOS build machinery remains inherited;
the current Perigee validation target is Linux/Qt 6. See the
[upstream build documentation](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/README.md)
for those platforms; their Perigee behavior is not claimed as verified.

## Development and tests

Read [AGENTS.md](AGENTS.md), the [architecture](docs/ARCHITECTURE.md) and
[development rules](docs/DEVELOPMENT_RULES.md) before changing session behavior.
Standalone Qt test targets have their own build/run instructions:

- [Identity/import](tests/identity-import/README.md)
- [Apollo capabilities](tests/apollo-capabilities/README.md)
- [Display launch policy](tests/display-launch/README.md)
- [Grid keyboard navigation](tests/grid-navigation/README.md)
- [App favorites](tests/app-favorites/README.md)
- [Appearance](tests/ui-theme/README.md)
- [Session services and clipboard](tests/session-services/README.md)
- [Timed notifications](tests/overlay-notifications/README.md)
- [Translations](tests/translations/README.md)

Perigee retains Moonlight's 30 language catalogs. New UI, CLI and notification
strings are translated with each change; see [the translation workflow](app/languages/README.md).

The checked-in CLion **Perigee Test Profile** launcher runs the existing debug
binary. It references the original developer's disposable profile outside the
checkout; adjust its environment paths for your own machine. It does not build
before launch or contain pairing credentials.

## License and attribution

Perigee derives from Moonlight Qt, baseline
`ac296864ace96f51ad75f78b90644bdbca76c332`, and retains upstream attribution,
[GPL licensing](LICENSE) and pinned dependency notices. The temporary icon is
upstream artwork; see [asset provenance](ASSET_NOTICES.md). See
[repository analysis](docs/REPOSITORY_ANALYSIS.md) for source provenance and
[protocol notes](docs/APOLLO_PROTOCOL.md) for supported Apollo behavior.

Moonlight's community, translations and package infrastructure belong to the
[upstream project](https://moonlight-stream.org). Perigee is a separate fork.

Milestone 7 distribution readiness is the current implementation stage. Commands
are explicitly deferred and disabled; the Common C pin is unchanged. Local checks
pass for the selected Milestone 6 features, while live-host, transfer and performance
acceptance remain pending in the [implementation plan](docs/IMPLEMENTATION_PLAN.md).
