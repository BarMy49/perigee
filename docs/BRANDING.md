# Perigee branding and identity migration

Status: Tasks 1A/1B implement Linux native and portable identity, desktop/AppStream
metadata, visible branding and artifact-update suppression. Native/portable
restart, staged metadata, separate Moonlight state, Wayland IDs and XWayland
properties are verified; visual shell grouping and system install remain pending.
Temporary upstream icon attribution is recorded in [ASSET_NOTICES.md](../ASSET_NOTICES.md).
Task 1C explicit import is implemented and locally verified; live pairing tests
are skipped by user instruction. See
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
| AppStream filename | `io.github.BarMy49.Perigee.metainfo.xml` |
| Installed icon basename | `io.github.BarMy49.Perigee` |
| Qt organization name / application name | `io.github.BarMy49` / `Perigee` |
| Qt organization domain | `barmy49.github.io`, identity metadata only; no live website assumed |

The user explicitly selected the reverse-DNS ID after inspection of the repository
remote. No different GitHub organization or domain ownership is assumed. Initial
platform acceptance is Linux; Windows/macOS packaging remains unverified.

## Baseline identity and sensitive state

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

On normal Linux native QSettings, the implemented and verified user file is:

```text
$XDG_CONFIG_HOME/io.github.BarMy49/Perigee.conf
```

With default XDG configuration this is
`~/.config/io.github.BarMy49/Perigee.conf`. Current Moonlight uses
`~/.config/Moonlight Game Streaming Project/Moonlight.conf`. Never make Perigee write
or fall back to Moonlight's namespace. Use QStandardPaths for native cache/data;
verify their actual outputs after metadata initialization rather than hardcoding
Qt's platform-specific path algorithm.

Implemented Linux portable identity: use `perigee-portable.dat` and a dedicated `perigee-data/` tree in
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

Implemented Linux interface (Task 1C):

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

### Implemented import validation and storage

Only an explicitly named regular UTF-8 INI file is read; there is no automatic
Moonlight discovery, registry import, directory scan or credential copy. Native
`.conf` and portable `.ini` inputs share the QSettings INI reader. Destination
selection follows normal Perigee startup: native settings, or portable settings
when the current directory contains `perigee-portable.dat`. The destination
settings file must not exist, even if empty; dangling symlinks are refused too.
There is no merge, reset or overwrite option. To test without altering native
state, provide a fresh `XDG_CONFIG_HOME` and isolated cache/data/TMPDIR roots.

`settings/moonlightimporter.cpp` implements the allowlist independently of startup
preference objects. Unknown settings, caches, window geometry, platform-specific
`renderer` selection and unrelated future fields are not copied. Known boolean
settings require `true`/`false` or string `0`/`1`; integer fields require decimal
integers. Numeric bounds are:

| Fields | Accepted values |
| --- | --- |
| `width`, `height` | 1–16384 |
| `fps`, `bitrate` | 10–480 FPS; 500–500000 Kbit/s |
| `audiocfg`, `videodec`, `windowmode`, `uidisplaymode`, `capturesyskeys` | 0–2 |
| `videocfg`, `language`, `defaultver` | 0–4; 0–31; 0–2 |
| `packetsize` | 0 (automatic), or 1024–65507 |

Legacy `fullscreen`/`startwindowed` booleans and `defaultver` remain compatible with
the existing preference migration logic. Limits describe import validation, not
hardware support. Source limit is 8 MiB and 100000 settings keys; at most 512 hosts,
16384 total cached apps, 512 characters per host/app name, 253 per endpoint, and
64 KiB per PEM value. Invalid recognized fields reject the entire import.

A nonempty `hostsbackup` takes precedence over `hosts`, matching `ComputerManager`
recovery. The chosen array is validated and published as `hosts`; arrays are never
merged. UUIDs must be valid and unique, app IDs nonnegative and unique within a
host, MAC values empty or six bytes, and ports 1–65535 for nonempty endpoints
(zero is allowed for empty endpoints). Endpoints must be IP addresses or plain
hostnames. Known host/app flags and cached app records are preserved. Nonempty
server pins must parse as exactly one PEM certificate and remain associated with
the chosen host UUID/endpoints, even without optional client identity import.

`--include-pairing` additionally requires one readable certificate, a matching
unencrypted RSA private key and a 1–16 digit hexadecimal client ID. Encrypted keys
are refused without prompting. Original credential and pin byte values, including
PEM CRLFs and client-ID case/leading zeros, are preserved; INI textual encoding may
be normalized by Qt. This validates local consistency, not certificate lifetime,
host trust decisions, host authentication or permission grants.

`settings/profilestore.cpp` supplies the same lifetime profile lock to import and
normal Linux startup, before preference/path/identity consumers. One process per
profile is allowed; a second process fails immediately without GUI initialization
or a new identity. The lock is `<settings-file>.profile-lock`, separate from Qt's
own settings write lock. Dead-process locks are recoverable; destination directories
must be owned by the current user, not symlinks, and not writable by group/others.
New profile directories and published files are owner-only.

Import parses a bounded in-memory source snapshot through an anonymous Linux file
rather than rereading a changing source. QSettings serializes nonsensitive metadata
in an owner-only temporary file; client credentials, client ID and host pins are
excluded from that named file and appended only in memory with INI escaping.
The final anonymous file is reread with QSettings and every key/value is checked
before publication. `O_TMPFILE`, file fsync and atomic no-overwrite `linkat` publish
a complete 0600 profile. Existing files created during publication races are not
replaced. The profile lock remains held throughout. Directory fsync failures after
successful publication are reported as durability warnings, not rolled back.

Filesystems lacking anonymous staging/publication, or environments without usable
`/proc/self/fd`, fail before creating destination settings. There is no fallback to
a named credential file or an overwrite rename. Normal failures remove temporary
metadata; abrupt process death can leave protected metadata or a stale lock, but
no named client credential/pin staging file or partial active profile. The kernel
releases unlinked anonymous files when the process exits.

Exit codes: 0 success/help, 1 validation/storage/profile refusal, 2 invalid CLI
arguments. Messages contain safe counts/status only. No ordinary preferences-export
feature exists yet; default import is credential-free and this task does not add
an export path. Any future ordinary export must preserve the exclusion contract.
Focused fixtures and the exact verification commands are in
[tests/identity-import](../tests/identity-import/README.md); runtime evidence and
remaining acceptance are in the [plan](IMPLEMENTATION_PLAN.md#task-1c-explicit-import).

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

Tasks 1A/1B native/portable paths and identity persistence, Wayland IDs, XWayland
window properties, staged metadata and process coexistence checks passed. Visual
shell grouping, installed launcher/icon-cache behavior, import and distributable
package tests remain pending; live host pairing was skipped by user instruction.
Detailed commands and limitations are in the implementation plan. Translation
catalogs retain upstream contexts; new product strings currently fall back to English.
