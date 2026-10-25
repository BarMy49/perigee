# Perigee branding and identity migration

Status: approved migration design; no application identifiers, installed files,
settings, credentials, or artwork have been changed. See
[ADR-004](DECISIONS.md#adr-004-independent-application-identity-and-explicit-pairing-import)
and [Milestone 1](IMPLEMENTATION_PLAN.md#milestone-1-perigee-identity-and-isolation).

## Official identity

| Item | Approved value |
| --- | --- |
| Name | Perigee |
| Tagline | Your remote desktop, closer than ever. |
| Description | Perigee is an open-source, low-latency remote desktop and game streaming client for Linux, built on Moonlight Qt with enhanced Apollo integration. Designed for productivity, engineered for performance. |
| Repository | `perigee`; observed remote `https://github.com/BarMy49/perigee.git` |
| Linux executable / RPM package | `perigee` |
| Linux application ID / proposed Flatpak ID | `io.github.BarMy49.Perigee` |
| Desktop entry | `io.github.BarMy49.Perigee.desktop` |
| Planned AppStream filename | `io.github.BarMy49.Perigee.metainfo.xml` |
| Planned installed icon basename | `io.github.BarMy49.Perigee` |
| Qt organization name / application name | `io.github.BarMy49` / `Perigee` |
| Qt organization domain | `barmy49.github.io`, identity metadata only; no live website assumed |

The user explicitly selected the reverse-DNS ID after inspection of the repository
remote. No different GitHub organization or domain ownership is assumed. Initial
platform acceptance is Linux; Windows/macOS packaging remains unverified.

## Existing identity and sensitive state

| Existing location | Current behavior | Migration responsibility |
| --- | --- | --- |
| `app/app.pro` | Linux `TARGET=moonlight`; other targets use Moonlight | Change Linux executable deliberately; preserve qmake |
| `app/main.cpp:431` | Qt organization `Moonlight Game Streaming Project`, domain `moonlight-stream.com`, application `Moonlight` | Set Perigee metadata before settings, `Path`, or credentials initialize |
| `app/main.cpp:435` | `portable.dat` selects CWD-based INI paths | Give Perigee its own marker and all portable storage |
| `app/main.cpp:467`, `:509` | Moonlight log filename and cleanup pattern | Rename creation and cleanup together; avoid another application's logs |
| `app/main.cpp:727` | SDL application/audio labels | User-visible Perigee labels |
| `app/main.cpp:938` | Qt desktop-file ID and SDL Wayland/X11 WMCLASS | Same Perigee application ID throughout |
| `app/backend/identitymanager.cpp` | `certificate`, `key`, `uniqueid` in default QSettings | Fresh independent identity; explicit validated bundle import only |
| `app/backend/nvcomputer.cpp` | Host UUID/addresses and `srvcert`; app records | Preserve matching host pins and endpoint associations |
| `app/backend/computermanager.cpp` | `hosts` / `hostsbackup` arrays | Import a coherent recoverable host set; do not mix partial arrays |
| `app/path.cpp` | Standard cache/data paths and temp logs; generic portable cache directories | Let early identity drive native paths; isolate every portable directory |
| `app/streaming/session.cpp:1859` | Desktop stream title includes Moonlight | Change visible title in a later cosmetic task without session logic changes |
| `app/deploy/linux/` and install rules | Moonlight desktop/AppStream/icon/binary names | Migrate the complete installed-file manifest together |
| `app/backend/autoupdatechecker.cpp` | Upstream update feed on supported artifact types | Disable Perigee artifact checks until a dedicated release feed exists |
| `app/cli/commandlineparser.cpp`, `app/gui/`, resources/translations | Visible labels, help and application assets | Focused user-facing migration; preserve internal translation contexts |
| `scripts/`, `.github/workflows/`, platform metadata | Artifact naming and platform installer details | Audit at distribution milestone; do not claim cross-platform completion |

Source references: [startup](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/main.cpp#L428),
[identity](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/backend/identitymanager.cpp#L109),
[Linux install rules](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/app.pro#L523).

## Identifier classification

| Class | Examples | Approved treatment |
| --- | --- | --- |
| 1: safe cosmetic/application rename | Display name, visible help/title strings, Linux target/package, log prefixes, Qt/SDL desktop labels, desktop/AppStream/icon IDs, release names | Rename related references together; validate file ownership and launch behavior |
| 2: rename with compatibility migration | Qt settings identity, native/portable paths, persistent schema, host/credential imports | Separate destination, explicit migration, protected atomic writes and coexistence tests |
| 3: preserve for compatibility | HTTP paths/query names, XML fields, packet types, `_nvstream._tcp.local.`, authentication algorithms, imported client key/cert/UID, host pins and app/session IDs, NVIDIA placeholder UID behavior | Do not rename or regenerate as branding work |
| 4: investigate; retain initially | Certificate CN `NVIDIA GameStream Client`, pairing `devicename=roth`, URI registration, external compatibility/mapping/connectivity/STUN endpoints | No evidence that all are mandatory literals, but changes need separate compatibility evidence |

Internal `Nv*`, `Session`, QML import names, `Li*`, `moonlight-common-c`, and data
filenames such as the controller database remain unchanged. They are not marketing
surfaces. Many are technically renameable but intentionally retained for upstream
merges. Do not call every retained symbol a protocol requirement.

The GFE placeholder UID `0123456789ABCDEF` supports existing cross-client behavior;
the true per-client UID path for non-NVIDIA hosts must remain intact. A generated
certificate's CN has not been proven host-enforced, so retain it rather than claim
either safe renaming or a mandatory protocol value.

Keep `moonlight://` parsing internally where it serves address compatibility. Do
not register Perigee as a replacement system handler for Moonlight during the
initial migration. A future Perigee URI scheme requires a separate launch/security
contract; it is not a reason to change host protocol fields.

## Native and portable configuration

On normal Linux native QSettings, the planned user file is:

```text
$XDG_CONFIG_HOME/io.github.BarMy49/Perigee.conf
```

With default XDG configuration this is
`~/.config/io.github.BarMy49/Perigee.conf`. Current Moonlight uses
`~/.config/Moonlight Game Streaming Project/Moonlight.conf`. Never make Perigee write
or fall back to Moonlight's namespace. Use QStandardPaths for native cache/data;
verify their actual outputs after metadata initialization rather than hardcoding
Qt's platform-specific path algorithm.

Portable design: use `perigee-portable.dat` and a dedicated `perigee-data/` tree in
the selected portable working directory, including `config/`, `cache/`, `boxart/`,
`qmlcache/` and `logs/`. With the approved organization/application names the INI
file is under `perigee-data/config/io.github.BarMy49/Perigee.ini`. Do not honor a
Moonlight `portable.dat` automatically or reuse its generic cache/log folders.
Keep this separate from normal native configuration; add native/portable coexistence
checks before distributing portable artifacts.

Flatpak must use `io.github.BarMy49.Perigee` and its own sandbox storage. A typical
Moonlight Flatpak source file is under
`~/.var/app/com.moonlight_stream.Moonlight/config/Moonlight Game Streaming Project/Moonlight.conf`,
but import requires user selection of the actual file. Do not request broad home
access merely to discover it. User-selected files/portals are the intended boundary.

Qt references: [QSettings locations and atomic sync](https://doc.qt.io/qt-6/qsettings.html),
[QStandardPaths](https://doc.qt.io/qt-6/qstandardpaths.html).

## Explicit import contract

Planned interface, not an existing command:

```text
perigee import-moonlight --source PATH [--include-pairing]
```

1. Run a dedicated startup branch before any normal preference or identity consumer.
   Refuse an existing/nonempty destination or in-use profile; coordinate importer
   and normal startup with the same destination lock to prevent first-run races.
2. Require an existing explicit source file, read it with QSettings INI semantics
   and disabled fallback lookup. Never edit, rename, move or delete the source.
3. Import only known compatible preferences with validated types/ranges and a
   coherent host set following the existing backup/recovery behavior. Preserve
   host certificate pins with their host UUID/address records. Do not copy caches,
   logs, unrelated UI state, or arbitrary future settings blindly.
4. Without `--include-pairing`, exclude client `certificate`, `key`, and `uniqueid`.
   Retain known host pins; pair as the new Perigee client using the normal flow.
5. With `--include-pairing`, validate the complete tuple, including a readable
   certificate, readable matching private key, and valid existing unique ID. Refuse
   an incomplete/corrupt tuple; do not silently generate replacements and claim
   pairing was imported. Preserve bytes/values and matching host pins.
6. Validate all destination data in protected temporary storage; publish atomically
   only after checks pass. Use owner-only directories/files and check write status.
   On failure, leave no partially active profile or credential-bearing temp files.
7. Print only safe summaries: source/destination, counts, mode, warnings, outcome.
   Never print credentials or clipboard material. Re-running against a completed
   destination refuses to overwrite it; no implicit merge or reset option in v1.

User-facing consequence must be clear: local settings remain independent, but
copied credentials identify both applications as the same host-side client. Apollo
permissions and virtual-display identity can be shared. Unpairing that identity
can affect both. Concurrent sessions need explicit testing and are not promised.

Ordinary preferences export always excludes client credentials and identity
bundles. A future explicit pairing backup is a separate feature, not permission to
leak secrets into a general settings file. The new client does not mutate host
permissions during import.

## Desktop, assets, packages, and attribution

- Set `QGuiApplication::setDesktopFileName()` without the `.desktop` suffix and
  SDL Wayland/X11 identifiers to the same full application ID.
- Desktop entry: `Name=Perigee`, `Exec=perigee`,
  `Icon=io.github.BarMy49.Perigee`; desktop-oriented categories/description and
  correct optional StartupWMClass. Do not register a Moonlight URI handler.
- AppStream: matching component/launchable/binary/icon, appropriate Perigee URLs
  based on the observed repository, preserved license/attribution, and accurate
  screenshots/features. Do not list unimplemented Apollo features as available.
- Linux package: `perigee`, no Moonlight `Conflicts`/`Obsoletes`; staged manifest
  must own only Perigee files. Install user-scoped during development after metadata
  is complete. Do not install the upstream baseline system-wide.
- AppImage: planned `Perigee-<version>-<arch>.AppImage`, independent identity/data
  behavior; retain upstream's build/dependency strategy unless evidence requires change.
- Icons: no artwork generation/replacement in planning. Audit source asset terms;
  an explicitly attributed existing asset may be installed under a distinct basename
  only after that audit, pending original Perigee artwork. Packaging remains pending
  if a redistributable icon has not been established.
- Retain Moonlight and contributing-project acknowledgements, copyright/license
  notices, and a source/provenance record. Do not rename upstream notices into
  Perigee authorship. Detailed obligations are in
  [repository analysis](REPOSITORY_ANALYSIS.md#licensing-observations).
- Do not show upstream Moonlight updates as Perigee updates. Disable the artifact
  feed until an independent release channel exists. Mapping/compatibility/STUN
  endpoints have separate roles and remain until individually assessed.

## Incremental tasks and regression gates

- **1A:** Linux target + early Qt/SDL runtime identity/log prefix only. No install,
  migration, protocol edits, or streaming-title edits in this smallest task.
- **1B:** Complete desktop/AppStream/install/resource labels, visible CLI/QML/stream
  branding, portable isolation and artifact update-feed behavior. Session title
  changes must remain cosmetic, without restructuring streaming.
- **1C:** Implement explicit import only after native identity/isolation passes.
- **Later distribution:** Original approved artwork, packaging manifests, independent
  release metadata, accurate screenshots, attribution, and cross-platform audit.

Required tests: clean native/portable launch, XDG overrides, restart persistence,
independent logs and cleanup, no Moonlight file modifications, both apps installed
and launched, Wayland taskbar grouping and X11 WMCLASS, metadata validation,
Flatpak isolation/file selection, explicit/non-pairing import, interrupted/corrupt
imports, destination races, source immutability, and real host pairing/authentication
after migration. Assert no credential regeneration on a valid imported tuple.

All runtime/migration/package tests are pending. The source-backed identity choices
do not constitute successful coexistence or pairing verification.
