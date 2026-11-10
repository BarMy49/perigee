# Upstream update procedure

This is the Milestone 7 maintenance workflow. No upstream merge has been performed
or accepted by this document. Choose a specific reviewed upstream revision before
an update; do not silently track a moving branch or replace Common C with a fork.

## Baseline and isolation

The source baseline is Moonlight Qt
`ac296864ace96f51ad75f78b90644bdbca76c332`. Preserve the recorded dependency pins:

| Component | Revision |
| --- | --- |
| Moonlight Common C | `04053843c90c4268583f2496b997d54d59f03957` |
| qmdnsengine | `920c097ffa742e2968290f15d4dde6693aec02e5` |
| SDL_GameControllerDB | `8d9fefd7b810f2541f78cc7a8ccbd185bc84c7a5` |

Start by recording git status, the actual Perigee HEAD and recursive submodule
status. Preserve unrelated changes and untracked implementation files; a bare HEAD
archive may omit required sources in a development worktree. Use the explicit
[source snapshot workflow](../packaging/rpm/README.md) when validating that tree.
Reference repositories, SDKs and compilation caches belong outside the IDE project.

Branch creation, commits, pushes, publication and installing services need the
user's authorization. Once an update target and working-copy arrangement are
authorized, use an isolated checkout for integration. Never merge into or clean
the user's dirty working tree merely to make an update easier. Keep dependency
updates and Perigee features separately reviewable. Commands remain deferred under
[ADR-005](DECISIONS.md#adr-005-preserve-common-c-initially-narrow-exception-for-commands).
The [Common C proposal](../patches/common-c/README.md) is not an applied dependency.

## Conflict review

| Source boundary | Perigee behavior that must survive |
| --- | --- |
| app/main.cpp, app/path.cpp, deployment metadata | Identity before settings/credentials, native/portable isolation, independent launchers, no upstream release feed |
| app/settings and importer/profile store | Allowlisted explicit import, anonymous no-overwrite credential publication, theme/shortcuts/profiles, detached session values |
| app/backend/nvhttp, nvcomputer and capability parsing | Paired certificate trust, authenticated optional permissions, numeric Sunshine path, fresh launch gates and stable local host fields |
| app/streaming/session and services | SDL/Qt thread ownership, cancellation/generations, consent/focus rules and cleanup order |
| app/streaming/input and overlay | AltGr handling, reserved recovery keys, remote release ordering, clipboard/panel dispatch |
| app/gui and translation resources | Grid selection/favorites, responsive settings, opaque theme dialogs, localized errors and exact placeholders |
| qmake and packaging/CI scripts | Existing probes and compiler flags, feature availability, Perigee-only manifests, all notices and corresponding source |

Resolve conflicts against the intended behavior, not by selecting a whole upstream
or downstream file. Separately analyze any change affecting decoder selection,
hardware interop/zero-copy, audio, recovery, input ordering or pacing. Document an
unexpected architecture problem and proposed decision before substituting a new
architecture. Preserve protocol strings, certificate algorithms, imported bytes,
server pins and GFE unique-ID behavior.

## Verification and release gate

1. Build native debug/release in separate directories. Run the
   [focused Linux checks](../tests/README.md), translations, stage/metadata checks
   and relevant packaging checks. Record actual commands and results.
2. Compare enabled and disabled optional features on the same workload/settings.
   Real acceptance includes identity/restart, Apollo and ordinary Sunshine launch/
   resume, clipboard/input recovery, actual GPU decoder/render path, audible audio,
   reconnect/resource use and controlled performance. Preserve the user's pairing
   skip and other permission limits; arrange separately authorized live checks.
3. Test RPM install/upgrade/remove and coexistence, AppImage relocation/backends,
   and Flatpak sandbox/portals/permissions. A previous package's acceptance does
   not automatically cover an updated binary or dependency set.
4. Review changed dependency/source/asset licenses, notices, build flags, full
   corresponding-source availability and update metadata before release. Do not
   publish an unsigned development artifact as an accepted release.
5. Update the implementation plan with the chosen upstream revision, conflict
   decisions, exact new dependency revisions (if any), commands, environment and
   unavailable checks. Mark a gate complete only from executed or explicit user
   acceptance evidence.

A clean compile, container check or manifest review does not establish compositor,
GPU, host authentication or streaming success. Retain pending secondary-platform,
host and performance coverage explicitly.
