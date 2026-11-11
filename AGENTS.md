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
  Milestone 4's first increment implements per-host virtual-display launch intent,
  readiness UI and a fresh paired-metadata/driver/permission gate before new
  launches. Default-off and resume behavior preserve standard streaming; no live
  display changes are implemented. On 2026-10-28 the user reported the requested
  display tests went smoothly and authorized continued implementation. Exact
  Windows modes and the broader host matrix remain unreported coverage details.
  Milestone 5 starts with host/app grid Home/End, Page Up/Page Down and Shift+F10
  context menus, now user-accepted. Task 5B adds per-host app favorites, saved
  locally and shown first without losing selection, now user-accepted. Task 5C
  adds responsive settings columns, resize-aware keyboard scrolling and standard
  heading colors; debug build, native Wayland rendering/focus checks and
  translation coverage pass; responsive settings are now user-accepted. Task 5D
  adds an immediate, persistent light/dark Appearance selector and readable
  light-mode icons/popups. Build, focused UI/preference/import/translation and
  saved-theme startup checks pass; appearance interactive acceptance is pending.
  Task 5E excludes SDL AltGr-marked events from local shortcut matching; debug
  build and 2,048 synthetic modifier combinations pass, live layout checks pending.
  Task 5F adds persistent letter/disabled shortcut overrides with reserved Q/Z/E
  recovery keys and conflict fallback. Debug build, 5 policy/modifier cases,
  3 application-linked preference cases and 35 importer cases pass; live input
  acceptance remains pending. Tasks 5G/5H add the shortcut editor and keyboard-only
  Ctrl+Alt+Shift+O stream panel, localized in all 30 locales. Q/Z/E/O are reserved;
  Z always releases capture. Debug build, 43 offscreen + 43 native Wayland + 5
  scale2 cases, 13 overlay/Memcheck, 6 policy, 32 catalog and 35 importer cases pass;
  146-key/30-locale coverage passes. Live input, OSD scaling and controlled
  performance acceptance remain open. Milestone 6 profiles,
  SFTP desktop dispatch and default-off authenticated UUID launch are implemented.
  Debug build, 8 profile, 9 policy, 47 offscreen/47 Wayland/6 scale2 UI, 35 importer,
  32 catalog and 13 favorites checks pass on Fedora 45. Profile Memcheck reports
  zero errors. Translation coverage is 163 keys/30 locales. Live host/profile/
  transfer and controlled performance acceptance remain pending. On 2026-11-10
  the user explicitly deferred commands and authorized advancing to Milestone 7.
  The core proposal is unapplied, commands disabled and dependency pins unchanged.
  Milestone 7 native RPM/source/debuginfo packaging and local validation pass.
  Release build, 16-file RPM payload, generated ELF requirements and copied-RPM-db
  dependency/conflict dry run pass. Disposable-root lifecycle is blocked by chroot
  permissions. User-authorized installation, both installed launchers/coexistence, and removal
  are now verified. Production settings remained unchanged across launches.
  Install/upgrade/remove are verified, with unchanged Moonlight and three profiles.
  Clean Fedora 44 main/source/debuginfo/debugsource RPM construction, dependency
  transaction and container install/verify/remove also pass with one job and a
  verified 896 MiB RAM/128 MiB swap cap, no OOM kills. Visual shell grouping remains pending.
  Perigee is removed again; no publication performed.
  See Task 7A for exact status and retained release gates.
  Task 7B adapts AppImage identity/staging/tool digests and preserves inherited
  X11/XWayland-only behavior. A Fedora 45 development image builds, its help and
  rendered GUI pass, and production profiles are unchanged. User now confirms
  Apollo video/audio/keyboard/mouse work in this XWayland AppImage. Fedora 44,
  native Wayland and Sunshine remain pending. Portable import/restart, relocation
  with spaces, identity persistence and legacy-marker isolation now pass with
  unchanged production profiles; sampled peak about 652 MiB. Bundled provenance
  maps 315 ELFs to 155 RPMs/145 source RPMs and retains 179 license files, with
  14 source-package notice gaps. Full source/license clearance remains open.
  Flatpak prototype now builds incrementally with a private KDE SDK and one job.
  Private install/remove/reinstall, exact permissions, shared libraries, hidden
  native profiles, selected-file portal import and native Wayland startup pass.
  No host credentials are copied or sessions started. User now accepts
  native Wayland Flatpak Apollo video/audio/keyboard/mouse on the prototype. A clean
  corrected-manifest build/export passes with one job and a highest aggregate
  memory sample about 867 MiB. Its different binary passes isolated sandbox/import
  and native Wayland startup checks. The user also accepts Apollo video/audio/
  keyboard/mouse on this exact clean bundle.
  The harness now exports disposable XDG paths inside the sandbox shell because
  Flatpak resets --env XDG values. All three production profiles are unchanged.
  Clean output is flatpak-clean-20261110 alongside the prototype.
  The private app/cache is outside
  the project at /home/barmy/.cache/perigee-build/flatpak-20261110. Resource limits
  are 896 MiB RAM/128 MiB swap on the actual Flatpak scope. The local Fedora CI
  runner passes 271 cases plus 163-key/30-locale translation coverage. Clean
  Fedora 44 container release/debug/staging and the same checks now pass with one
  job under verified kernel 896 MiB RAM/128 MiB swap limits, no OOM kills. RPM/CI
  now explicitly request libavformat headers. CI adds a separate RPM lane and
  file/notice/ELF inventories. Legacy cross-platform packaging is manual/opt-in.
  User-authorized review branch ci/milestone-7-packaging is pushed. Fedora CI at
  d14e4b9f passes debug/release/RPM; 271 focused cases pass. The initial container
  Git ownership failure was fixed with exact workspace/submodule trust.
  The proposed CI review snapshot is at
  /home/barmy/.cache/perigee-build/ci-review-20261110. It includes required untracked
  implementation prerequisites but excludes IDE data, profiles, the deferred
  Common C patch and its local checker. The user authorized this branch/commit/push.
  The 0.2 RPM upgrade test passes with user-reported success and retained evidence
  in /tmp/perigee-rpm-upgrade-result-fUvM2Z. Perigee is removed again.
  Upstream maintenance is documented; no merge or
  publication performed. Milestone 7 remains open.
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

Conventional binary: `../perigee-build/debug/app/perigee` relative to the source root.
That build directory is absent on 2026-11-10; current verified debug binary is
`/tmp/perigee-6a-build/app/perigee`, built with `CCACHE_DISABLE=1`.
Run it with the isolated XDG/TMPDIR environment in `docs/FEDORA_BUILD.md`.
Use `INSTALL_ROOT` for staging; do not install without user authorization.
Use a separate build directory and `make release` for release validation.
Task 1C has a standalone Qt Test target; use the commands in
[tests/identity-import/README.md](tests/identity-import/README.md). It uses synthetic
credentials only. Milestone 2 adds a separate parser/state target documented in
[tests/apollo-capabilities/README.md](tests/apollo-capabilities/README.md).
Task 3A adds [session lifecycle tests](tests/session-services/README.md), using
synthetic identities and local TLS, without pairing or real hosts.
Milestone 4 adds [display launch policy tests](tests/display-launch/README.md),
using synthetic metadata without contacting or changing a host display.
Milestone 5 adds [grid navigation](tests/grid-navigation/README.md) and
[app favorites](tests/app-favorites/README.md) checks with synthetic hosts.
The [appearance checks](tests/ui-theme/README.md) cover saved settings and the
actual QML window/popups with an isolated profile and no configured host.
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
- Translate every added/changed user-facing string in the same task, using the
  original Moonlight Qt contexts/catalogs and terminology. Update all 30 TS/QM
  locales, preserve placeholders/command tokens, and run the translation checks.
  This includes CLI/startup text and notifications; do not defer localization.
  See [the translation guide](app/languages/README.md).

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


## Milestone 7 continuation, 2026-11-11

Work is isolated at /home/barmy/.cache/perigee-build/ci-milestone-7-worktree; preserve
the original checkout and its independently staged docs/UPSTREAM_MAINTENANCE.md.
The user approved the original orbit icon. SVG/provenance changes are in this
worktree; original Moonlight artwork remains attributed and unchanged. Local 0.3
RPM with approved icon passes incremental build and exact payload checks, source
SHA 5f509bb9d53f597ca3680ac17eda4ed58964dd79e3b8bbca6f6e6bba84faccde.
Branding output root: /home/barmy/.cache/perigee-build/branding-20261111.
The Flatpak branding rebuild/export/bundle and isolated sandbox/Wayland smoke
checks pass under one job/CPU and the aggregate 1 GiB guard at
/home/barmy/.cache/perigee-build/flatpak-branding-20261111. Highest combined sample
is 934137856 bytes (about 891 MiB). Native compiled-window-icon palette passes,
with unchanged production profiles. Installed GNOME visual grouping needs the
user to run /tmp/perigee-orbit-installed-check.sh (sudo password required). The prior turn
interruption sent SIGTERM during FFmpeg preparation; no compiler failure is claimed.
The wrapper now copies sources inside a dedicated manifest-local sources directory
and keeps updates disabled. Collision/no-overwrite/failure-propagation fixtures pass.
The notice audit now retains legacy COPYING/attribution files and source-matched
split-package notices: 328 files plus 13 source notice candidates from three exact
source RPMs. Raw Koji SRPMs are unsigned; header/payload digests are checked, not
publisher-signature clearance. Full corresponding-source/license review remains open.

The user now accepts the installed orbit icon and separate GNOME groups. No new
0.3 transaction log was located; keep that distinct from visual acceptance.
Commit dfbf6689 is pushed; constrained Fedora CI run 42999866715 passes debug
(271 cases) and its initial kernel limits. Release/RPM remain pending. A fresh
AppImage branding build is running at appimage-branding-20261111 outside the IDE,
with one compiler/LTO job, CPU 0 and the existing 1 GiB guard.

The fresh orbit AppImage now builds and passes XWayland/compiled-icon/portable
import/restart/relocation/identity/legacy-marker checks with all three production
profiles unchanged. Artifact SHA256 is
593d0ec74fe7d3919108af066b8f6795864cd215de7e8c750a139b1fe3d23539.
Build sample peak is 725852160 bytes (about 692 MiB). New inventory is 1848 entries/
294 ELFs; external notices cover 138 owner RPMs/132 sources plus three providers.
Full source/license clearance remains open. Fedora release now passes too; RPM is
still running. Branded AppImage/Flatpak live checks are pending user feedback via
branding-20261111/launch-orbit-check.sh.

On 2026-11-11 the user accepts Apollo video/audio/keyboard/mouse in both refreshed
packages. Capped launches exit zero at about 576 MiB AppImage/478 MiB Flatpak.
Constrained Fedora CI 42999866715 now passes debug/release/RPM, all kernel limit
assertions and 271 focused cases. Source retention for the refreshed AppImage is
running serially under the 1 GiB guard at appimage-sources-20261111 outside the IDE;
scripts/retain-rpm-sources.py records exact identities, hashes, separate signature
status and verified receipts. No source scripts execute and no release is published.

All 132 source RPMs are now retained and digest/identity checked (about 1.58 GiB
on disk); serial fetch/resume peak about 640 MiB, zero swap. Publisher signatures
remain unverified. Runtime source/build provenance and six static-library notices
are now retained; the upstream runtime LICENSE omitted linked mimalloc, whose MIT
license is included separately. AppImage staging copies packaging/appimage/runtime.
A notice-only repack preserves all original 1848 payload entries/294 ELFs and passes
isolated XWayland/portable checks. Its source/notice review is not release clearance.
The user now accepts standard Sunshine launch/resume, video/audio/input and
reconnect in the branded Flatpak; no new pairing, release or core change occurs.

The final notice-only AppImage SHA256 is
a98823a05e0bab56ae127b022880c193e3304bd60dc4ba503c378c2998d69bb1; all accepted
payload files/294 ELFs and runtime code are unchanged, with packaged help passing.
The user has Fedora 44 GNOME Wayland hardware. Its portable Flatpak test kit is
fedora44-flatpak-check-20261111/Perigee-Fedora44-check.tar under the build cache;
syntax/negative preflight pass, actual target tests need the user. It uses a
private installation and existing Flatpak profile, never copies credentials or
requests pairing, and preserves native profiles. No release is authorized.

The user now reports Fedora 44 GNOME Wayland Flatpak startup and streaming pass.
Target logs were not directly inspected; exact codec/GPU/performance remain
unreported. Review commit b684374f is pushed and CI 43036260324 is running.
A fresh approved-icon native Fedora 44 0.4 RPM build runs from clean 634-file
source at fedora44-rpm-orbit-20261111/retry outside the IDE. No old objects remained.
It uses the prior container's dependency snapshot offline, one make/LTO job/CPU,
and a verified common controller/container 896 MiB RAM/128 MiB swap budget before
compilation. Initial nested-cgroup unit lookup failed safely before the barrier;
corrected retry is active. No host or target RPM installation occurs here.

The fresh approved-icon Fedora 44 0.4 RPM now passes clean build, exact 16-file
payload/icon/notices, SRPM source/spec equality, 75 requirements and disposable
install/verify/remove. Shared controller/container combined sample peaks at
800784384 bytes (about 764 MiB), zero OOM kills. Main RPM SHA256 is
3ed8ea1c96e5528d8796c28652e837627b7be6b998af6e26c3375b2755e2579a.
Native target kit: fedora44-rpm-orbit-20261111/Perigee-Fedora44-native-check.tar
under the build cache. It needs the user's target-machine sudo/Wayland observations;
no credentials are copied or pairing requested. Latest CI 43036260324 passes all
three jobs at b684374f. Documentation evidence after that commit is uncommitted
in this isolated worktree; original checkout/index remain preserved.

On 2026-11-11 the user accepts the Fedora 44 native RPM 0.4 install/startup/icon/
video/audio/input/reconnect/removal. Target logs were not directly inspected.
The build ceiling is now 2 GiB, still one job. Controlled AppImage build is active
at appimage-controlled-20261111 outside the IDE, from a frozen 635-file source.
Its Ubuntu 22.04 workflow container and local common controller/container budget
verify 1792 MiB RAM/256 MiB swap/one CPU before setup. All dependency version refs
are preserved and asserted against exact commits; make/ninja/CMake/LTO use one job.
The IPO hook keeps optimization enabled. No new artifact/runtime pass is claimed
until the build finishes. Previous Fedora CI/native/source work remains retained.


The controlled Ubuntu AppImage now completes with documented SDK/configuration/
metadata/test retries. Qt 6.2.4 application-linked checks pass 47/0/0 after fixing
NvHTTP's XML end-of-document iteration and matching the existing Material 2 color
in the fixture. No streaming/core/trust/wire or pin changes. Corrected GUI startup/
icon/portable import/restart/relocation/identity/legacy marker and three-profile
preservation pass. Final notice-only image SHA256:
02e6b1c82758751f1a37844feb469a81cdd28c05aca53b1ca171b606791f1c77.
It preserves all 112 executable ELF files, includes 29 notice files for all 16
custom source repositories, and passes final help/runtime-byte checks. All 16
exact source archives/build patches are retained (94211745 bytes). Maximum kernel
RAM across build phases is about 1281 MiB, zero swap/OOM kills; one job/CPU remains.
Current task is the final AppImage Fedora 44 hardware check. Native RPM and Flatpak
Fedora 44 user reports remain accepted; AppImage target/live acceptance and full
Ubuntu system-library source/notice/publication clearance remain open. See the
plan's final controlled-build section; all output stays outside the original tree.

The final AppImage kit is ready at appimage-controlled-20261111/
Perigee-Fedora44-AppImage-check.tar under the cache, SHA256
28c20f23d7d9237c5baca2401fddac0af088f3e6d74c2c22b06d3943d3e453c6.
Syntax/transfer hashes/Fedora 45 refusal and 496 compiled-input source matches
pass. The user has been asked for target startup/icon and Apollo A/V/input/
reconnect. Continue independent review CI while awaiting that report.

Review commit d208482f is pushed. Fedora CI 43060338754 debug passes 271/0/0
in 11 suites; release/RPM are still pending. The unchanged AppImage's Ubuntu
ELFs map to 73 owner packages/43 source versions. All 73 package copyright files,
17 common licenses and all 43 exact system source groups (136 files/287214585
bytes) are retained outside the artifact, with .dsc identities/digests and five
repository InRelease signatures verified. See deb-audit-usrmerge and deb-sources
under appimage-controlled-20261111. Individual uploader signatures, full legal
classification, source hosting and final release acceptance remain separate.
Seven verified exited task containers were removed after a layer ENOSPC; SDK
image, source/artifacts/logs, user files and original index are preserved.


The user accepts all Fedora 44 AppImage kit checks for hash 02e6b1c...1f1c77
(startup/icon, Apollo A/V/input/reconnect and Moonlight preservation). CI
43060338754 at d208482f now passes debug/release/RPM, 271/0/0. A follow-up enables
automatic dpkg system notice collection (73 packages/43 sources, 90 notices) and
uses the same embedded appimagetool with explicit one-thread compression; source
input/overwrite/attribution fixtures pass. The old plugin had four compression
workers despite one CPU; compiler/LTO jobs were one. PNG date metadata is now
removed, with two identical renders and equal pixel/color data. Final local image
c2402b88e0a3f596d199b5f8fbf3cde4ebebf089fee417a1fae6b667fcdece1e preserves
all 112 accepted ELFs/runtime code; 692 of 693 prior entries are byte-identical.
It adds 91 notice entries.
Final XWayland/portable/icon/profile preservation smoke passes (smoke-fa1e74y9).
Current task: finish review snapshot/CI and await the already-paired native
Sunshine AppImage check requested via the unchanged Fedora 44 kit. No new pairing
or credential copying. Full source hosting/license and broader hardware gates
remain open; no release, tag, merge or Common C publication is authorized.
