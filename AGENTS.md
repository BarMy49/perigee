# Perigee agent guide

Perigee is a Linux remote desktop and game-streaming client based on Moonlight Qt.
The stack is C++17, Qt/QML, SDL, FFmpeg and Moonlight Common C. Fedora 45 GNOME
Wayland is the desktop used for the verified package checks. Apollo on Windows
and standard Sunshine are supported hosts.

## Working in this repository

1. Inspect `git status` and preserve unrelated changes and initialized submodules.
2. Read [development rules](docs/DEVELOPMENT_RULES.md) and the relevant source
   before editing. Consult [architecture](docs/ARCHITECTURE.md),
   [Apollo protocol](docs/APOLLO_PROTOCOL.md) and [decisions](docs/DECISIONS.md)
   when working on their respective boundaries.
3. Run checks appropriate to the change and record what was verified. For
   documentation changes, check Markdown links and `git diff --check`.
4. Document architectural problems and review a decision change before adopting
   a different architecture.

Use the [Fedora build guide](docs/FEDORA_BUILD.md) for build commands and a
disposable configuration. Run the [focused checks](tests/README.md) for behavior
changes and use the [upstream maintenance procedure](docs/UPSTREAM_MAINTENANCE.md)
for package releases or upstream updates. Keep compilation to one job under the
documented [build resource limits](packaging/BUILD_RESOURCES.md) when working
in the constrained build environment.

The [prerelease notes](https://github.com/BarMy49/perigee/releases/tag/v6.2.0-perigee.1) record final package acceptance. Host commands
remain deferred. Keep the dependency pins and refer to [branding and import
rules](docs/BRANDING.md) for native and portable identity behavior.

## Important source locations

- `app/main.cpp` and `app/path.cpp` initialize runtime identity, paths and Qt/SDL.
- `app/backend/` holds host records, discovery, HTTP and pairing.
- `app/gui/` and `app/settings/` hold QML, models and persistent preferences.
- `app/streaming/` holds the session lifecycle, input, decoding, audio and pacing.
- `app/deploy/linux/`, `packaging/` and `.github/workflows/` contain Linux packaging.
- `moonlight-qt.pro`, `app/app.pro` and `globaldefs.pri` configure qmake.
- `moonlight-common-c/moonlight-common-c/` is the pinned protocol submodule.

## Boundaries and conventions

- Preserve upstream qmake, C++/Qt conventions, internal classes, and dependency pins.
- Isolate optional Apollo behavior through capability-driven composition. Missing
  extension metadata must preserve ordinary Sunshine behavior.
- During streaming, SDL owns the GUI thread/event loop. Keep new networking/timers
  on a Qt worker thread; use bounded, generation-tagged session messages.
- Access clipboard/window objects only on their owner thread. No GUI-thread
  network waits or new nested event loops for session integrations.
- Preserve decoder selection, hardware acceleration, zero-copy paths, audio,
  packet recovery, input ordering, and pacing. No streaming-core changes unless
  necessary, separately analyzed, documented, reviewed, and regression-tested.
- Follow existing naming/formatting locally; avoid broad refactoring, one-letter
  new names, new inline comments, and unnecessary dependencies.
- Translate every added/changed user-facing string in the same task, using the
  original Moonlight Qt contexts/catalogs and terminology. Update all 30 TS/QM
  locales, preserve placeholders/command tokens, and run the translation checks.
  This includes CLI/startup text and notifications; do not defer localization.
  See [the translation guide](app/languages/README.md).

## Identity, security, and Git

- Native runtime identity: `Perigee`, executable `perigee`, Linux application ID
  `io.github.BarMy49.Perigee`. Linux stages independent desktop/AppStream/icon
  files. Portable mode uses `perigee-portable.dat` and `perigee-data/`; never
  import Moonlight credentials implicitly.
- Set independent identity before settings/path/credential initialization. Never
  overwrite Moonlight state or automatically copy its pairing credentials.
- Preserve protocol strings, discovery service, authentication algorithms,
  imported credential bytes, server pins, and GFE unique-ID compatibility behavior.
- Linux `perigee import-moonlight --source PATH [--include-pairing]` imports only
  into fresh settings, with the same profile lock as normal startup. Default import
  excludes client credentials; optional copied credentials share host-side identity
  and unpairing effects. General preferences export is not implemented.
- Preserve anonymous staging/no-overwrite publication and owner-only credential
  files. Unsupported filesystems fail closed; no named credential staging fallback.
- Require paired-certificate trust for clipboard traffic. No `VerifyNone`, secret
  logging, clipboard persistence, fabricated host commands, or permission bypass.
- Preserve GPL and dependency notices, attribution, and source provenance. Review
  licenses before copying code; keep reference checkouts outside this working tree.
- Do not create branches, commit, push, publish, or install services without user
  authorization. Do not stage or revert unrelated files.
- Report files/behavior changed, exact commands and outcomes, unavailable checks,
  risks, documentation updates, and the next task. Mark completion only after the
  task's acceptance criteria are verified.
