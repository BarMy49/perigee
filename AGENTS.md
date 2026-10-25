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
- Planning documents are present. Application code still has upstream identity.
- Milestone 0 Fedora build/runtime acceptance is pending. Do not start application
  features or Milestone 1A until its prerequisite checks pass.
- Next task: obtain and record the real Fedora baseline. Windows has no Qt tools;
  the inspected Docker daemon was unavailable and WSL had no Fedora distribution.
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

Milestone 0 helper: `bash scripts/collect-fedora-baseline.sh` on Fedora 44 writes a
private evidence report; add `--build` for frozen-source debug/release builds and
an isolated launch command. It does not install packages or run host/session tests.

Linux commands, from the repository root, after installing the documented packages:

```bash
git submodule update --init --recursive
mkdir -p ../perigee-build/debug
cd ../perigee-build/debug
qmake6 ../../perigee/moonlight-qt.pro
make -j"$(nproc)" debug
```

Current binary: `../perigee-build/debug/app/moonlight` relative to the source root.
Run it with the isolated XDG/TMPDIR environment in `docs/FEDORA_BUILD.md`; do not
install this upstream-branded baseline over an existing Moonlight installation.
Use a separate build directory and `make release` for release validation.
There is no first-party application unit-test command yet. `config.tests/` holds
qmake capability probes, not an application test suite. Do not invent test results.
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

- Planned identity: `Perigee`, executable/package `perigee`, Linux application ID
  `io.github.BarMy49.Perigee`. These are not implemented yet.
- Set independent identity before settings/path/credential initialization. Never
  overwrite Moonlight state or automatically copy its pairing credentials.
- Preserve protocol strings, discovery service, authentication algorithms,
  imported credential bytes, server pins, and GFE unique-ID compatibility behavior.
- Pairing import is explicit and separate from ordinary preference export. Copied
  credentials share host-side identity; explain that consequence.
- Require paired-certificate trust for clipboard traffic. No `VerifyNone`, secret
  logging, clipboard persistence, fabricated host commands, or permission bypass.
- Preserve GPL and dependency notices, attribution, and source provenance. Review
  licenses before copying code; keep reference checkouts outside this working tree.
- Do not create branches, commit, push, publish, or install services without user
  authorization. Do not stage or revert unrelated files.
- Report files/behavior changed, exact commands and outcomes, unavailable checks,
  risks, documentation updates, and the next task. Mark completion only after the
  task's acceptance criteria are verified.
