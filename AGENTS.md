# Perigee agent guide

Perigee is a productivity-first, low-latency remote desktop and game-streaming
client based on Moonlight Qt. Primary target: Fedora 44 GNOME Wayland, followed
by KDE Plasma Wayland. Primary host: Apollo on Windows; standard Sunshine must
remain compatible. Stack: C++17, Qt/QML, SDL, FFmpeg, and Moonlight Common C.

## Start every session

1. Inspect `git status`; preserve unrelated user changes and initialized submodules.
2. Read [the implementation plan](docs/IMPLEMENTATION_PLAN.md), identify one task,
   and read the linked architecture/protocol/branding sections for that task.
3. Inspect the relevant source before editing. Describe scope, constraints, files,
   and verification briefly. Complete one implementation task, update its evidence,
   and stop. An approved routine task does not need another approval question.
4. Document unexpected architectural problems and propose a decision change before
   implementing a different architecture.

## Current state and gate

- Source baseline: Moonlight Qt `ac296864ace96f51ad75f78b90644bdbca76c332`.
- Tasks 1A/1B Linux identity are implemented: `perigee`, independent native and
  portable state, desktop/AppStream metadata, attributed temporary icon and disabled
  upstream artifact checks. Native/portable restart, staged metadata, process
  coexistence and Wayland/XWayland identity checks passed. Visual shell grouping
  and system install remain pending. Task 1C explicit Linux import is implemented;
  32 focused fixture checks and imported-profile startup/restart checks passed.
  User-directed Milestone 2 now implements read-only Apollo-compatible metadata
  and explicit HTTPS admin links. Debug build, 57 focused fixtures and two local
  HTTP polling/startup runs pass; live-host and interactive UI acceptance remain
  pending. Task 3A now adds a disabled worker/SDL bridge with 15 passing lifecycle
  cases, including rapid restarts and pending-reply cancellation. Live disabled-stream
  comparison remains pending. User-authorized real-host checks now cover pinned
  metadata, launch-denial handling and three disabled-service H.264 VA-API Wayland
  stream/disconnect runs; stereo initialization passed, audible audio/input and
  controlled performance remain pending. See the plan for permission limits.
  Task 3B implements opt-in clipboard with manual Ctrl+Alt+Shift+J/K and focused
  GNOME Wayland automatic wiring; services stay off by default. Other backends
  stay manual-only; unavailable or over-4-ms reads trigger session manual fallback.
  User now confirms manual clipboard works. Result feedback adds five-second
  in-stream notices through the existing status slot; warnings take priority.
  User reports all four proposed GNOME manual checks passed: Unicode both ways,
  timed notices, focus/reconnect protection and permission revoke/restore.
  User also reports automatic transfer works both ways on GNOME Wayland.
  The user confirms the remaining automatic behavior works as intended: Task 3B
  is accepted and closed for GNOME Wayland. KDE and real Sunshine coverage remain
  follow-ups, with manual-only behavior on unverified backends.
  Next implementation task: Milestone 4 display launch management; read its gates.
- Milestone 0 is done by explicit user acceptance on 2026-10-25. Native Fedora
  debug/release builds, isolated startup and later real-host streaming passed.
  Pairing remains skipped; unrun performance/secondary-platform checks remain
  coverage follow-ups rather than claimed passes. See the plan and ADR-008.
- Task 1A native debug build, startup/restart, paths, identity persistence, Moonlight
  file preservation and Qt Wayland window ID checks passed. SDL stream-window,
  live audio labels, visual shell grouping and installed launcher behavior remain
  unverified; portable and XWayland property checks passed in Task 1B.
- Do not treat source inspection, package availability, or a container build as
  successful GPU, compositor, pairing, or streaming verification.

## Read only what the task needs

- [Architecture](docs/ARCHITECTURE.md): existing components, Qt/SDL lifecycle,
  approved integration boundaries, and performance constraints.
- [Apollo protocol](docs/APOLLO_PROTOCOL.md): verified wire behavior and limitations.
- [Repository analysis](docs/REPOSITORY_ANALYSIS.md): exact references, ancestry,
  feature matrix, provenance, licensing, and risks.
- [Implementation plan](docs/IMPLEMENTATION_PLAN.md): checklist, acceptance gates,
  verification ledger, and first-task prompt.
- [Development rules](docs/DEVELOPMENT_RULES.md): implementation and review rules.
- [Fedora build](docs/FEDORA_BUILD.md): source-checked commands and pending tests.
- [Branding](docs/BRANDING.md): identity, configuration isolation, and import rules.
- [Decisions](docs/DECISIONS.md): accepted design and circumstances for revisiting it.

## Important source locations

- `app/main.cpp`, `app/path.cpp`: startup metadata, paths, logging, Qt/SDL setup.
- `app/backend/`: discovery, host records, HTTP, identity, pairing, system properties.
- `app/gui/`: QML views and C++ models; `app/settings/`: persistent preferences.
- `app/streaming/`: session lifecycle, input, audio, decoding, presentation, pacing.
- `app/deploy/linux/`, `scripts/`, `.github/workflows/`: desktop/release integration.
- `moonlight-qt.pro`, `app/app.pro`, `globaldefs.pri`: qmake configuration.
- `moonlight-common-c/moonlight-common-c/`: pinned protocol submodule; do not replace.

## Build and checks

The previously documented `scripts/collect-fedora-baseline.sh` is absent from
this checkout. Use manual instructions and evidence in `docs/FEDORA_BUILD.md`.

Linux commands, from the repository root, after installing the documented packages:

```bash
git submodule update --init --recursive
mkdir -p ../perigee-build/debug
cd ../perigee-build/debug
qmake6 ../../perigee/moonlight-qt.pro
make -j"$(nproc)" debug
```

Current binary: `../perigee-build/debug/app/perigee` relative to the source root.
Run it with the isolated XDG/TMPDIR environment in `docs/FEDORA_BUILD.md`.
Use `INSTALL_ROOT` for staging; do not install without user authorization.
Use a separate build directory and `make release` for release validation.
Task 1C has a standalone Qt Test target; use the commands in
[tests/identity-import/README.md](tests/identity-import/README.md). It uses synthetic
credentials only. Milestone 2 adds a separate parser/state target documented in
[tests/apollo-capabilities/README.md](tests/apollo-capabilities/README.md).
Task 3A adds [session lifecycle tests](tests/session-services/README.md), using
synthetic identities and local TLS, without pairing or real hosts.
`config.tests/` still holds qmake capability probes, not the app
test suite. Do not invent tests or host-authentication results.
For documentation changes, check Markdown links, source references, and
`git diff --check`; do not change build files to validate Markdown.

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

## Identity, security, and Git

- Native runtime identity: `Perigee`, executable `perigee`, Linux application ID
  `io.github.BarMy49.Perigee`. Linux stages independent desktop/AppStream/icon
  files. Portable mode uses `perigee-portable.dat` and `perigee-data/`; never
  import Moonlight credentials implicitly. Distributable packages remain Milestone 7.
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
