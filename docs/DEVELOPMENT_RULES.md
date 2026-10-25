# Development rules

Status: approved workflow. See [AGENTS.md](../AGENTS.md) first, then the assigned
[implementation task](IMPLEMENTATION_PLAN.md) and its linked decision records.

## Session and Git workflow

1. Inspect the worktree and submodule state. Preserve unrelated changes, including
   the pre-existing `TODO.md` and IDE files; do not stage them incidentally.
2. Identify one checklist task, its prerequisites, likely files, and acceptance.
   Read relevant source rather than rediscovering the entire repository.
3. Before editing, summarize intended behavior, purpose, files, constraints, and
   verification. Proceed with approved routine work without repeated permission.
4. Make small changes within the accepted design. If evidence invalidates it,
   document the issue and propose an ADR change before implementing an alternative.
5. Run relevant checks, update evidence/status/docs, and stop after the task.

Do not create branches, commit, push, publish, install services, or change global
developer settings without authorization. A feature branch is the preferred
eventual review unit when branch creation is authorized. Do not combine upstream
merges with new features. Never reset or clean the user's worktree to simplify a
task. Keep references outside the primary checkout and record their exact SHAs.

## C++ and Qt/QML conventions

- Follow the surrounding upstream code: C++17, qmake, existing brace/indentation,
  descriptive names, local `m_` member style, and current QObject ownership patterns.
- Do not rename internal classes, QML modules, libraries, or files for branding.
- Do not introduce one-letter names or inline code comments unless requested.
  Put design rationale in these documents; preserve existing upstream comments.
- Prefer RAII and explicit ownership. Respect API-specific SDL/OpenSSL allocation
  and release rules. Avoid singleton lifetime coupling for new session services.
- Use Qt signals/slots for asynchronous state changes where the receiver's event
  loop actually runs. Register necessary value types for queued connections.
- Keep QML declarative and presentation-focused. Do not issue HTTP, manipulate
  credentials, or interpret protocol fields in QML.
- Use existing Qt Quick Controls/layouts, translation conventions, and navigation.
  Avoid fixed handheld assumptions, custom window decorations, and large UI deps.
- Use the repository's existing formatting; do not add a formatter or perform
  repository-wide style changes as part of a feature.

## Asynchrony, errors, and thread safety

- SDL owns the normal GUI thread while streaming. Read the
  [lifecycle analysis](ARCHITECTURE.md#session-lifecycle-and-threading) before adding
  timers, clipboard operations, UI interactions, or networking.
- New network operations use a Qt worker event loop and worker-owned network
  objects. No synchronous GUI network waits or new nested event loops.
- Workers consume immutable endpoint/capability/credential snapshots. Do not read
  mutable `NvComputer` or the global active session across threads without their
  documented ownership/synchronization. Prefer eliminating that access entirely.
- Use bounded queues and session generations. Cancel requests before destroying
  dependencies; ignore late callbacks and drain/discard owned pending messages.
- Window, SDL clipboard, and Qt GUI objects remain on their owner thread.
- Distinguish unsupported, not authorized, temporarily unreachable, malformed,
  cancelled, and successful-empty results. Do not turn an empty string into a
  generic success/failure flag or retry permanent failures in a busy loop.
- Host commands can report sent/failed; no verified execution acknowledgement
  exists. Never tell the user a remote command completed based only on send success.
- User-facing errors describe an action and available recovery; diagnostics may
  record safe context. Do not swallow unexpected failures or fabricate support.

## Security and compatibility

- Preserve standard Sunshine behavior without Apollo metadata. Unauthenticated
  metadata must not establish permissions or grant access to sensitive operations.
- Require paired TLS trust and a pinned host certificate for clipboard. Reject
  mismatched peers and redirects; never use `VerifyNone` or HTTP fallback.
- Clipboard sync is per-host opt-in and active-session scoped. Bound network
  payloads during receipt; do not log/persist clipboard content or use PRIMARY.
- Never log/export private keys, certificates as identity bundles, PINs, passphrases,
  input encryption keys, or session tokens. Treat pre-existing upstream diagnostic
  behavior as something to audit, not permission to duplicate sensitive logging.
- Keep pairing import separate from preferences export. Never silently merge or
  overwrite an existing Perigee identity. Preserve Moonlight source configuration.
- Do not invent host commands, imply client-side permission escalation, provision
  SFTP, or assume administrative web credentials equal pairing credentials.
- Keep protocol fields, discovery identifiers, TLS algorithms, imported credential
  bytes, and NVIDIA unique-ID compatibility behavior unchanged during branding.

## Dependencies, performance, and licensing

Preserve qmake and pinned upstream dependencies. Review new libraries against an
existing Qt/SDL/system alternative and document build, runtime, sandbox, and license
costs. No fork dependency replacement to obtain a single feature.

Changes touching `app/streaming/` or Common C require an explicit explanation of
thread/lifetime effects and the affected input, decoder, presentation, audio, or
transport paths. Keep them separate from cosmetic work. Do not add framebuffer
readback, software fallback, continuous offscreen QML rendering, or per-frame
allocations for UI conveniences. Check repeated baseline comparisons, including
feature-disabled operation. Use supported hardware as a test requirement rather
than claiming hardware coverage from a successful compile.

Preserve GPL, third-party notices, upstream attribution and source provenance.
Before a port, inspect the exact file/commit and license; record source, selected
behavior, adaptations, and rejected dependencies. Distribution must include the
applicable license information and access to corresponding source/build material.
Do not assume all assets or dependencies inherit the root license.

## Testing, documentation, and completion

- No first-party application suite exists in the inspected baseline; capability
  probes in `config.tests/` are not such a suite. The roadmap separately plans a
  focused Qt Test target for new protocol/state logic, not tests mirroring UI text.
- Use disposable configuration/identity fixtures and synthetic clipboard values.
  Integration tests must not unpair or overwrite the user's production client.
- Begin with checks relevant to the change; broaden only for new failures or
  unresolved concerns. Do not fix unrelated failures under the current task.
- Record exact command, source/dependency versions, environment, exit/result,
  observed behavior, and limitations. Separate automated, manual, and proposed tests.
- Update protocol docs for wire changes, branding docs for identity/storage changes,
  architecture/ADRs for boundaries, and the plan for task status. Cross-link rather
  than duplicating source catalogs or approval history.
- An unavailable check is pending, not passed. Complete a milestone only when all
  acceptance criteria have evidence. Do not invent latency thresholds or results.
- Report files/behavior changed, build/test outcomes, unresolved risks, docs updated,
  and the next task. Distinguish completed documentation from an unverified feature.
