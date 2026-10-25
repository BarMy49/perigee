# Implementation plan and handoff

Approved plan date: 2026-10-25. This document is the task/status entry point; read
[AGENTS.md](../AGENTS.md), the relevant [architecture](ARCHITECTURE.md),
[protocol](APOLLO_PROTOCOL.md), [branding](BRANDING.md), and
[decision](DECISIONS.md) sections before application work.

## Current state

- Source is based on Moonlight Qt `ac296864ace96f51ad75f78b90644bdbca76c332`
  with unchanged dependency pins. Task 1A implements Linux native runtime identity
  and output `perigee`. Task 1B now adds Linux desktop/install metadata, portable
  isolation, visible labels and disabled upstream artifact-update checks.
- Tasks 1A/1B/1C implement independent Linux identity, portable/desktop metadata
  and explicit import. New import/profile-store components are separate from
  protocol and streaming code; dependency pins are unchanged.
- User selected Fedora 44 GNOME Wayland first, KDE Wayland second,
  `io.github.BarMy49.Perigee`, optional explicit pairing-identity import, and per-host
  opt-in bidirectional session text clipboard synchronization.
- Fedora 44 GNOME Wayland debug and release builds passed after the user installed
  development packages. Two isolated Wayland startup/restart runs also passed;
  local identity persisted and checked native Moonlight files were unchanged.
  Milestone 0 is closed by user acceptance below. Earlier
  Windows and missing-package limitations are historical evidence.
- The earlier handoff reported a baseline helper at
  `scripts/collect-fedora-baseline.sh`, but it is absent from this checkout. Use
  manual documented commands; earlier helper validation does not establish availability.
- **User-directed gate exception, 2026-10-25:** user requested skipping pairing
  tests and starting Milestone 1. Proceed with Task 1A without claiming Milestone 0
  passed. Other real-session/performance checks remain unverified.
- **Task 1B implementation and staged/native/portable checks are recorded below.**
  Visual shell grouping remains a manual acceptance item; no system install or
  publication was performed. Task 1C explicit import and focused fixtures now
  pass local checks. The user explicitly requested Milestone 2 on 2026-10-25;
  its read-only foundation now passes local checks below. Pairing/host-authentication
  tests remain skipped; live Sunshine/Apollo and earlier acceptance gaps remain pending.
- Existing user `TODO.md` and unrelated IDE state must be preserved. The user
  authorized committing/pushing project work after 3B completion; that condition
  is now satisfied by the acceptance below. Do not provision services or perform
  broad source renaming.

## Milestone summary

| Milestone | State | Dependency / gate |
| --- | --- | --- |
| Documentation materialization | Complete | Required documents and evidence checks passed |
| 0: Baseline | Done, accepted by user | Pairing skipped by user; unrun measurements remain recorded as coverage follow-ups |
| 1A: Runtime identity isolation | Implemented; native Linux acceptance passed | User-authorized baseline exception; host pairing skipped |
| 1B: Desktop/portable/package identity | Implemented; staged/runtime checks passed; visual shell grouping pending | 1A acceptance |
| 1C: Explicit Moonlight import | Implemented; local fixtures/runtime checks passed; host authentication skipped | Isolated native identity and import fixtures |
| 2: Apollo foundation | Implemented; fixtures/build/local polling pass; live-host acceptance pending | User-directed progression; identity isolation and ADR-002 |
| 3A: Session service bridge | Implemented; local lifecycle checks pass; live disabled-stream comparison pending | User-directed 3A; capability values and ADR-003 |
| 3B: Text clipboard | Done, user-accepted on GNOME Wayland | Manual and focused automatic sync; other backends manual-only; secondary-platform coverage follow-ups retained |
| 4: Display launch controls | Not started | Capabilities and baseline |
| 5: Productivity UI and controls | Not started | Existing actions, ownership/performance checks |
| 6: Optional integrations | Not started | Feature-specific evidence and prerequisites |
| 7: Distribution and maintenance | Not started | Complete identity plus compatibility matrix |

## Documentation delivery

- [x] Root `AGENTS.md` as a concise recurring entry point.
- [x] Existing/target architecture, diagrams, ownership and design boundaries.
- [x] Apollo protocol/authorization and unsupported-path evidence.
- [x] Repository revisions, ancestry, selected changes, matrix, licensing and risks.
- [x] Development rules and approved decision log.
- [x] Fedora source-checked build/run/debug/package instructions and limitations.
- [x] Branding/storage/import/coexistence design.
- [x] Milestones, acceptance, testing strategy, evidence ledger and agent handoffs.
- [x] Verify all nine files, local links/anchors, pinned Git source references and
  whitespace; record results in the verification ledger.

## Milestone 0: baseline

**Goal:** prove the frozen upstream code builds and behaves reproducibly before
Perigee application changes. **State: done by explicit user acceptance on
2026-10-25.** Pairing tests remain skipped by user direction. Historical evidence
and unrun checks below are retained rather than relabeled as executed passes.

**Scope/dependencies:** no application/dependency changes; supporting baseline
automation is allowed. Fedora 44 GNOME Wayland, real client GPU,
Windows Apollo and a standard Sunshine test host. Collect versions/permissions and
use an isolated configuration instead of the production Moonlight identity.

**Relevant sources:** `moonlight-qt.pro`, `app/app.pro`, `globaldefs.pri`,
`.gitmodules`, and existing `Session`/decoder/input paths for diagnosis only.

- [x] Baseline accepted and closed by the user; documented manual commands were
  used. The absent optional helper is not restored or claimed to have run.
- [x] Verify source/pins and resolve/document Fedora packages; installation confirmed.
- [x] Build debug and release using [FEDORA_BUILD.md](FEDORA_BUILD.md).
- [ ] Record actual decoder/renderer and basic video/audio/keyboard/mouse operation.
- [ ] Test pairing persistence (skipped by user), launch/resume,
  disconnect/reconnect and network loss (unverified).
- [ ] Verify Apollo permissions and ordinary Sunshine operation independently.
- [ ] Record comparable release performance/resource observations and variance.
- [ ] Complete [baseline result record](FEDORA_BUILD.md#baseline-result-record).

**Coverage follow-ups, not closure blockers after user acceptance:** unchecked
items above remain unrun/partial; pairing is intentionally skipped. The instruction
to mark this milestone done does not fabricate hardware or performance evidence.

**Acceptance:** reproducible successful build and real GNOME Wayland sessions on
both host types, selected hardware acceleration verified, working audio and input,
recovery evidence, isolated state and measured baseline. Broader hardware/KDE/X11
coverage may continue in the matrix but cannot be claimed from GNOME alone.

**Closure evidence:** native debug/release builds and isolated startup passed;
later real-host runs exercised 1080p H.264 VA-API/EGL-GLES streaming and reconnect,
and the user accepted manual and automatic clipboard behavior. The user explicitly
requested marking Milestone 0 done. See [the result record](FEDORA_BUILD.md#baseline-result-record)
for actual measurements and historical limitations. Real Sunshine/KDE, controlled
release performance variance and other unchecked coverage remain follow-ups;
closure is user acceptance, not a claim that those checks were executed.

## Milestone 1: Perigee identity and isolation

### Task 1A: runtime identity isolation

**Goal:** smallest useful first coding task: uninstalled Linux `perigee` with
independent runtime settings/cache identity. **Depends on:** Milestone 0 passed
or the recorded user gate exception (used on 2026-10-25).

**Files:** `app/main.cpp`, `app/app.pro`. Keep this first task limited to these
startup/build surfaces; any genuinely necessary expansion must be explained.

- [x] Set Linux target `perigee` without changing qmake or dependency names.
- [x] Set Qt organization `io.github.BarMy49`, application `Perigee`, organization
  domain `barmy49.github.io` before paths/preferences/identity initialization.
- [x] Set Qt desktop identification and SDL Wayland/X11 IDs to
  `io.github.BarMy49.Perigee`; use Perigee SDL application/audio labels.
- [x] Use independent log creation and cleanup prefixes.
- [x] Verify clean native startup, settings/cache path, credentials across restart,
  unchanged Moonlight state, and matching runtime desktop identity.

**Expected behavior:** binary `app/perigee`, native settings under
`$XDG_CONFIG_HOME/io.github.BarMy49/Perigee.conf`, fresh independent client identity.
No import or installation. Stream-title/translated text and portable migration are
deliberately later tasks; do not test this increment in a portable.dat directory.

**Tests/acceptance:** debug build; disposable clean XDG profile; verify file paths
and safe credential fingerprints before/after restart; compare Moonlight settings
and pairing files byte-for-byte without exposing them. No GameStream, library or
streaming-source changes. Record every unavailable check as pending.

**Evidence, 2026-10-25:** `qmake6 ../../perigee/moonlight-qt.pro` and
`make -j2 debug` exited 0 in `../perigee-build/debug`, producing `app/perigee`.
Logs: `task1a-qmake.log`, `task1a-build.log`, `task1a-ldd.log`; no unresolved linked
libraries. Frozen baseline release output is preserved in its separate directory.

Private verification artifacts are under
`/home/barmy/.local/state/perigee-baseline/task1a-20261025-lwbmkcuu`:
`result.json`, two `startup-*.log` files with Wayland protocol evidence,
`startup-default-cache.log`, and an isolated `launch.sh`. Two fresh-profile runs
survived 15 seconds, exited 0 after SIGTERM, and loaded QML without component-load
failure. The observed Wayland top-level app ID was `io.github.BarMy49.Perigee`.
Native settings were owner-only (0600); all three identity fields persisted across
restart and differed from the Moonlight baseline. All 544 compared existing
Moonlight native/baseline files and a synthetic Moonlight settings sentinel remained
unchanged. A third run without a QML cache override exited 0 and created 12 QML
cache files under `$XDG_CACHE_HOME/io.github.BarMy49/Perigee/qmlcache`.

**Limits:** pairing skipped by user; no live stream/audio label verification,
X11 WMCLASS observation, or visual taskbar grouping acceptance. Qt Wayland window
identity is observed; SDL IDs/audio hints are source-checked. Native Linux logs to
stderr, so the file-log prefix/cleanup branches are source-reviewed rather than
runtime exercised. Existing startup probe/QML/GTK warnings remain. Portable mode,
installed desktop metadata, UI/stream text, and update-feed changes belong to 1B.
Do not install or launch this increment from a `portable.dat` directory.

### Task 1B: complete desktop and portable identity

**Goal:** coherent installable identity after runtime isolation.

**Relevant sources:** Linux deploy files and `app.pro` install rules; startup/path,
CLI/QML visible branding, stream title, resources and update checker.

- [x] Align desktop entry, AppStream ID/launchable/binary, icon basename, WMCLASS,
  installed paths and user-facing Perigee labels.
- [x] Retain attribution and verify asset terms before any attributed temporary icon
  reuse; original artwork remains an explicit later task.
- [x] Isolate portable marker and every portable config/cache/log directory.
- [x] Disable upstream artifact-update announcements for Perigee until a dedicated
  feed exists; assess mapping/compatibility endpoints separately.
- [x] Stage install files and validate metadata; test Moonlight/Perigee coexistence.

**Acceptance:** no Moonlight-owned installed path/state modified; native/portable
restart and grouping pass; desktop/AppStream/icon references resolve; no wrong
artifact-update path. Session-title edit is cosmetic only. No mass replacements.
**Remaining acceptance:** visual GNOME/KDE taskbar grouping and installed
launcher/icon-cache behavior have not been observed; staged identity/property
checks passed. Original artwork is a later task. No RPM/AppImage/Flatpak release
or cross-platform acceptance is claimed.

**Implementation/evidence, 2026-10-25:**

- Linux install rules stage only `perigee`, its full-ID desktop/AppStream/icon
  files and `share/doc/perigee/{LICENSE,ASSET_NOTICES.md}`. Old upstream metadata
  is retained in the source for non-Linux targets; it is not in the Linux manifest.
  Icon content is unchanged from upstream; source/license/history review and
  attribution are recorded in [ASSET_NOTICES.md](../ASSET_NOTICES.md).
- Linux recognizes only `perigee-portable.dat`; config, cache, boxart, qmlcache and
  logs are rooted under `perigee-data/`. Legacy `portable.dat` and generic Moonlight
  directories are left intact. Native and portable identities are independent.
- CLI and QML product labels use Perigee; upstream help/community/tool references
  retain attribution. Changed translation source strings fall back to English
  until new translations are supplied. Session edits are limited to visible error
  text, title and icon resource; event-loop/input/decode/render logic is unchanged.
- Linux `AutoUpdateChecker` neither creates its network manager nor compiles the
  upstream request path, including AppImage/Steam Link Linux builds. Runtime logs
  report disabled artifact checks; the upstream update URL is absent from the
  tested ELF. Existing mapping and compatibility HTTPS endpoints retain their
  separate data roles; connectivity/STUN and upstream help URLs are unchanged.
- `qmake6 ../../perigee/moonlight-qt.pro PREFIX=/usr` and `make -j2 debug` passed
  in `../perigee-build/debug`. Because the root qmake invocation did not propagate
  PREFIX to the app, ran `qmake6 ../../../perigee/app/app.pro PREFIX=/usr` and
  `make -f Makefile.Debug -j2` in `debug/app`, then
  `make -f Makefile.Debug INSTALL_ROOT=/home/barmy/Projects/perigee-build/task1b-stage-4ce4q2ot install`.
  All exited 0. Debug logs are `task1b-*.log` under `debug/` and `debug/app/`.
  The frozen baseline release directory was not rebuilt.
- Staged manifest is `/home/barmy/Projects/perigee-build/task1b-stage-result.json`:
  six files, no collisions with `rpm -ql moonlight-qt` (installed version
  `6.2.0-1.fc44`). `desktop-file-validate` and
  `appstreamcli validate-tree --no-net <stage>/usr` passed. AppStream pedantic
  notes are the user-approved mixed-case component ID and missing release history;
  no Perigee release/screenshot was fabricated. Resource references exist, icon
  bytes match upstream, and `ldd` reports no missing shared libraries.
- Private runtime evidence:
  `/home/barmy/.local/state/perigee-baseline/task1b-20261025-7jcnowps/result.json`.
  Installed Moonlight and staged Perigee ran with disposable shared XDG roots.
  Two native and two portable Perigee runs stayed alive nine seconds, loaded QML
  without component failure and exited 0 after SIGTERM. Local identity tuples
  persisted across restart and differed between Moonlight/native/portable profiles.
  Portable runs left native settings/cache unchanged, populated all five portable
  directories and created 12 portable QML cache files. Legacy portable sentinels,
  Moonlight test state and all 512 checked production files remained unchanged.
  Both applications also remained alive together for eight seconds and exited 0;
  their identities were unchanged. This is process/state coexistence, not a session test.
- Initial X11 check found Qt WM_CLASS `perigee`/`Perigee`; `RESOURCE_NAME` is now
  set before Qt initialization. Final X11 properties report instance
  `io.github.BarMy49.Perigee`, class `Perigee`, and matching full-ID
  `_GTK_APPLICATION_ID` / `_KDE_NET_WM_DESKTOP_FILE`. This preserves QSettings
  identity while matching StartupWMClass. Qt behavior was checked against
  [Qt 6.11.2 XCB source](https://github.com/qt/qtbase/blob/v6.11.2/src/plugins/platforms/xcb/qxcbintegration.cpp).
  Final native/portable Wayland runs emit full-ID `set_app_id`, exit 0 and log
  disabled updates. No native X11 desktop or visual taskbar grouping was tested;
  the X11 check used XWayland within GNOME Wayland.
- `perigee --help` exited 0 with Perigee command examples. New private launchers
  `launch-native.sh` and `launch-portable.sh` are beside the runtime result.
  Linux continues logging to stderr; portable log directory configuration is
  source-reviewed, not a claim that Linux file logging was enabled. Existing
  startup driver/codec-probe warnings remain. Pairing/live session tests are skipped
  or unverified as previously recorded.


### Task 1C: explicit import

**Goal:** retain selected settings/hosts and optionally pairing without overwriting
either application's existing identity. **State:** implemented on Linux; local
acceptance passed, real-host authentication skipped by user instruction.

**Files:** `app/settings/moonlightimporter.{h,cpp}`, `profilestore.{h,cpp}`,
early `app/main.cpp`, global CLI help and `app/app.pro`; independent
`tests/identity-import/` target. No identity-manager, host-protocol, dependency or
streaming-source changes in Task 1C.

- [x] Implement `perigee import-moonlight --source PATH [--include-pairing]` before
  normal settings/identity consumers, with the [approved contract](BRANDING.md#explicit-import-contract).
- [x] Use an allowlist, shared startup/import destination lock, validated credential
  tuple/host pins, protected atomic no-overwrite publication, source immutability.
- [x] Add disposable native/portable, corrupt input, existing/in-use destination,
  concurrency, interruption and credential-excluding default-import fixtures.
  There is no ordinary preferences-export feature in this source; none was added
  or falsely reported as tested.
- [ ] Real host authentication: skipped by user instruction. Shared host-side
  identity/unpairing consequences are explained by CLI help/success and branding docs.

**Acceptance/evidence, 2026-10-25:**

- In `../perigee-build/debug/app`,
  `qmake6 ../../../perigee/app/app.pro PREFIX=/usr` and
  `make -f Makefile.Debug -j2` exited 0. Logs are `task1c-qmake.log` and
  `task1c-build.log`. The frozen baseline release and prior Task 1B stage were
  preserved; current output is `debug/app/perigee`.
- New standalone test commands, from `../perigee-build/tests/identity-import`:

  ```bash
  qmake6 ../../../perigee/tests/identity-import/identity-import.pro
  make -j2
  PERIGEE_TEST_BINARY=/home/barmy/Projects/perigee-build/debug/app/perigee ./tst_identityimport -o results.txt,txt
  ```

  All exited 0; **32 passed, 0 failed, 0 skipped** under Qt/QtTest 6.11.2 and
  GCC 16.2.1 on Fedora 44. Synthetic credentials only. Cases include default
  credential exclusion, byte-preserving explicit tuples and CRLFs, invalid/encrypted/
  mismatched keys, corrupt/oversized inputs, host/app/pin/range validation,
  backup precedence, duplicate UUIDs, zero-byte/symlink/existing destinations,
  held locks, no-overwrite publication races, SIGKILL before publication and stale
  lock recovery. Concurrent headless CLI imports allow exactly one winner;
  invalid arguments do not create settings. CLI cases disable all display access
  and use an invalid GUI platform, proving import bypasses GUI initialization.
- Private runtime artifacts:
  `/home/barmy/.local/state/perigee-baseline/task1c-20261025-01q1rd_5/result.json`.
  A readonly synthetic source was imported into three disposable profiles:
  default native, optional-identity native and optional-identity portable. All
  import commands exited 0. Each profile ran twice for eight seconds on Wayland
  and exited 0 after SIGTERM, without QML component failure. Default import
  omitted the client tuple; first startup generated fresh credentials and restart
  preserved them. Both explicit imports retained decoded certificate/key bytes
  and client ID across startup/restart with no credential regeneration.
- The initial runtime harness compared encoded INI lines and flagged Qt's text
  normalization. A private Qt Core verifier (`verify-values.cpp` / `verify-values`)
  checks actual QSettings byte/string values instead; all three expectations pass.
  The result record retains the initial text-comparison outcome. This was a
  harness correction, not a credential change or a claimed byte-identical INI file.
- While the imported native GUI was active, both import and a second normal startup
  exited 1 with the in-use profile diagnostic; the active settings file was
  unchanged. Source bytes/mode stayed unchanged (0400), destination files are 0600,
  and all 512 checked production files stayed unchanged. No production credential
  was imported, host unpaired, installation performed or artifact published.
- Import schema/bounds, backup selection, supported filesystems, atomic publication,
  lock lifecycle and safe exit messages are documented in
  [branding](BRANDING.md#implemented-import-validation-and-storage). Qt Test
  compilation retains the existing GCC/Qt header SFINAE warning; no app compile
  errors or new runtime architecture were introduced.

**Limits:** no real Apollo/Sunshine authentication, permission or virtual-display
identity test; these remain skipped/unverified as directed. Local PEM consistency
is not host acceptance. Visual shell grouping from 1B remains a manual follow-up.
Non-Linux import, general preference export and merge/overwrite are not implemented.

## Milestone 2: Apollo foundation

**Goal:** conservative capability values with unchanged standard Sunshine behavior.
**Relevant sources:** `NvHTTP`, `NvComputer` update path, existing GUI models; new
small parser/value components. **Dependencies:** identity baseline and ADR-002.

- [x] Parse authenticated optional permissions, ordered commands and Windows display
  flags. Distinguish absent/invalid/unauthenticated/denied values.
- [x] Expose read-only state and a user-configured HTTPS admin link. Do not mutate
  permissions or require an Apollo product-version field.
- [x] Establish a focused separate qmake/Qt Test target for new pure parsing/state
  behavior; keep product build independent of test/live-host dependencies.
- [ ] Verify standard-host fixtures and real Sunshine generate no extension probes.

**Acceptance:** deterministic parser tests, correct read-only Apollo state and
unchanged normal host operations. **Blockers:** no versioned capability spec; keep
the UI honest about Apollo-compatible capabilities rather than inventing identity.

### Milestone 2 implementation and local evidence (2026-10-25)

User explicitly requested this milestone despite the pending real-session baseline
and remaining Milestone 1 manual checks. This authorizes the implementation scope;
it does not make those acceptance checks pass.

- `ApolloCapabilities` parses direct optional serverinfo fields into bounded,
  ephemeral values. Permissions require a successful same-origin pinned HTTPS
  reply and paired status. Missing, invalid, unauthenticated and denied remain
  distinct. Unknown permission bits survive; ordered/duplicate command names are
  preserved only with authenticated command permission. Display availability is
  separate from authorization. Product/version guessing is absent.
- `NvHTTP` records reply provenance without changing existing request/fallback or
  TLS policy. `NvComputer` replaces capability snapshots on updates; polling
  invalidates trusted values when the host becomes offline. Values are not saved.
- `ComputerModel`/`PcView.qml` expose a read-only plain-text snapshot panel and an
  explicit per-host HTTPS browser link. The link is validated, stored independently
  of capabilities and addressed by host UUID. Empty input removes it. No generated
  admin port, embedded credentials, query/fragment, automatic visit or admin writes.
  The Moonlight import allowlist does not import this new local setting.
- New standalone [Qt Test target](../tests/apollo-capabilities/README.md):
  `qmake6 ../../../perigee/tests/apollo-capabilities/apollo-capabilities.pro`,
  `make -j2`, `./tst_apollocapabilities -o results.txt,txt` in
  `../perigee-build/tests/apollo-capabilities`: **57 passed, 0 failed/skipped**.
  Fixtures cover strict XML/uint32 validation, ordering, provenance policy, stale
  replacement/invalidation, standard-host absence and HTTPS admin URL limits.
- Debug app: `qmake6 ../../../perigee/app/app.pro PREFIX=/usr` and
  `make -f Makefile.Debug -j2` in `../perigee-build/debug/app`: exit 0 on
  Fedora 44, Qt 6.11.2, GCC 16.2.1. Logs: `milestone2-qmake.log`,
  `milestone2-build.log`, `milestone2-build-final.log` in that directory.
- Import regression: `PERIGEE_TEST_BINARY=/home/barmy/Projects/perigee-build/debug/app/perigee
  ./tst_identityimport -o results-milestone2.txt,txt` in
  `../perigee-build/tests/identity-import`: **32 passed, 0 failed/skipped**.
- `python3 ../perigee-build/milestone2-runtime.py`: two disposable 12-second
  Wayland GUI runs against local HTTP fixtures (standard fields only, then
  untrusted advertised extensions), each exit 0 after SIGTERM. Each received four
  `/serverinfo` requests and no extension probes; no QML load/reference errors.
  A configured admin URL was not visited automatically. All **512** checked
  production Moonlight/Perigee files were unchanged. Private evidence:
  `../perigee-build/milestone2-dc32yu22/summary.json` and per-case startup logs.

`git diff --check` and explicit new-file whitespace checks pass; all 90 local
Markdown links/anchors resolve. Recursive dependency pins remain unchanged.

**Pending acceptance:** real Sunshine/Apollo polling, authenticated TLS/permission
changes and revocation, interactive panel/link save/restart/browser checks, and
normal live host/session behavior. A real host address/version was requested but
not supplied. Fixtures and local HTTP polling do not establish these results.
No clipboard, command execution, display-launch controls, dependency-pin changes
or streaming-core changes were made for Milestone 2. The final acceptance checkbox
remains open until real Sunshine evidence exists.

## Milestone 3: clipboard integration

### Task 3A: session service bridge

**Goal:** optional asynchronous services without moving the streaming/render loop.
**Relevant sources:** `Session` start/teardown and SDL user-event dispatch; new
`SessionServices`, worker transport and bounded bridge. **Dependencies:** capability
values and [ownership design](ARCHITECTURE.md#data-flow-and-contracts).

- [x] Configure worker-owned Qt network/timers from immutable session snapshots.
- [x] Deliver owned bounded messages to SDL with registered wakeups and generations.
- [x] Invalidate/cancel/drain before cleanup; do not access raw mutable host/session
  pointers from workers or wait for blocked GUI-thread slots.
- [x] Stress start failure, rapid disconnect/reconnect and shutdown with replies
  pending; verify no stale action, leaked worker/reply or use-after-free.

**Acceptance:** lifecycle tests pass; feature-disabled streaming retains baseline
behavior and no new event-loop or renderer architecture. **Blocker:** teardown and
clipboard ownership must be proven, not copied from another fork.

### Task 3A implementation and local evidence (2026-10-25)

The user explicitly requested Task 3A after Milestone 2. Earlier real-host, UI and
baseline acceptance gaps remain pending; this implementation does not close them.

- New `streaming/services/SessionServices` owns a const value snapshot, QThread,
  worker and bridge. The worker creates/uses/destroys its QNetworkAccessManager,
  replies and timeout timer on its event-loop thread. It has no host, Session,
  window or clipboard pointers and never queues work to the GUI thread.
- The owner permits only one outstanding GET, including its queued invocation and
  unconsumed result. The generic transport is scaffolding for subsequent services:
  explicit same-origin path, paired snapshot, client credentials, pinned HTTPS,
  no proxy/fallback/redirect, bounded response and timeout. It issues no requests
  until called. It is not a clipboard implementation or Apollo endpoint probe;
  operation-specific permission/consent/status policy remains Task 3B.
- `SessionEventBridge` owns at most 16 messages, 64 KiB per payload and 256 KiB in
  aggregate. A coalesced registered SDL wakeup carries no pointers. Registration
  reserves a type distinct from upstream's fixed `SDL_USEREVENT`; invalidation
  never flushes frame/controller events. Generations reject stale publications.
  Failed wakeups fail publication without retaining an undeliverable message.
- Owner-thread `stop()` invalidates/discards before queuing worker cancellation,
  aborts pending replies/timers on the worker and joins that thread. It never waits
  for a GUI slot. Worker deletion and pending reply deletion complete on the worker.
  This follows the existing single-active-session semaphore contract.
- `Session::start()` snapshots the locked host/endpoint/certificate/capabilities
  and client TLS identity. Services are **disabled** in the product; no worker,
  timer, registered bridge wakeup or service network traffic is started. Existing
  exec/start-failure/window-failure/SDL-exit cleanup paths release service ownership
  before video/window teardown and deferred Common C cleanup. The SDL dispatcher
  can drain owned messages; no clipboard/window action is implemented.
- Streaming impact is limited to those lifecycle/dispatch hooks. No changes to
  input forwarding, decoder selection, media callbacks, recovery, rendering,
  audio or pacing algorithms; no new GUI event pump or nested loop. Live media
  performance and feature-disabled session equivalence still require a host.

**Commands and evidence:**

- `qmake6 ../../../perigee/app/app.pro PREFIX=/usr`, then
  `make -f Makefile.Debug -j2` in `../perigee-build/debug/app`: exit 0,
  Fedora 44 / Qt 6.11.2 / GCC 16.2.1. Logs: `task3a-qmake.log`, `task3a-build.log`.
  Existing Qt/GCC header warnings remain; dependency pins are unchanged.
- Separate [session-services Qt Test target](../tests/session-services/README.md):
  `qmake6 ../../../perigee/tests/session-services/session-services.pro`, `make -j2`,
  `./tst_sessionservices -o results.txt,txt` in
  `../perigee-build/tests/session-services`: **15 passed, 0 failed/skipped**.
  Includes 200 start/stop cycles, 30 cancellations with real local TLS replies
  pending, wrong pins, connection failure, successful-empty/oversized/redirect
  responses, bounded queue ownership, generation rejection and a producer race.
  SDL-only waiting verifies worker startup and timeout progress without GUI events;
  shutdown likewise does not pump GUI events. Temporary synthetic identities and
  loopback TLS fixtures do not perform pairing or verify a real host.
- `valgrind --tool=memcheck --leak-check=full
  --show-leak-kinds=definite,indirect --errors-for-leak-kinds=definite,indirect
  --error-exitcode=99 --log-file=valgrind.log ./tst_sessionservices
  -o results-valgrind.txt,txt`: exit 0, **15 passed**, **0 memory errors**;
  definitely/indirectly/possibly lost: **0 bytes**. Still-reachable allocations:
  569,104 bytes; no suppression file used. Evidence is in the test build directory.
  The initial instrumented run exceeded a 1-second TLS fixture deadline; the
  non-timeout fixture deadline was raised to 10 seconds before this passing run.
- `python3 ../perigee-build/milestone2-runtime.py` rerun after the final app build:
  both 12-second isolated Wayland startups exit 0 after SIGTERM, no QML errors,
  four ordinary `/serverinfo` requests per case and no extension probes. **512**
  production files are unchanged. Evidence:
  `../perigee-build/task3a-runtime-result.json`, with private per-case logs in
  `../perigee-build/milestone2-tpzwnni3/`.

`git diff --check`, explicit new-file whitespace checks and all 96 local Markdown
links/anchors pass. Recursive dependency pins remain unchanged.

**Pending acceptance:** actual Session start/connection/window failure and repeated
feature-disabled streaming against real Sunshine/Apollo, performance comparison,
media/input regression and compositor clipboard ownership/latency. Synthetic bridge
lifecycle stress is verified; it cannot establish those hardware/session results.
Task 3B remains unimplemented and must add explicit consent/permissions, clipboard
operations, revision/coalescing state and platform-adapter evidence before enablement.

### Supervised real-host checks after Task 3A (2026-10-25)

The user configured the isolated test profile and briefly connected, then explicitly
authorized using that connection for testing. No new pairing/import, host admin
write, permission bypass, clipboard access, test keystroke or host-app quit was
performed. Host product/release was requested but is not yet confirmed; advertised
fields establish Apollo-compatible metadata, not product identity.

- The user's initial short session logged 1920×1080 H.264, VA-API decode and the
  EGL/GLES renderer on native Wayland, stereo audio initialization and a quit event.
  These logs establish stream setup, not audible audio or input correctness.
- A private headless probe links the existing debug `NvHTTP`, identity, parser and
  service object files. Three ordinary serverinfo polls used the configured pin
  and client identity, returned paired status and authenticated capability values;
  `/applist` returned three apps. Traced paths were only HTTP `/serverinfo` for
  HTTPS-port discovery, HTTPS `/serverinfo` and `/applist`; no extension probes.
  The probe left the test settings byte-for-byte unchanged.
- Initially the live mask was `0x03000000` (list/view). A fresh Desktop `/launch`
  returned **403 Permission denied**, consistent with missing launch permission.
  The client handled this real start failure and exited 0 after SIGTERM without
  requiring a forced kill. Subsequent polling observed `0x07000000`, adding launch,
  without re-pairing. No host permissions were changed by the agent. This verifies
  fresh authenticated observation of a changed grant, not revocation or live UI
  refresh. Keyboard/mouse/clipboard/server-command bits remain absent; command
  state is denied with no names. Display-capable/driver-ready flags are available
  and true, independently of those permissions; driver functionality is untested.
- Three real `SessionServices` worker start/GET/stop cycles completed on pinned
  HTTPS, with SDL-only waiting and no GUI event pumping. Each stopped and drained.
  These deliberately bare `/serverinfo` GETs omit the normal GameStream query
  fields and returned metadata that the parser classified **unauthenticated**;
  no capabilities were accepted from them. They verify real transport progress
  and cleanup, not equivalence to authenticated NvHTTP metadata. Operation-specific
  request construction remains part of Task 3B; no generic request was enabled in
  the application and no clipboard endpoint was contacted.
- After launch became available, three consecutive CLI Desktop streams each ran
  for roughly 12 seconds after renderer selection, then disconnected via SIGTERM.
  Traced stream commands were `/resume` in all three runs; successful fresh
  `/launch` after the grant change is not established. All **exit 0**, log a quit
  event and **1920×1080×60 H.264**, choose **VA-API with
  EGL/GLES** and initialize **two-channel audio**. No forced kills. The product
  service enable flag remains false throughout. No `/cancel` host-app quit was sent.
- Logged incoming rates were 47.70 / 45.95 / 33.38 FPS; network drops 0% and jitter
  drops 0.38 / 0.39 / 0.53%; average decode 1.01 / 3.38 / 4.41 ms, frame queue
  0.14 / 0.16 / 0.12 ms and render 2.49 / 2.45 / 2.46 ms. Workload was uncontrolled
  desktop content. These observations do **not** establish sustained 60 FPS or a
  performance regression result; no comparable before/after release runs exist.
- All **512** checked production Moonlight/Perigee files remained unchanged.
  Only the user-authorized isolated test identity was used; no identity values,
  host address, pairing material or input encryption keys are recorded here.

**Exact reproduction/evidence:** from the source root,
`bash ../perigee-build/live-host-check/build-probe.sh`,
`python3 ../perigee-build/live-host-check/run-metadata.py`, and
`python3 ../perigee-build/live-host-check/stream-check.py` all exit 0. The stream
helper invokes `../perigee-build/debug/app/perigee stream <test-host-UUID> Desktop
--display-mode windowed --capture-system-keys never --no-quit-after` with this
profile's disposable XDG/TMPDIR roots, real runtime sockets and Qt/SDL Wayland.
Private profile/evidence root: `../perigee-build/interactive-test-wwiyzrvg/`;
`metadata-check.json`, `grant-observations.json`, `stream-check-result.json`,
`stream-check-summary.json`, `stream-check-{1,2,3}.log` and original `startup.log`.
The later successful runs supersede the stream helper's initial denied attempt;
its 403 observation is retained in `grant-observations.json`.

**Still pending:** confirmed host product/version and separate standard Sunshine
coverage, successful fresh launch, controlled frozen/release baseline comparison,
permission revocation,
interactive capability/admin-link UI acceptance, audible audio/input/focus tests,
long sessions/recovery, and GNOME/KDE clipboard ownership/latency. The last
Milestone 2 acceptance checkbox and overall baseline gates remain open. Task 3A
now has real start-failure and repeated disabled-stream evidence; full regression
and performance acceptance is not claimed. `git diff --check` and all 97
local Markdown links/anchors pass.

### Task 3B: secure text synchronization

**State: done for the primary GNOME Wayland target by user acceptance, 2026-10-25.**
Other backends retain manual fallback; outstanding secondary-platform coverage is
tracked separately below rather than claimed as tested.

**Goal:** manual send/receive plus per-host opt-in bidirectional session text sync.
**Sources:** new clipboard controller/adapter/HTTPS operations; existing SDL
clipboard/input path and preferences/QML settings. **Depends on:** 3A and Windows
Apollo paired permissions/active session.

- [x] Implement raw UTF-8 GET/POST, pinned trust, redirect refusal, timeout and bounded
  receiving; distinguish empty success from failure.
- [x] Add content revisions/echo suppression, one in-flight operation, latest-write
  coalescing, cancellation and no initial automatic clipboard upload.
- [x] Measure focused GNOME SDL reads and verify access; add session manual
  fallback for slow/unavailable reads and manual-only policy on other backends.
  No PRIMARY, payload logs or persisted content.
- [x] Add protocol/state fixtures for the transport and controller.
- [x] Wire focused GNOME Wayland automatic observation/polling with session manual
  fallback; integrated GNOME behavior is user-accepted.
- [x] Complete user-verified GNOME permission/focus/reconnect acceptance.
- [ ] Coverage follow-up: execute the real KDE/manual-fallback and standard
  Sunshine matrix when those environments are available.

**Acceptance:** [protocol contract](APOLLO_PROTOCOL.md#text-clipboard-contract) passes,
no unauthorized traffic/loops/stale overwrites, Sunshine unaffected, no unexplained
input/video stalls. **Blockers:** compositor restrictions and SDL retrieval latency;
images and Linux-host placeholders are outside this milestone.

### Task 3B implementation and local evidence (2026-10-25)

The user authorized Task 3B. The manual session path and controller are implemented;
automatic SDL reads remain gated, so full Task 3B acceptance is **not complete**.
At initial implementation verification the real host reported `0x07000000`: no
clipboard grants. The later user-attempt review below records newly granted bits. Host product/version and Windows status remain unconfirmed;
the user was asked for these prerequisites. No host administrative writes or
real clipboard reads/writes were performed by the agent.

**Implemented behavior:**

- Explicit per-host `clipboardenabled` consent, false by default, is stored in the
  host record and exposed through a host-context-menu confirmation dialog. It is
  independent of pairing and is not in the Moonlight import allowlist. Changes
  take effect on the next stream. The dialog explains manual operation and the
  unresolved Wayland automatic-access gate. No payload or hash is saved.
- Session snapshots enable services only with consent, paired authenticated
  capabilities, view-or-launch permission and at least one clipboard direction.
  Each operation separately checks its direction before any local SDL read or
  network dispatch. Standard hosts/opted-out hosts retain their existing shortcut
  behavior and issue no clipboard probes. Existing type-text paste is unchanged.
- On a focused streaming window, **Ctrl+Alt+Shift+J** sends local text and
  **Ctrl+Alt+Shift+K** explicitly receives current host text. Nothing transfers on
  connection establishment. No remote key/paste action is synthesized.
- `SessionServices::clipboardRequest()` builds exactly GET/POST
  `/actions/clipboard?type=text` on the snapshot's paired HTTPS endpoint. POST uses
  `text/plain; charset=utf-8` and raw UTF-8 bytes; HTTP status remains separate from
  success/body. Existing pin checking, client TLS identity, redirect refusal,
  five-second worker timeout and cancellation remain in force; no fallback.
- Transfer validation permits up to **1 MiB UTF-8 bytes**, rejects malformed or
  truncated UTF-8 and embedded NULs (SDL uses C strings), and preserves Unicode,
  BOM and newline bytes. Receiving is bounded during reads. The owned bridge now
  permits 1 MiB per message / 4 MiB aggregate, with the existing 16-message cap.
- `ClipboardSync` serializes requests and coalesces pending sends to the latest
  value, tracks revisions and session generations, suppresses echoes, preserves
  successful empty reads, and rejects replies after a local revision change or
  focus loss. Authorization/session denial (401/403) and definitive unsupported
  statuses (404/405/410/501) stop operations until reconnect. Transient failures
  impose a five-second backoff; pending writes remain in memory for retry.
- Automatic controller logic has one-second scheduling, local/remote baseline
  establishment without initial upload/overwrite, and echo suppression, verified
  in fixtures. The product constructs the **manual** controller: no automatic SDL
  read or poll source is enabled on any unmeasured platform. Wiring automatic
  adapter reads/poll wakeups requires measured GNOME/KDE/X11 evidence first.
- `SdlClipboard` isolates focused-window clipboard access on the SDL/GUI owner
  thread. It logs read duration only, validates size/UTF-8 and reports write
  failure without content. Manual receives recheck local content and pending SDL
  clipboard-change events before applying a reply. No Qt clipboard access, PRIMARY,
  image/HTML/file-transfer support or GUI network waits were added. SDL itself
  retrieves an OS text allocation before the client can enforce its outgoing byte
  bound; this is another reason automatic retrieval remains gated.

**Local verification:**

- `qmake6 ../../../perigee/app/app.pro PREFIX=/usr` and
  `make -f Makefile.Debug -j2` in `../perigee-build/debug/app`: exit 0 on Fedora 44,
  Qt 6.11.2 / GCC 16.2.1; `task3b-qmake.log`, `task3b-build.log`.
- In `../perigee-build/tests/session-services`, regenerate the separate qmake target,
  `make -j2`, then `./tst_sessionservices -o results-3b.txt,txt`: **16 passed,
  0 failed/skipped**. The added case covers exact URL/method/body, empty success,
  Unicode/newlines/BOM, malformed/truncated UTF-8, NUL/oversize rejection, local
  revision protection, latest-write coalescing, initial automatic baselines,
  echo suppression and authorization stop, alongside the 3A lifecycle coverage.
- The documented Valgrind command with `--log-file=valgrind-3b.log` and
  `-o results-valgrind-3b.txt,txt`: **16 passed**, exit 0, **0 memory errors** and
  **0 definitely/indirectly/possibly lost bytes**, no suppressions. Earlier
  instrumented attempts failed with fixture deadlines and a read reported in the
  fixture's regex header parsing. The fixture now parses byte headers directly;
  the passing run does not establish an external-library root cause.
- `PERIGEE_TEST_BINARY=/home/barmy/Projects/perigee-build/debug/app/perigee
  ./tst_identityimport -o results-3b.txt,txt` in the importer test directory:
  **32 passed, 0 failed/skipped**.
- `python3 ../perigee-build/milestone2-runtime.py`, with result in
  `../perigee-build/task3b-runtime-result.json`: both isolated Wayland GUI fixture
  runs exit 0, no QML errors and only four `/serverinfo` requests each. All **512**
  checked production files remain unchanged. Private startup logs are under the
  evidence directory named by that result file.

- `python3 ../perigee-build/live-host-check/stream-check-3b.py`: one further
  consent-disabled real-host resume ran for approximately 12 seconds after renderer
  selection, logged 1920×1080 H.264 / VA-API / EGL-GLES and stereo initialization,
  then exited 0 after SIGTERM. No forced kill, clipboard request or manual-read log;
  512 production files unchanged. Private evidence: `stream-3b-result.json` and
  `stream-3b-1.log` in the user-created test profile. The uncontrolled desktop
  workload is not a performance acceptance comparison.

`git diff --check`, explicit new-file whitespace checks and all 100 local Markdown
links/anchors pass. The final debug GUI was reopened in the same isolated test
profile for user review (`manual-clipboard-ui.log`), with no QML load errors.
Consent was not enabled by the agent and no clipboard contents were accessed.

### User-attempt log review (2026-10-25)

The user requested checking the reopened test GUI logs. `manual-clipboard-ui.log`
contains two 1920×1080 H.264 / VA-API / EGL-GLES stream setups and two quit events,
with no QML errors. It contains **zero manual clipboard-read markers**, no adapter
unavailable/write-failure markers and no evidence confirming a clipboard operation.
The saved profile has no `clipboardenabled` setting, which defaults to false.
This establishes that clipboard operation was not confirmed; missing worker HTTP
log lines alone would not prove absence of traffic because payload-safe worker
requests are not logged by default.

Fresh paired/pinned metadata (existing read-only probe) now reports **0x07131f00**,
including both clipboard direction grants. The earlier host-grant blocker has
therefore cleared. Local opt-in and a new focused manual shortcut attempt are the
next checks: enable Session Text Clipboard (Manual) in the PC context menu,
reconnect, then Ctrl+Alt+Shift+J/K using synthetic test text. No setting was changed
or clipboard content read by the agent during this review. Private evidence:
`clipboard-log-review.json` and `manual-clipboard-ui.log` in the test profile.
End-to-end transfer and SDL latency/focus acceptance remain unverified.

### Clipboard confirmation follow-up (2026-10-25)

The user reports manual clipboard transfer now works and requested visible
confirmation for roughly five seconds. The test profile now has explicit opt-in.
The latest `clipboard-menu-startup.log` contains **92** focused SDL read timings,
all reported as **0 ms** at integer-millisecond resolution; this means below the
reported resolution, not zero latency. Payloads/directions were not recorded, so
this user-confirmed smoke result does not establish the full clipboard matrix.
No agent action read or changed clipboard content during this follow-up.
Private summary: `clipboard-confirmation-preflight.json` in the test profile.

- `ClipboardSync` now exposes a consumed, owner-thread outcome value. Send success
  is produced only after a successful host reply. Receive success is shown only
  after valid non-stale data is accepted by the focused SDL clipboard adapter.
  Queued, stale-generation and initial-baseline operations do not fabricate success.
- `Session` maps results to translated, content-free notices: sent, received,
  denied/unsupported, failed, invalid text, local write failure and skipped receive.
  Result diagnostics record these fixed labels, never clipboard content, hashes,
  server response bodies, credential material or query strings.
- `OverlayManager` reuses the existing status renderer slot, with a cached 24-point
  white outlined font and a five-second deadline. New results replace the notice
  and refresh its deadline; identical text refreshes without allocating/rendering
  a new surface. Connection and gamepad-mode warnings retain priority. Expiry
  never hides those primary messages. A hidden notice is visible only if the
  primary clears before its original deadline.
- Expiry runs on the existing SDL owner loop, including idle waits bounded by the
  remaining deadline. No GUI Qt timer, nested event loop, new renderer overlay type,
  DRM plane, video readback or continuous text rendering was added. Text is rendered
  only for relevant state changes; existing decoder/pacing/audio paths are untouched.
  Notifications end with the session and have no delayed window callback.

**Verification:**

- `make -f Makefile.Debug -j2` in `../perigee-build/debug/app`: exit 0;
  `clipboard-notification-build.log`. Fedora 44, Qt 6.11.2, GCC 16.2.1.
- New standalone [overlay notification checks](../tests/overlay-notifications/README.md):
  qmake6/make and `./tst_overlaynotifications -o results.txt,txt` in
  `../perigee-build/tests/overlay-notifications`: **6 passed, 0 failed/skipped**.
  Real font/surface creation, exact deadline/replacement, unchanged-text refresh,
  warning priority and tick wraparound are checked without wall-clock sleeps.
- `make -j2` and `./tst_sessionservices -o results-feedback.txt,txt` in
  `../perigee-build/tests/session-services`: **16 passed, 0 failed/skipped**,
  including success only after completion, consumed outcomes, stale receive and
  denial classifications, alongside the existing transport/lifecycle checks.

- Overlay lifetime command from its README (`--log-file=valgrind.log`,
  `-o results-valgrind.txt,txt`): exit 0, **6 passed**, **0 memory errors** and
  **0 definitely/indirectly/possibly lost bytes**; 28,647 bytes still reachable,
  no suppressions. No production host/clipboard is used by this target.
- Updated debug GUI reopened with the existing opted-in test profile; startup log
  `clipboard-feedback-startup.log` has no QML load errors. No clipboard operation
  or host setting was triggered by the agent. User can review the notice on the
  next manual send/receive.

**Limits/next:** actual notice placement/readability on the live stream still needs
user review. Automatic/background clipboard access, focus-loss/revocation and the
remaining GNOME/KDE matrix remain gated. This is a small 3B feedback follow-up,
not completion of Milestone 5's command panel or overall streaming acceptance.

**Remaining Task 3B work/acceptance:** confirm Windows Apollo/version and verify
the full synthetic manual transfer matrix (general manual success is user-confirmed),
including empty/multibyte/large content, host
revocation, focus changes and reconnect on real GNOME and KDE. Measure SDL read
latency and compositor access before wiring/enabling automatic reads and poll
wakeups for any backend; retain manual fallback where restricted. Real Sunshine
and controlled media/input/performance comparisons remain pending. Do not start
Milestone 4 or claim seamless automatic synchronization from these local fixtures.

### Task 3B completion and publication checkpoint (2026-10-25)

The user authorized summarizing, committing and pushing the work to GitHub **after
Task 3B is complete**. That condition is not yet satisfied: automatic integration
and the live acceptance matrix above remain outstanding. No commit or push was
performed at this checkpoint; the existing `master` branch targets
`https://github.com/BarMy49/perigee.git`. Preserve the unrelated staged `TODO.md`
deletion and `.idea/` files when preparing the eventual commit.

Reverification of the current sources/build:

- `make -f Makefile.Debug -j2` in `../perigee-build/debug/app`: exit 0,
  already up to date.
- `PERIGEE_TEST_BINARY=/home/barmy/Projects/perigee-build/debug/app/perigee
  ./tst_identityimport -o results-precommit.txt,txt`: 32 passed, 0 failed.
- `./tst_apollocapabilities -o results-precommit.txt,txt`: 57 passed, 0 failed.
- `./tst_sessionservices -o results-precommit.txt,txt`: 16 passed, 0 failed.
- `./tst_overlaynotifications -o results-precommit.txt,txt`: 6 passed, 0 failed.
- `git diff --check`: exit 0 before this documentation update.

Test commands ran in their respective external `../perigee-build/tests/`
directories. Results use synthetic fixtures; no live clipboard content was read
or changed. `/usr/share/wayland-sessions` contains GNOME and GNOME Classic only;
KDE acceptance cannot be inferred from this environment. Host Windows/Apollo
version, access to a KDE session and standard Sunshine host, and availability for
synthetic live clipboard checks were requested. These outstanding checks remain
pending rather than waived by the publication instruction.

### Task 3B live feedback review and interactive handoff (2026-10-25)

The current isolated test client is still running. Review of
`clipboard-feedback-startup.log` found one `Clipboard sent to host` outcome
(host acknowledgement) and one `Clipboard received from host` outcome (successful
focused SDL application). Three manual reads each reported 0 ms at integer-ms
resolution, with no adapter-unavailable, local-write-failure or QML-load-error
markers. This confirms both operation paths, but not transferred bytes, notice
visibility or the full acceptance matrix. No clipboard payload was inspected or
changed by this review. Private content-free evidence is
`clipboard-feedback-review.json` in the isolated test profile.

Next interactive checks, using disposable text only:

1. Copy `Perigee test: zażółć 😀` locally, focus the stream and press
   Ctrl+Alt+Shift+J. Paste into a host text editor and compare exactly. Confirm the
   send notice is readable and disappears after approximately five seconds.
2. Copy different disposable Unicode text on the host, press Ctrl+Alt+Shift+K,
   then paste into a local editor and compare. Confirm the receive notice.
3. Request a receive and immediately switch to a local editor and copy a new
   marker. Report any late overwrite of that marker. Repeat after disconnecting
   and reconnecting; report unexpected clipboard changes at reconnect or stalls.
4. Revoke clipboard access in Apollo while streaming, attempt a transfer, and
   confirm a denial rather than a success notice. Restore the grants and reconnect
   before checking that transfers work again. Record which direction was revoked.

Also obtain Windows/Apollo versions and availability of KDE Wayland and a standard
Sunshine host. KDE and Sunshine tests need those environments; local fixtures do
not replace them. Automatic synchronization wiring remains agent implementation
work after platform evidence, not a manual test the user can currently perform.

### CLion test-profile launcher (2026-10-25)

User requested a CLion run configuration to reopen the test client. Added
`.run/Perigee Test Profile.run.xml` as a custom Application configuration, using
the local custom target in `.idea/customTargets.xml` and the existing
`../perigee-build/debug/app/perigee` executable. No build-before-run action is
configured. XDG config/cache/data, TMPDIR and the QML cache point to the existing
isolated interactive profile, preserving its saved connection without copying
credentials into IDE configuration. Qt and SDL select Wayland; desktop session
environment is inherited.

CLion's configuration discovery recognizes `Perigee Test Profile`; execution
through its run API succeeds. The resulting live process was verified to use the
expected executable and isolated XDG config path, with no QML component-load
failure. The GUI was left running. `git diff --check` passes. This launcher does
not change Task 3B acceptance or enable automatic synchronization. Existing IDE
settings and the staged `TODO.md` deletion were preserved.

### Task 3B user-verified manual acceptance (2026-10-25)

The user reports that the four interactive tests proposed above passed on the
current GNOME Wayland test setup. Record these as **user-verified passes**:

- Exact disposable Unicode text in both directions.
- Readable send/receive notices lasting approximately five seconds.
- Focus/local-change protection and disconnect/reconnect without the unexpected
  overwrite or stalls described in the test instructions.
- Permission revocation denies transfer; restoration and reconnect recover it.

The report does not identify which direction was revoked, a host product/version,
or additional KDE/Sunshine environments. Do not infer those details. A passing
focus-switch attempt also does not prove every network race was exercised.

Content-free review of the latest CLion log
`ij_run__Perigee_Test_Profile_186831251918805453.log` independently found two send
acknowledgements, three receives applied locally, eight manual read timings all
reported as 0 ms at integer-ms resolution, and no QML component-load failure.
No denial outcome was present in that log, so revocation acceptance comes from
the user's report rather than an independently observed HTTP status. No payload
was read or changed during review. No application code changed in this update;
`git diff --check` passes.

The GNOME manual handoff is satisfied. Next implementation task: wire focused
GNOME automatic synchronization within the approved SDL-owner/worker boundary,
with no initial upload, serialized bounded operations, revision/echo protection,
focus gating and manual fallback. Measure the integrated automatic path and
verify its lifecycle before declaring it accepted. KDE, standard Sunshine, host
version details and the broader acceptance gaps remain pending. Task 3B is still
partial; its completion-conditioned commit/push has not been performed.

### Task 3B focused GNOME automatic integration (2026-10-25)

The user requested proceeding with automatic implementation after reporting the
four manual checks passed. This supersedes earlier statements that automatic
wiring is absent; full Task 3B acceptance remains pending.

- Native SDL Wayland plus an exact GNOME desktop token enables the automatic
  controller for eligible, consented hosts. KDE, X11 and other backends retain
  manual J/K. The consent dialog now explains automation and fallback.
- A worker-owned one-second Qt timer publishes pointer-free, generation-tagged
  `ClipboardTick` messages. Duplicate ticks coalesce, sharing the bounded bridge
  with completions without filling it during a stalled owner. No networking or
  Qt timer runs on the SDL/GUI thread.
- The SDL owner observes only while focused. The first local read and first
  eligible remote poll establish baselines without uploading or replacing existing
  contents. Later changes use the existing serialized transport, latest-write
  coalescing, UTF-8 bounds, echo suppression and stale-revision checks.
- Focus loss drops queued writes, invalidates reads and stops dispatch. Actual SDL
  focus is checked again when draining worker messages. A reply skipped because
  focus changed does not consume a known remote change; a later focused poll can
  recover it. A local change still wins over an older pending receive.
- Direction-specific grants are checked before queueing changes. Receive-only
  clients cannot become stuck behind a forbidden automatic upload. Denial or
  definitive unsupported status stops automatic ticks until reconnect. Transient
  failures retain the existing five-second backoff, with ticks driving retries.
- All automatic-mode reads, including reply-time local checks and explicit J/K,
  are timed. A read unavailable/invalid or exceeding a chosen **4 ms API-call
  budget** switches the session to manual, clears queued changes and shows a
  fallback notice. This runtime cutoff is not an empirical acceptance threshold
  or a claim that all compositors meet a latency guarantee. The first slow read
  has already blocked; safe owner-thread access cannot cancel it retroactively.
  A failed local write also disables automation, preventing an old local value
  from being sent back over a remote change that could not be applied.
- Only the first automatic read duration, mode and fixed fallback/result labels
  are logged. No clipboard text/hash is persisted or logged; no PRIMARY, renderer,
  decoder, audio or transport algorithms were changed. Worker timer cancellation
  stays inside existing invalidate/drain/join teardown.

Verification on Fedora 44 / Qt 6.11.2 / GCC 16.2.1:

- `make -f Makefile.Debug -j2` in `../perigee-build/debug/app`: exit 0. Build
  output is `clipboard-automatic-build.log` there.
- `qmake6 ../../../perigee/tests/session-services/session-services.pro`,
  `make -j2`, then `./tst_sessionservices -o results-automatic.txt,txt` in the
  external session-services test directory: **22 passed, 0 failed**. New cases
  cover platform gating, tick coalescing, worker ticks without GUI processing,
  focus-loss/recovery, fallback and both one-direction permission configurations.
- The documented Valgrind command with `--log-file=valgrind-automatic.log` and
  `-o results-valgrind-automatic.txt,txt`: exit 0, **22 passed**, **0 errors**,
  zero definitely/indirectly/possibly lost bytes, 569,181 bytes still reachable,
  no suppressions. The final focus-recovery correction was included in this run.
- Import, capability and notification regressions: **32**, **57**, **6** passed
  respectively, all zero failures, in `results-automatic.txt` under their external
  test directories. Total local checks: **117 passed**.
- CLion `Perigee Test Profile` launches the final build with the existing isolated
  connection and native GNOME Wayland environment. No QML component-load failure;
  the GUI was left running. No stream was started or OS clipboard accessed by the
  agent. Its private log is
  `ij_run__Perigee_Test_Profile_5026369990607271078.log` in CLion's temporary directory.
- `git diff --check`: exit 0. All **103** checked local Markdown file links resolve.

**Next live test:** connect from the reopened client, allow initial baselines to
settle, and verify pre-existing local/remote text stays unchanged. Copy new
disposable Unicode text locally, refocus the stream and paste remotely without J;
copy different text remotely, keep the stream focused and paste locally without K.
Verify no echo loop, newer local overwrite or visible input/video stall. Repeat
focus loss/reconnect and permission revoke/restore in automatic mode; the earlier
manual-mode passes do not substitute for these integrated checks. Check the first
automatic read timing and any fallback warning in the log. A fallback notice means
automatic acceptance failed on that run; J/K should still work. KDE and real
Sunshine remain separate unavailable checks, and host version details remain
unconfirmed. No Task 3B completion or conditional commit/push is claimed yet.

### Task 3B automatic transfer smoke acceptance (2026-10-25)

After the automatic integration handoff, the user reports that transfer works
both ways. Record focused GNOME automatic local-to-host and host-to-local
transfers as **user-verified passes**. This is acceptance of the new automatic
path, distinct from the earlier manual J/K checks.

The inspected CLion capture files contain no new automatic-mode/read/result
markers, so this review does not independently establish automatic latency or
operation counts. No clipboard payload was inspected, no application code changed,
and no new test run was needed for this evidence-only update.

Still pending: automatic-mode initial-baseline, focus/local-change protection,
reconnect and permission revoke/restore confirmation, plus the separate KDE and
real Sunshine environments and host version details. Earlier manual-mode checks
passed, and automated state/transport fixtures cover these invariants locally;
neither is a substitute for the remaining live automatic checks. Do not mark full
Task 3B complete or perform the completion-conditioned commit/push yet.

### Milestone 0 and Task 3B closure (2026-10-25)

Following confirmation that automatic transfer works both ways, the user reports
that the remaining proposed behavior works as intended and explicitly requests
Milestone 0 be marked done. Record initial-baseline preservation, automatic
focus/local-change and reconnect protection, and permission revoke/restore as
user-verified primary-target acceptance. Together with prior manual checks,
117 passing local checks, debug builds and clean session-service Memcheck,
this closes **Task 3B for Fedora GNOME Wayland**. Milestone 0 is **done by explicit
user acceptance**. Pairing remains skipped as instructed.

This acceptance supersedes historical pending/conditional-publication statements
above. No new test execution or independently measured automatic latency is
claimed by the user's report. Real KDE and standard Sunshine, host version details,
controlled release performance and remaining broader hardware/input/audio coverage
stay documented follow-ups. Keep other platforms manual-only until verified.

The user's earlier instruction to summarize, commit and push after 3B completion
now applies. Publish project source/tests/docs and the requested CLion launcher
(including its custom target); exclude the unrelated staged `TODO.md` deletion
and all other IDE/private runtime files. The next implementation task is
Milestone 4 display launch management, subject to its own prerequisites.

Publication verification: debug `make -f Makefile.Debug -j2` exits 0, already up to
date; the latest four suite reports total 117 passes with zero failures; all 104
checked local Markdown links/anchors resolve; `git diff --check` passes. Dependency
pins are unchanged. The 70-file publication set contains no private runtime files
or private-key/token material detected by the focused scan.

## Milestone 4: display launch management

**Goal:** verified Apollo virtual-display/resolution/refresh launch controls.
**Sources:** `NvHTTP::startApp()`, capability values, preferences and settings view.
**Dependencies:** capabilities and real host display baseline.

- [ ] Add per-host virtual-display intent and host driver/permission information.
- [ ] Append only verified launch parameters for supporting hosts; preserve ordinary
  dimensions/FPS and Sunshine requests.
- [ ] Test missing driver, denied permission, existing display, launch/resume and
  no-active-session cases; distinguish stream resolution from host display mode.
- [ ] Keep unsupported live reconfiguration/physical monitor switching unavailable.
  Validate fractional host refresh/pacing separately before exposing it as reliable.

**Acceptance:** correct observed host behavior and clear limitations, no disruption
to another active session, standard-host regression pass. **Blockers:** no verified
general live/physical-display API; reconnect is not proof of reconfiguration.

## Milestone 5: productivity UI and in-stream controls

**Goal:** workstation-oriented navigation, input configuration and accessible controls.
**Sources:** existing `app/gui/` views/models, `StreamingPreferences`, input actions
and `OverlayManager` hooks. **Dependencies:** relevant integration capability/state.

- [ ] Incrementally improve host/app/desktop selection, favorites, settings layout,
  light/dark behavior, keyboard navigation, spacing and native decorations.
- [ ] Expose configurable local/remote shortcuts with an emergency escape action;
  test key releases, AltGr/Windows keys/dead keys/IME and focus transitions.
- [ ] Add a keyboard-accessible small OSD command panel for existing disconnect,
  fullscreen, capture/mouse, clipboard and stats actions; reconnect uses a controlled
  new session lifecycle. Unsupported host/display controls remain unavailable.
- [ ] Validate scaling, pointer transitions, no video readback/continuous QML scene,
  feature-disabled overhead and repeated performance comparisons.

**Acceptance:** normal desktop navigation/input/control scenarios pass without
renderer/pacing regression. **Blockers:** compositor shortcut policy, overlay input
capture and lifetime. Tray and richer pointer UI are optional separate increments.

## Milestone 6: additional integrations

Each item is an independent task; do not implement them as one change.

- [ ] Profiles: reuse preference snapshots, editable Programming/Gaming presets,
  no mutation of active global state. Test session/profile isolation and restart.
- [ ] SFTP: open a user-configured `sftp://` destination through desktop integration;
  no SSH provisioning or plaintext secret storage. Test unsupported/missing handlers.
- [ ] Application UUID: parse/preserve host extensions and launch only supported
  hosts; standard numeric ID remains the compatibility path. Test both hosts.
- [ ] Commands: separately review an additive Common C patch and distributable pin,
  host-list-only uint8 indices, permission/session gating and sent/failed status.
  Test wire bytes, bounds, stale/missing list and inactive session. No guessed APIs.
- [ ] Investigate images, native transfer, multimonitor and enhanced VRR only with
  new host/hardware evidence and a separately reviewed decision where necessary.

**Acceptance:** each selected item has its own documented behavior and passing tests.
**Blockers:** host APIs do not implement images/native transfer; command dependency
publication/provenance must exist before a reproducible release can include it.

## Milestone 7: distribution and maintenance

**Goal:** reproducible independent packages and sustainable upstream updates.
**Sources:** deploy metadata, scripts and CI; new RPM/Flatpak packaging reviewed
against then-current dependencies. **Depends on:** complete identity and acceptance.

- [ ] Native RPM `perigee`, staged manifest, dependency resolution and install/upgrade/
  uninstall checks without Moonlight conflicts/overwrites.
- [ ] Adapt existing AppImage build; independent artifact names and update channel.
- [ ] Create independent Flatpak ID/manifest with tested GPU/audio/input/network and
  file-selection permissions; no broad home access for import.
- [ ] CI for debug/release, new focused automated tests, metadata and license/source
  inventory; supervised hardware matrix remains separately recorded.
- [ ] Dedicated upstream merge changes: record baseline/pins, inspect conflict-heavy
  files, rerun fixtures and real pairing/stream/input/performance regression gates.

**Acceptance:** tested packages coexist with Moonlight, publish accurate features,
have complete provenance/source access, and support a documented update procedure.
**Blockers:** driver/codec/sandbox differences and access to representative hardware.
No publication, commits or service provisioning without authorization.

## Verification strategy

Automate meaningful new pure logic and mock-transport behavior with a separate Qt
Test/qmake target when that milestone begins. Task 1C now provides a focused importer/profile-store Qt Test target; other
feature suites remain planned. Do not list hypothetical commands as runnable. Existing qmake configure
probes test build capabilities, not authentication or runtime correctness.

| Area | Automated scenarios | Real integration/hardware scenarios |
| --- | --- | --- |
| Host capabilities | Missing/malformed/unknown fields, HTTP vs HTTPS masks, ordered/duplicate commands | Apollo/standard Sunshine, grants/revocation |
| Clipboard | UTF-8/empty/limits, TLS mismatch, 400/401/403/404/500, timeout, echoes, local-edit races, cancellation | GNOME/KDE focus, owner changes, active session, repeated reconnect |
| Import/identity | Allowlist/types, matching tuple, corrupt/incomplete input, destination race/existence, atomic failure, source unchanged | Fresh/imported pairing, virtual-display identity, coexistence |
| Input | Mapping/release state where separable, local command consumption | Windows/Super/AltGr/Alt+Tab/Alt+F4, Polish text, dead keys/IME, mouse capture, emergency ungrab |
| Display | Query gating/parameters, ordinary-host requests | 1080p/1440p/4K, scaling/high refresh, multiple local monitors, Apollo driver lifecycle |
| Video/audio | Existing build/probe results; no new pipeline logic initially | Actual hardware codec/chroma/depth, VA-API/Vulkan, HDR/VRR where supported, audio |
| Commands | Packet/list bounds/send semantics | Advertised permitted host command; never destructive implicit retries |
| Recovery/resources | Old generations, failure cleanup, pending requests | Network interruption, repeated sessions, memory/thread/resource use |
| Packaging | Metadata/manifest/ID validation | RPM/AppImage/Flatpak, install/upgrade/remove, GPU/sandbox and Moonlight coexistence |

For performance compare identical release builds/settings/network/workloads with
features disabled and enabled; record repeated decode/render/drop/CPU/GPU/memory
results and variance before setting thresholds. No silent software fallback or
unexplained stalls are acceptable. Sanity checks cannot substitute for physical
GPU/compositor/host validation.

## Verification ledger

| Date | Check | Outcome |
| --- | --- | --- |
| 2026-10-25 | Five source repositories, relevant history, dependency pins and release metadata | Inspected; exact source manifest and references recorded |
| 2026-10-25 | Primary source/worktree | Baseline unchanged; pre-existing user changes preserved |
| 2026-10-25 | Windows Qt/compiler, WSL and Docker engine checks | No available Fedora build/runtime; Docker Linux pipe unavailable |
| 2026-10-25 | Required Markdown files | All nine created; local link/anchor and code-fence checks passed |
| 2026-10-25 | Documentation source references | 58 pinned Git link occurrences / 45 distinct commit-or-file objects verified against cached reference repositories; line anchors valid |
| 2026-10-25 | Documentation whitespace and change scope | Git whitespace checks and explicit new-file checks passed; application/build/dependency tracked files unchanged |
| 2026-10-25 | Fedora baseline helper local validation | Git Bash 5.3.9: syntax, --help/-h, unknown-option/positional-argument rejection, and non-Linux collect/build rejection without state creation passed; ShellCheck unavailable; Fedora collection/build paths not executed |
| 2026-10-25 | Fedora 44 GNOME Wayland preflight, source comparison and recursive pins | Application/build paths unchanged from frozen baseline; pins match and submodules clean; Intel Haswell/i915 enumerated; VA-API/Vulkan diagnostics recorded in Fedora result record; no application GPU verification |
| 2026-10-25 | Build prerequisites and helper availability | qmake6 and queried development packages missing; documented helper absent; no build or session attempted |
| 2026-10-25 | Fedora dependency resolution | Corrected `wayland-protocols` to `wayland-protocols-devel`; dry-run resolves 30 installs, no removals/upgrades; installation blocked by sudo password and pending offline transaction warning; see Fedora build guide |
| 2026-10-25 | Fedora dependency installation/debug/release | User completed dependency installation; RPM/pkg-config checks pass; qmake6 6.11.2 and `make -j2 debug` / `make -j2 release` all exit 0 in separate build directories; expected binaries exist, debug symbols present, no unresolved linked libraries; Qt/FFmpeg warnings recorded in Fedora result record |
| 2026-10-25 | Isolated release Wayland startup/restart | Two 15-second runs exit 0 after SIGTERM; SDL reports Wayland; local identity fields persist; 512 checked native Moonlight files unchanged; capability-probe warnings recorded; no live session or interactive UI acceptance |
| 2026-10-25 | Local host discovery | `timeout 8s avahi-browse --resolve --terminate _nvstream._tcp` exits 0, no advertisements; host versions/addresses/permissions not supplied |
| 2026-10-25 | User gate exception | User requested skipping pairing tests and starting Milestone 1; baseline remains partial, not passed |
| 2026-10-25 | Task 1A Linux debug build/native isolation | qmake/make and three startup runs exit 0; native settings/cache, new persistent identity and Wayland app ID verified; 544 Moonlight/baseline files unchanged; dependency pins unchanged; limits recorded in 1A evidence |
| 2026-10-25 | Task 1B debug/stage/metadata and isolated runtime | Builds/staging/desktop/AppStream/resource/ldd checks pass; zero Moonlight package path collisions; native/portable identities persist, legacy state untouched; concurrent startup passes; Wayland IDs and XWayland properties verified; 512 production files unchanged; visual shell grouping pending |
| 2026-10-25 | Task 1C Linux debug/import fixtures | Build and 32 focused Qt checks pass; headless CLI, shared lock, concurrency, interruption/no-overwrite, credential-byte preservation and source immutability verified with synthetic fixtures |
| 2026-10-25 | Task 1C Wayland startup/restart | Three imported profiles, six runs exit 0; default fresh identity and explicit tuple persistence verified; active-profile refusal and 512 production files unchanged; host authentication skipped |
| 2026-10-25 | Milestone 2 debug/fixtures/local polling | Debug build passes; 57 capability checks and 32 import regressions pass; two isolated Wayland HTTP fixture runs use only serverinfo, no QML errors; 512 production files unchanged; live-host/UI acceptance pending |
| 2026-10-25 | Task 3A debug/lifecycle/disabled startup | Debug build and 15 lifecycle cases pass, including 200 restarts and 30 pending-reply shutdowns; isolated Wayland startup retains serverinfo-only polling and 512 production files; live disabled-stream comparison remains pending |
| 2026-10-25 | User-provided isolated real-host connection | Pinned paired metadata/app list pass; missing launch produced 403, later launch grant observed; three native Wayland 1080p H.264 VA-API/EGL-GLES stream/disconnect runs exit 0 with stereo init; three worker GET/stop cycles complete; 512 production files unchanged; controlled baseline/input/audio/UI/standard-Sunshine acceptance pending |
| 2026-10-25 | Task 3B manual path/controller | Debug build, 16 protocol/lifecycle cases, 32 import regressions and isolated Wayland fixtures pass; Memcheck reports 0 errors/lost allocations; opt-in manual UI exists; real clipboard grants/platform latency/automatic wiring pending |
| 2026-10-25 | Task 3B focused GNOME automatic integration | Debug build and 117 local checks pass; worker-tick/focus/fallback/direction cases included; 22 Memcheck cases pass with 0 errors/lost bytes; final isolated GUI reopened; integrated automatic/KDE/Sunshine live acceptance pending |
| 2026-10-25 | Milestone 0 / Task 3B user acceptance | User confirms automatic behavior works as intended and requests Milestone 0 done; 3B closed for primary GNOME target, 0 closed by acceptance; pairing skipped and unrun secondary-platform/performance checks remain follow-ups |
| Pending | Streaming/input/audio/GPU/compositor/performance/package matrix | Real-session acceptance unverified; pairing tests skipped by user |

Update this ledger with actual command/outcome evidence; do not turn a proposed
test or inability to run into a pass. Keep host details and baseline results in
[FEDORA_BUILD.md](FEDORA_BUILD.md#baseline-result-record).

## Planning-to-implementation handoff

Current state: Tasks 1A/1B/1C and the Milestone 2 read-only capability foundation
and the Task 3A bridge are implemented and locally verified above. Visual shell
grouping, real-host capability/UI acceptance and real-session/performance checks
remain unverified; host pairing/authentication is skipped by user instruction.
Milestone 0 is closed by explicit user acceptance, with skipped/unrun coverage
retained. Task 3B implements opt-in clipboard, manual actions and focused GNOME
Wayland automatic integration, now accepted by the user for the primary target.
KDE and real Sunshine checks remain coverage follow-ups. Services stay off by default. All ADRs in [DECISIONS.md](DECISIONS.md) are accepted design;
implementation and acceptance status are recorded separately.

Next: Milestone 4 display launch management, after reading its capability and
display prerequisites. Retain manual clipboard fallback on unverified backends
and keep broader platform/performance coverage recorded as follow-ups. Deferred APIs
are listed in [APOLLO_PROTOCOL.md](APOLLO_PROTOCOL.md#unsupported-and-unverified-behavior).

The prompts below are historical baseline/1A reproduction instructions. Task 1A is
already implemented; use the task checklist above for current work. Their original
baseline gate is superseded by the recorded user exception.

Ready-to-use next-session prompt:

```text
Work only on Perigee Milestone 0 baseline verification.
Read AGENTS.md, docs/IMPLEMENTATION_PLAN.md, docs/FEDORA_BUILD.md and the source
manifest in docs/REPOSITORY_ANALYSIS.md. Preserve user changes and dependency pins.
Use Fedora 44 GNOME Wayland, disposable XDG/TMPDIR state, and the documented qmake6
debug/release commands. Record packages, actual GPU decoder/renderer, host versions,
Apollo permissions, video/audio/input, pairing persistence, recovery and repeated
release performance. Test ordinary Sunshine separately. Never use or unpair the
production Moonlight identity to make tests pass.
No application changes, installs over Moonlight, commits, pushes, or publication.
Update the result record and checklist only from executed evidence. If hardware or
host access is missing, report the exact unmet gate and stop without starting 1A.
```

First coding prompt, runnable only after Milestone 0 is recorded as passed:

```text
Implement only Perigee Task 1A: Runtime Identity Isolation.
Read AGENTS.md, docs/IMPLEMENTATION_PLAN.md, docs/BRANDING.md, relevant architecture
and ADRs. Confirm required docs exist and Milestone 0 acceptance is recorded.
Otherwise complete/report that prerequisite without application edits.

Inspect git status and preserve unrelated changes. Researched starting point:
Moonlight Qt ac296864ace96f51ad75f78b90644bdbca76c332; pinned Common C
04053843c90c4268583f2496b997d54d59f03957. Do not change dependency pins.

In app/main.cpp and app/app.pro implement Linux output perigee, Qt organization
io.github.BarMy49, application Perigee, domain barmy49.github.io, Linux application
ID io.github.BarMy49.Perigee for Qt/SDL, Perigee SDL labels and independent log prefix.
Set identity before settings/paths/credentials initialize.

No import, packaging, portable migration, clipboard, display or UI modernization.
Preserve GameStream, pairing, discovery, internal APIs and streaming-source behavior.
Build with qmake6/make debug as documented. Verify disposable clean native startup,
independent paths, credential persistence after restart and unchanged Moonlight
state. Record exact commands/results and unavailable checks; update status/docs.
Do not install, commit, push or publish. Stop after this task.
```
