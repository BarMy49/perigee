# Implementation plan and handoff

Approved plan date: 2026-10-25. This document is the task/status entry point; read
[AGENTS.md](../AGENTS.md), the relevant [architecture](ARCHITECTURE.md),
[protocol](APOLLO_PROTOCOL.md), [branding](BRANDING.md), and
[decision](DECISIONS.md) sections before application work.

## Current state

- Source remains Moonlight Qt `ac296864ace96f51ad75f78b90644bdbca76c332` with its
  upstream dependency pins. Linux output/metadata/settings still identify Moonlight.
- Research and the approved plan are now materialized in the requested Markdown
  documents. No application feature, build configuration, dependency or asset changed.
- User selected Fedora 44 GNOME Wayland first, KDE Wayland second,
  `io.github.BarMy49.Perigee`, optional explicit pairing-identity import, and per-host
  opt-in bidirectional session text clipboard synchronization.
- Actual Fedora build/runtime baseline remains pending. Windows has no queried Qt
  build command; Docker Linux daemon is unavailable; WSL lists only docker-desktop.
- A baseline collection/build helper is now available at
  `scripts/collect-fedora-baseline.sh`; this supporting tool does not implement or
  validate application features. Its local check results are recorded in the ledger.
- **Next task is Milestone 0 baseline collection, not application implementation.**
  Milestone 1A starts only after all baseline acceptance is recorded as passed.
- Existing user `TODO.md` and untracked IDE state must be preserved. Do not commit,
  push, publish, provision services, or perform broad source renaming.

## Milestone summary

| Milestone | State | Dependency / gate |
| --- | --- | --- |
| Documentation materialization | Complete | Required documents and evidence checks passed |
| 0: Baseline | Pending environment; not complete | Real Fedora/GPU and Apollo/Sunshine host evidence |
| 1A: Runtime identity isolation | Not started | Milestone 0 passed |
| 1B: Desktop/portable/package identity | Not started | 1A acceptance |
| 1C: Explicit Moonlight import | Not started | Isolated native identity and import fixtures |
| 2: Apollo foundation | Not started | Identity isolation and capability design |
| 3A: Session service bridge | Not started | Capability values and lifecycle tests |
| 3B: Text clipboard | Not started | Bridge, paired HTTPS, platform adapter gate |
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
Perigee application changes. **State:** pending, not verified.

**Scope/dependencies:** no application/dependency changes; supporting baseline
automation is allowed. Fedora 44 GNOME Wayland, real client GPU,
Windows Apollo and a standard Sunshine test host. Collect versions/permissions and
use an isolated configuration instead of the production Moonlight identity.

**Relevant sources:** `moonlight-qt.pro`, `app/app.pro`, `globaldefs.pri`,
`.gitmodules`, and existing `Session`/decoder/input paths for diagnosis only.

- [x] Implement an opt-in baseline collector/build helper without installing or
  launching software; local syntax/CLI checks are recorded separately from Fedora acceptance.
- [ ] Verify source/pins and resolve/document Fedora packages.
- [ ] Build debug and release using [FEDORA_BUILD.md](FEDORA_BUILD.md).
- [ ] Record actual decoder/renderer and basic video/audio/keyboard/mouse operation.
- [ ] Test pairing persistence, launch/resume, disconnect/reconnect and network loss.
- [ ] Verify Apollo permissions and ordinary Sunshine operation independently.
- [ ] Record comparable release performance/resource observations and variance.
- [ ] Complete [baseline result record](FEDORA_BUILD.md#baseline-result-record).

**Acceptance:** reproducible successful build and real GNOME Wayland sessions on
both host types, selected hardware acceleration verified, working audio and input,
recovery evidence, isolated state and measured baseline. Broader hardware/KDE/X11
coverage may continue in the matrix but cannot be claimed from GNOME alone.

**Blockers:** no suitable Fedora build/runtime in the current Windows session; host,
GPU/driver and network inventory/results not supplied. A container compile is useful
but does not satisfy real-session acceptance. Diagnose baseline failures before
implementing around them. Do not mark passed from source/package inspection.

## Milestone 1: Perigee identity and isolation

### Task 1A: runtime identity isolation

**Goal:** smallest useful first coding task: uninstalled Linux `perigee` with
independent runtime settings/cache identity. **Depends on:** Milestone 0 passed.

**Files:** `app/main.cpp`, `app/app.pro`. Keep this first task limited to these
startup/build surfaces; any genuinely necessary expansion must be explained.

- [ ] Set Linux target `perigee` without changing qmake or dependency names.
- [ ] Set Qt organization `io.github.BarMy49`, application `Perigee`, organization
  domain `barmy49.github.io` before paths/preferences/identity initialization.
- [ ] Set Qt desktop identification and SDL Wayland/X11 IDs to
  `io.github.BarMy49.Perigee`; use Perigee SDL application/audio labels.
- [ ] Use independent log creation and cleanup prefixes.
- [ ] Verify clean native startup, settings/cache path, credentials across restart,
  unchanged Moonlight state, and matching runtime desktop identity.

**Expected behavior:** binary `app/perigee`, native settings under
`$XDG_CONFIG_HOME/io.github.BarMy49/Perigee.conf`, fresh independent client identity.
No import or installation. Stream-title/translated text and portable migration are
deliberately later tasks; do not test this increment in a portable.dat directory.

**Tests/acceptance:** debug build; disposable clean XDG profile; verify file paths
and safe credential fingerprints before/after restart; compare Moonlight settings
and pairing files byte-for-byte without exposing them. No GameStream, library or
streaming-source changes. Record every unavailable check as pending.

**Blocker:** baseline environment/results. Do not claim complete from a compile alone.

### Task 1B: complete desktop and portable identity

**Goal:** coherent installable identity after runtime isolation.

**Relevant sources:** Linux deploy files and `app.pro` install rules; startup/path,
CLI/QML visible branding, stream title, resources and update checker.

- [ ] Align desktop entry, AppStream ID/launchable/binary, icon basename, WMCLASS,
  installed paths and user-facing Perigee labels.
- [ ] Retain attribution and verify asset terms before any attributed temporary icon
  reuse; original artwork remains an explicit later task.
- [ ] Isolate portable marker and every portable config/cache/log directory.
- [ ] Disable upstream artifact-update announcements for Perigee until a dedicated
  feed exists; assess mapping/compatibility endpoints separately.
- [ ] Stage install files and validate metadata; test Moonlight/Perigee coexistence.

**Acceptance:** no Moonlight-owned installed path/state modified; native/portable
restart and grouping pass; desktop/AppStream/icon references resolve; no wrong
artifact-update path. Session-title edit is cosmetic only. No mass replacements.
**Blockers:** runtime validation and redistributable icon provenance/artwork.

### Task 1C: explicit import

**Goal:** retain selected settings/hosts and optionally pairing without overwriting
either application's existing identity. **Files:** new importer plus early startup,
CLI parsing and existing serialization/identity seams; choose exact new paths in
the task after inspecting the startup ordering.

- [ ] Implement `perigee import-moonlight --source PATH [--include-pairing]` before
  normal settings/identity consumers, with the [approved contract](BRANDING.md#explicit-import-contract).
- [ ] Use an allowlist, destination lock, validated credential tuple/host pins,
  protected atomic publication, and source immutability.
- [ ] Add disposable fixtures for native/portable input, corrupt data, existing or
  in-use destination, interruption, and credential-free ordinary export.
- [ ] Verify real host authentication and explain shared host identity when copied.

**Acceptance:** fresh pairing by default; optional valid bundle remains unchanged
across restart and works with the matching host; failed imports leave no partial
profile and never modify source/destination identity. **Blockers:** credential
fixture design and host testing. Import success is not inferred from readable PEM.

## Milestone 2: Apollo foundation

**Goal:** conservative capability values with unchanged standard Sunshine behavior.
**Relevant sources:** `NvHTTP`, `NvComputer` update path, existing GUI models; new
small parser/value components. **Dependencies:** identity baseline and ADR-002.

- [ ] Parse authenticated optional permissions, ordered commands and Windows display
  flags. Distinguish absent/invalid/unauthenticated/denied values.
- [ ] Expose read-only state and a user-configured HTTPS admin link. Do not mutate
  permissions or require an Apollo product-version field.
- [ ] Establish a focused separate qmake/Qt Test target for new pure parsing/state
  behavior; keep product build independent of test/live-host dependencies.
- [ ] Verify standard-host fixtures and real Sunshine generate no extension probes.

**Acceptance:** deterministic parser tests, correct read-only Apollo state and
unchanged normal host operations. **Blockers:** no versioned capability spec; keep
the UI honest about Apollo-compatible capabilities rather than inventing identity.

## Milestone 3: clipboard integration

### Task 3A: session service bridge

**Goal:** optional asynchronous services without moving the streaming/render loop.
**Relevant sources:** `Session` start/teardown and SDL user-event dispatch; new
`SessionServices`, worker transport and bounded bridge. **Dependencies:** capability
values and [ownership design](ARCHITECTURE.md#data-flow-and-contracts).

- [ ] Configure worker-owned Qt network/timers from immutable session snapshots.
- [ ] Deliver owned bounded messages to SDL with registered wakeups and generations.
- [ ] Invalidate/cancel/drain before cleanup; do not access raw mutable host/session
  pointers from workers or wait for blocked GUI-thread slots.
- [ ] Stress start failure, rapid disconnect/reconnect and shutdown with replies
  pending; verify no stale action, leaked worker/reply or use-after-free.

**Acceptance:** lifecycle tests pass; feature-disabled streaming retains baseline
behavior and no new event-loop or renderer architecture. **Blocker:** teardown and
clipboard ownership must be proven, not copied from another fork.

### Task 3B: secure text synchronization

**Goal:** manual send/receive plus per-host opt-in bidirectional session text sync.
**Sources:** new clipboard controller/adapter/HTTPS operations; existing SDL
clipboard/input path and preferences/QML settings. **Depends on:** 3A and Windows
Apollo paired permissions/active session.

- [ ] Implement raw UTF-8 GET/POST, pinned trust, redirect refusal, timeout and bounded
  receiving; distinguish empty success from failure.
- [ ] Add content revisions/echo suppression, one in-flight operation, latest-write
  coalescing, cancellation and no initial automatic clipboard upload.
- [ ] Measure SDL reads and Wayland access; add manual fallback where automation is
  restricted or causes stalls. No PRIMARY, payload logs or persisted content.
- [ ] Add protocol/state tests and real GNOME/KDE permission/focus/reconnect checks.

**Acceptance:** [protocol contract](APOLLO_PROTOCOL.md#text-clipboard-contract) passes,
no unauthorized traffic/loops/stale overwrites, Sunshine unaffected, no unexplained
input/video stalls. **Blockers:** compositor restrictions and SDL retrieval latency;
images and Linux-host placeholders are outside this milestone.

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
Test/qmake target when that milestone begins. No such application suite exists yet;
do not list a hypothetical command as already runnable. Existing qmake configure
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
| Pending | Fedora dependency resolution/debug/release | Not executed |
| Pending | Pairing/streaming/input/audio/GPU/compositor/performance/package matrix | Not executed |

Update this ledger with actual command/outcome evidence; do not turn a proposed
test or inability to run into a pass. Keep host details and baseline results in
[FEDORA_BUILD.md](FEDORA_BUILD.md#baseline-result-record).

## Planning-to-implementation handoff

Current state: upstream application plus approved documentation; no features or
identity changes. Research covers exact references, architecture, Apollo protocols,
fork port choices, risks, build instructions and migration. All ADRs in
[DECISIONS.md](DECISIONS.md) are accepted design; implementation status is separate.

Next actionable task is Milestone 0 on a real Fedora machine. Unknowns to collect:
client GPU/driver/monitor, host GPU/encoder, actual Apollo and standard Sunshine
versions, network and compositor behavior. Do not ask another agent to infer these
from the Windows planning environment. Deferred APIs are listed in
[APOLLO_PROTOCOL.md](APOLLO_PROTOCOL.md#unsupported-and-unverified-behavior).

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
