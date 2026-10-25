# Architecture decisions

Approved on 2026-10-25 when the user requested implementation of the presented
plan. Acceptance authorizes the recorded approach; it does not mark application
features or environment checks complete. [Implementation status](IMPLEMENTATION_PLAN.md#current-state)
is maintained separately. Revisit a decision when its stated evidence changes,
document the new evidence, and obtain review before an architectural deviation.

## ADR-001: retain upstream Moonlight Qt and qmake

- **Date/status:** 2026-10-25; accepted, application baseline unchanged.
- **Context:** Current Moonlight provides maintained hardware/video/input behavior;
  Artemis and Vibemis diverge substantially in session and rendering code.
- **Alternatives:** Base on either fork; rewrite the client/build; selective ports
  onto current Moonlight.
- **Decision:** Use Moonlight Qt `ac296864ace96f51ad75f78b90644bdbca76c332`, its
  qmake/C++17 structure, and its existing dependency pins as the initial baseline.
- **Rationale:** Preserve tested upstream responsibilities and a practical merge path.
- **Consequences:** Obtain a real Fedora baseline before application changes; adapt
  selected behaviors rather than import whole managers or renderer shims.
- **References:** [Repository ancestry](REPOSITORY_ANALYSIS.md#ancestry-and-fork-differences),
  [existing architecture](ARCHITECTURE.md#existing-moonlight-qt-architecture).

## ADR-002: capability-driven Apollo composition

- **Date/status:** 2026-10-25; accepted; Milestone 2 read-only foundation implemented and locally
  verified. Live-host acceptance and session services remain pending.
- **Context:** Apollo extensions are optional, permissions depend on pairing, and
  the inspected server does not emit the `ApolloVersion` parsed by Artemis.
- **Alternatives:** Assume every non-NVIDIA host is Apollo; modify all existing
  classes directly; generic backend plugin system; small composed services.
- **Decision:** Parse optional authenticated capabilities into values, keep ordinary
  Sunshine behavior, and compose Apollo HTTPS/session services around existing code.
- **Rationale:** Avoid false product detection and centralize extension policy with
  minimal changes to upstream-heavy classes.
- **Consequences:** Missing, invalid and denied permissions are distinct. UI may say
  Apollo-compatible extensions without asserting an unverified product/version.
- **References:** [Host discovery](APOLLO_PROTOCOL.md#capability-discovery),
  [target components](ARCHITECTURE.md#approved-apollo-integration-design).

## ADR-003: respect the SDL event loop and thread ownership

- **Date/status:** 2026-10-25; accepted; Task 3A bridge implemented with local lifecycle evidence.
  Services remain disabled; live media and clipboard platform acceptance pending.
- **Context:** Current `Session::exec()` runs an SDL loop on the GUI thread; normal
  Qt GUI events are not pumped through that streaming loop.
- **Alternatives:** Port a fork's threaded session; pump Qt events continuously;
  integrate workers and an SDL-owned clipboard/window adapter.
- **Decision:** Preserve session threading. Use worker-owned asynchronous Qt HTTPS
  and timers, immutable snapshots, and bounded generation-tagged SDL messages.
- **Rationale:** Avoid renderer/context and lifetime changes to obtain clipboard/UI.
- **Consequences:** QML timers/queued GUI slots cannot be assumed to run in-stream;
  clipboard retrieval latency and Wayland focus require a measured platform gate.
- **References:** [Lifecycle](ARCHITECTURE.md#session-lifecycle-and-threading),
  [clipboard contract](APOLLO_PROTOCOL.md#text-clipboard-contract).

## ADR-004: independent application identity and explicit pairing import

- **Date/status:** 2026-10-25; accepted; Linux identity and explicit import are
  implemented in Tasks 1A/1B/1C with local verification. Real-host authentication
  remains skipped by user instruction.
- **Context:** Qt names determine persistent paths; pairing spans a client key,
  certificate, unique ID and host pins. Silent copying can share host permissions.
- **Alternatives:** Share Moonlight settings; always pair afresh; independent state
  with deliberate credential import.
- **Decision:** Use Perigee / `perigee` / `io.github.BarMy49.Perigee`; independent
  QSettings/cache identity; fresh credentials by default; optional explicit import
  before normal identity initialization into an unused destination.
- **Rationale:** Coexistence and user-controlled retention of existing pairings.
- **Consequences:** Copied credentials share host-side identity and unpairing effects;
  ordinary settings export excludes identity. Protocol/internal symbols stay intact.
- **References:** [Branding and import](BRANDING.md).

## ADR-005: preserve Common C initially; narrow exception for commands

- **Date/status:** 2026-10-25; accepted; upstream submodule unchanged.
- **Context:** Clipboard/display launch operations use HTTPS. Host commands use
  packet `0x3000`; upstream has no `LiSendExecServerCmd` API.
- **Alternatives:** Replace Common C with a fork; guess an HTTP endpoint; defer
  commands until an additive extension has review and reproducible source.
- **Decision:** Keep the pinned core through initial milestones. A later commands
  task may add only the necessary reviewed control function and dependency revision.
- **Rationale:** Minimize streaming-core divergence while acknowledging a verified need.
- **Consequences:** Commands remain disabled until that prerequisite exists; no
  fallback commands, local-list indices, or execution-success claims from send status.
- **References:** [Commands](APOLLO_PROTOCOL.md#host-commands),
  [Common C relationship](REPOSITORY_ANALYSIS.md#dependency-relationships).

## ADR-006: incremental desktop UI and small in-stream controls

- **Date/status:** 2026-10-25; accepted, not implemented.
- **Context:** The QML shell is reusable. Fork quick-menu implementations alter
  threading; Vibemis's inspected offscreen path reads back an OpenGL framebuffer.
- **Alternatives:** Import handheld UI; render a second QML scene continuously;
  modernize existing views and extend existing overlay/session actions.
- **Decision:** Keep Qt Quick Controls/native decorations and incrementally improve
  navigation/themes/accessibility. First session controls use a keyboard-accessible
  panel through existing overlay facilities.
- **Rationale:** Deliver desktop convenience without a new rendering architecture.
- **Consequences:** No video-frame readback or continuous QML overlay. Rich pointer
  controls, unsupported display operations, and tray behavior need separate tests.
- **References:** [UI boundaries](ARCHITECTURE.md#ui-profiles-and-controls),
  [selected fork changes](REPOSITORY_ANALYSIS.md#reuse-recommendations).

## ADR-007: verified text clipboard and optional existing SFTP

- **Date/status:** 2026-10-25; accepted, not implemented; user selected per-host
  opt-in session synchronization.
- **Context:** Apollo Windows implements a paired HTTPS text clipboard; image and
  native file-transfer paths are incomplete in the inspected host.
- **Alternatives:** Invent/extend a host protocol now; provision SSH automatically;
  text-only sync plus an optional existing secure transfer destination.
- **Decision:** Implement bounded active-session text sync with manual fallbacks;
  defer images/native transfer. Later open a user-configured `sftp://` destination
  via desktop integration only when a user-managed SSH service already exists.
- **Rationale:** Reuse implemented secure mechanisms without adding a host service.
- **Consequences:** Full background Wayland clipboard access is not promised; the
  portal API requires a compatible authorized session and backend validation.
- **References:** [Clipboard](APOLLO_PROTOCOL.md#text-clipboard-contract),
  [unimplemented host paths](APOLLO_PROTOCOL.md#unsupported-and-unverified-behavior).

## ADR-008: Fedora acceptance and evidence before features

- **Date/status:** 2026-10-25; accepted; baseline closed by explicit user acceptance.
- **Context:** The current workspace is Windows. No Qt development command was
  found; Docker's Linux daemon was unavailable; WSL listed only docker-desktop.
- **Alternatives:** Claim source inspection as validation; start features without a
  reproducible baseline; require the real environment before Milestone 1A.
- **Decision:** Fedora 44 GNOME Wayland is first, KDE Wayland second, with X11 where
  available. Build and measure upstream first; keep actual versus proposed tests clear.
- **Rationale:** Preserve performance and identify environment failures independently
  of Perigee changes.
- **Consequences:** Documentation can complete now, but application work waits for
  recorded baseline acceptance. Numerical performance gates follow measured variance.
- **References:** [Environment/evidence](FEDORA_BUILD.md#verification-status),
  [Milestone 0](IMPLEMENTATION_PLAN.md#milestone-0-baseline).

### ADR-008 user-directed implementation gate exception, 2026-10-25

After native Fedora debug/release builds and isolated Wayland startup checks,
the user instructed: “skip pairing tests, and start working towards milestone 1”.
This authorizes advancing to Task 1A without full Milestone 0 acceptance. Pairing
tests are skipped; streaming/input/audio/recovery/performance remain unverified.
No test is marked passed by this exception. The identity architecture and all
security/protocol boundaries remain as approved. Task 1A native evidence is in the
[implementation plan](IMPLEMENTATION_PLAN.md#task-1a-runtime-identity-isolation).

### ADR-008 user acceptance and closure, 2026-10-25

After the Fedora builds, real-host streaming and user-verified manual/automatic
clipboard checks, the user explicitly requested marking Milestone 0 done.
Milestone 0 is closed by that acceptance; pairing remains intentionally skipped.
Unchecked measurements and secondary-platform/host coverage stay follow-ups,
not fabricated passes. Task 3B is accepted for the primary GNOME Wayland target,
with manual-only behavior retained on unverified backends. This supersedes the
historical pending gate without changing thread ownership or security boundaries.
