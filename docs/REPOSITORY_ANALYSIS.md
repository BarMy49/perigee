# Repository analysis and technical report

Research date: 2026-10-25. This is source/history evidence and an approved design,
not a claim that any Perigee feature was implemented or run. See
[implementation status](IMPLEMENTATION_PLAN.md#current-state),
[architecture](ARCHITECTURE.md), [protocol](APOLLO_PROTOCOL.md), and
[Fedora verification](FEDORA_BUILD.md#verification-status).

## Executive summary

Perigee is feasible as an incremental Moonlight Qt fork. Apollo-compatible host
metadata, permission display, text clipboard, and display launch options can live
primarily in the Qt application. Keep the streaming pipeline and pinned Common C
unchanged through the initial milestones. Command execution is a verified later
exception requiring an additive control-stream API.

The main constraints are the SDL/Qt event-loop boundary, Wayland focus/clipboard
rules, careful migration of a credential bundle, and incomplete host functionality.
Do not promise image clipboard, native file transfer, general physical-monitor
selection, seamless live resolution changes, or compositor-independent HDR/VRR.
Source inspection does not establish the cause of the user's Artemis/Vibemis
crashes. Those failures have not been reproduced in this environment.

## Inspected revisions

All five repositories were accessible. Default branch names and HEADs were checked
against Git remotes and cloned outside the primary working tree. References are
temporarily cached under `%TEMP%/perigee-research-20261025/`; the cache is optional,
not a durable project dependency. This manifest and pinned links are durable.

| Repository | Branch | Exact inspected SHA | HEAD date |
| --- | --- | --- | --- |
| [Moonlight Qt](https://github.com/moonlight-stream/moonlight-qt) | `master` | [`ac296864ace96f51ad75f78b90644bdbca76c332`](https://github.com/moonlight-stream/moonlight-qt/commit/ac296864ace96f51ad75f78b90644bdbca76c332) | 2026-10-24 |
| [Apollo](https://github.com/ClassicOldSong/Apollo) | `master` | [`adc5c5a0bd80831ce495434bb16aee2cd4175fb8`](https://github.com/ClassicOldSong/Apollo/commit/adc5c5a0bd80831ce495434bb16aee2cd4175fb8) | 2026-05-21 |
| [Artemis Qt](https://github.com/wjbeckett/artemis) | `develop` (default) | [`afe2de7f2b24a2f6161f5672e8aa38450fa793ef`](https://github.com/wjbeckett/artemis/commit/afe2de7f2b24a2f6161f5672e8aa38450fa793ef) | 2025-08-31 |
| Artemis additional branch | `master` | [`9835c681d2de630e04de36a58a9c73e35febd2fa`](https://github.com/wjbeckett/artemis/commit/9835c681d2de630e04de36a58a9c73e35febd2fa) | 2025-09-11 |
| [Vibemis](https://github.com/navyas321/vibemis) | `vibemis-main` | [`04beac9efeeafc89e9d66039840dec7936124c86`](https://github.com/navyas321/vibemis/commit/04beac9efeeafc89e9d66039840dec7936124c86) | 2026-10-24 |
| [Moonlight Common C](https://github.com/moonlight-stream/moonlight-common-c) | `master` | [`41cedb89d6b8592f99c09975c764ca3875034726`](https://github.com/moonlight-stream/moonlight-common-c/commit/41cedb89d6b8592f99c09975c764ca3875034726) | 2026-10-24 |

The primary Perigee checkout equals the Moonlight Qt source SHA above. Its observed
remote is `https://github.com/BarMy49/perigee.git`. Existing user changes were an
added/modified `TODO.md` and untracked `.idea/`; preserve them. No source, build,
dependency, application metadata, or icon changes are part of documentation work.

Release API observations on the research date:

| Project | Latest stable API result | Relevant assets/limitation |
| --- | --- | --- |
| Moonlight Qt | `v6.2.0`, published 2026-10-04 | Linux AppImage, source and other platform artifacts |
| Apollo | `v0.4.6`, published 2025-07-13 | Windows installer; researched master includes later changes |
| Artemis | `/releases/latest` returned 404; listed recent releases were prereleases | `0.6.7-dev.20250831.0017+afe2de7` had Linux tar/Flatpak artifacts; an earlier listed prerelease had an AppImage |
| Vibemis | `0.6.0`, published 2026-10-24 | Linux x86_64 AppImage |

These are time-stamped observations, not a promise about future releases. API
sources: [Moonlight](https://api.github.com/repos/moonlight-stream/moonlight-qt/releases/latest),
[Apollo](https://api.github.com/repos/ClassicOldSong/Apollo/releases/latest),
[Artemis releases](https://api.github.com/repos/wjbeckett/artemis/releases?per_page=3),
[Vibemis](https://api.github.com/repos/navyas321/vibemis/releases/latest).

## Dependency relationships

| Source | Dependency | Pinned SHA / relationship |
| --- | --- | --- |
| Moonlight Qt / current Perigee | `moonlight-common-c/moonlight-common-c` | `04053843c90c4268583f2496b997d54d59f03957`, upstream repository |
| Same | `qmdnsengine/qmdnsengine` | `920c097ffa742e2968290f15d4dde6693aec02e5` |
| Same | `app/SDL_GameControllerDB` | `8d9fefd7b810f2541f78cc7a8ccbd185bc84c7a5` |
| Pinned Common C | ENet / nanors | `aca87840b57f045a1f7f9299e4b1b9b8e2a5e2f1` / `b1e3c22ca0cdc0bb83e3cd6ed1a2fc77869ed99a` |
| Latest Common C | nanors | `a9b1636f9c98dbfdd0daeaf6982ccd24d1c30f29`, different from the Qt pin |
| Artemis `develop` | ClassicOldSong Common C fork | `ad329b240f18826f320ce6a99226b36354b86b59` |
| Artemis `develop` | qmdnsengine / h264bitstream / libsoundio / prebuilts | Separate older submodules; not the current Moonlight dependency layout |
| Vibemis | navyas321 Common C fork, branch `vibemis-rtp-timestamp` | `7974aaef40a8e7f1d3130e649656f58b02268456` |
| Vibemis | qmdnsengine | `b7a5a9f225d5e14b39f9fd1f905c4f505cf2ee99` |
| Apollo | host-side `third-party/moonlight-common-c` | `c999436858471dfefa7617af3b7dc03ec1644ce4`; not the Qt client's dependency |

Sources: [upstream .gitmodules](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/.gitmodules),
[Artemis .gitmodules](https://github.com/wjbeckett/artemis/blob/afe2de7f2b24a2f6161f5672e8aa38450fa793ef/.gitmodules),
[Vibemis .gitmodules](https://github.com/navyas321/vibemis/blob/04beac9efeeafc89e9d66039840dec7936124c86/.gitmodules),
[Apollo .gitmodules](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/.gitmodules).

Latest Common C differs from Qt's pin in `ControlStream.c`, `InputStream.c`, and
nanors, including input batching/safety and ENet disconnect changes. Do not advance
the pin merely because master is newer. Vibemis's fork additionally diverges in
audio/microphone, RTP/RTSP, platform and crypto code; wholesale replacement is not a
small way to obtain a server-command function.

## Ancestry and fork differences

`git merge-base` established Moonlight common ancestor
`1bf86f52d368935e60780acc2d9d3c5265f6bce8` (2025-07-04) for both downstream clients.
Vibemis includes the researched Artemis `develop` head in its ancestry. Cherry-picks
and independent implementations mean graph divergence is not a reliable count of
missing fixes or a security-quality ranking.

| Area | Moonlight Qt | Artemis `develop` | Vibemis |
| --- | --- | --- | --- |
| Foundation | Current maintained client | Older foundation plus Apollo managers and branding | Artemis descendant plus substantial later changes and selected upstream ports |
| Host integration | Standard host discovery/pairing/apps | Clipboard, command/permission managers, UUID parsing and launch parameters | Inherits these and adds fixes; some optimistic detection remains |
| Session | GUI-thread SDL loop after asynchronous connection start | Different `ExecThread`/`execInternal` path on Linux | Further thread, queued-call, lifetime and UI handling changes |
| UI | Existing Qt Quick Controls shell | QML quick menu and Apollo configuration | Handheld/gamepad UI and offscreen-rendered menu |
| Video | Current upstream renderers and probing | HDR/AV1/renderer preferences and compatibility shims | Extensive VRR pacing/worker/swapchain/timing changes |
| Dependencies | Current upstream pins | Older Common C, qmdnsengine and additional submodules | Different Common C fork and older qmdnsengine pin |
| Tests | Capability probes and artifact-build CI | Some standalone OTP tests/build checks | Additional Qt/standalone tests for timing, updater and related logic |

Artemis `master` is a different older line: comparison against `develop` lacks its
clipboard, quick-menu, command and permission-manager files. Its newer head date
comes from small repository changes and must not select the feature-port source.

Potential regressions are evidence-based concerns, not reproduced failures:
optimistic host detection, built-in command fallback indexing, blocking network
calls, cross-thread GUI/network access, credential exports, and offscreen readback.
Current Vibemis corrects several of these while retaining other inherited patterns.

## Source reference catalog

All links below use immutable commits. Existing local paths are relative to the
repository; names under Apollo/Artemis/Vibemis refer to separate reference trees.

| Reference | File, symbols and important location |
| --- | --- |
| M-start | [`app/main.cpp:421`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/main.cpp#L421), `main()`: metadata at 431, log prefix at 467, SDL labels at 727, desktop IDs at 938, identity initialization at 973 |
| M-host | [`app/backend/computermanager.cpp:361`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/backend/computermanager.cpp#L361), `startPolling()`, `PcMonitorThread`, pairing task and host serialization |
| M-identity | [`app/backend/identitymanager.cpp:29`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/backend/identitymanager.cpp#L29), `createCredentials()`, constructor at 109 and stored identity tuple |
| M-pair | [`app/backend/nvpairingmanager.cpp:218`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/backend/nvpairingmanager.cpp#L218), pairing exchanges and `devicename=roth` |
| M-http | [`app/backend/nvhttp.cpp:200`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/backend/nvhttp.cpp#L200), `startApp()`; paired SSL errors at 435, query/UID behavior at 482 |
| M-session | [`app/streaming/session.cpp:1762`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/streaming/session.cpp#L1762), `start()`, `interrupt()`, `exec()`; connection at 1593 and cleanup task at 1253 |
| M-video | [`app/streaming/video/ffmpeg.cpp:220`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/streaming/video/ffmpeg.cpp#L220), `FFmpegVideoDecoder`; renderer paths under `ffmpeg-renderers/` |
| M-input | [`app/streaming/input/keyboard.cpp:104`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/streaming/input/keyboard.cpp#L104), `performSpecialKeyCombo()` text injection; keyboard capture in `input.cpp:300` |
| M-settings | [`app/settings/streamingpreferences.cpp:13`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/settings/streamingpreferences.cpp#L13), serialized preference keys and `save()` |
| M-build | [`app/app.pro:1`](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/app/app.pro#L1), Qt/C++/target setup; pkg-config at 64, Linux install rules at 523 |
| H-info | [`Apollo src/nvhttp.cpp:900`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/nvhttp.cpp#L900), `serverinfo()`: paired commands, Permission, Windows virtual-display state |
| H-launch | [`Apollo src/nvhttp.cpp:426`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/nvhttp.cpp#L426), `make_launch_session()`: mode, virtualDisplay, scaleFactor; UUID apps at 1127/1166 |
| H-clipboard | [`Apollo src/nvhttp.cpp:1526`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/nvhttp.cpp#L1526), `getClipboard()`, `setClipboard()`; route registration at 1729 |
| H-permissions | [`Apollo src/crypto.h:45`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/crypto.h#L45), `PERM` |
| H-admin | [`Apollo src/confighttp.cpp:897`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/confighttp.cpp#L897), `updateClient()`; separate authentication at 179 and routes at 1551 |
| H-control | [`Apollo src/stream.cpp:1004`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/stream.cpp#L1004), server commands, clipboard/file nonce stubs |
| A-http | [`Artemis app/backend/nvhttp.cpp:657`](https://github.com/wjbeckett/artemis/blob/afe2de7f2b24a2f6161f5672e8aa38450fa793ef/app/backend/nvhttp.cpp#L657), clipboard methods; launch options at 203 |
| A-detection | [`Artemis app/backend/nvcomputer.cpp:210`](https://github.com/wjbeckett/artemis/blob/afe2de7f2b24a2f6161f5672e8aa38450fa793ef/app/backend/nvcomputer.cpp#L210), unfulfilled ApolloVersion expectation, commands and permission parsing |
| A-clipboard | [`Artemis app/backend/clipboardmanager.cpp:190`](https://github.com/wjbeckett/artemis/blob/afe2de7f2b24a2f6161f5672e8aa38450fa793ef/app/backend/clipboardmanager.cpp#L190), optimistic support and focus-based synchronization |
| V-clipboard | [`Vibemis app/backend/nvhttp.cpp:702`](https://github.com/navyas321/vibemis/blob/04beac9efeeafc89e9d66039840dec7936124c86/app/backend/nvhttp.cpp#L702), paired-certificate clipboard methods |
| V-commands | [`Vibemis app/backend/servercommandmanager.cpp:304`](https://github.com/navyas321/vibemis/blob/04beac9efeeafc89e9d66039840dec7936124c86/app/backend/servercommandmanager.cpp#L304), host-list indexing and send-status handling |
| V-overlay | [`Vibemis app/backend/quickmenumanager.cpp:302`](https://github.com/navyas321/vibemis/blob/04beac9efeeafc89e9d66039840dec7936124c86/app/backend/quickmenumanager.cpp#L302), `renderToSurface()`, framebuffer readback and surface copy |

## Reuse recommendations

| Behavior | Preferred source/evidence | Action |
| --- | --- | --- |
| Virtual display/scaling | H-launch; Artemis [`3e9e6b51`](https://github.com/wjbeckett/artemis/commit/3e9e6b51b92ca3a76dc99aec1a7b7d33b96974d7) | Adapt verified query options behind capabilities; do not copy global preference coupling |
| Permission visibility | H-permissions; Artemis [`442f878d`](https://github.com/wjbeckett/artemis/commit/442f878de8c4bb08ed63d162ceee2f40a14de8f5) | Reuse host field definitions and UI concept, with authenticated unknown/denied states |
| Clipboard direction/focus concept | H-clipboard; Artemis [`c8eaaed7`](https://github.com/wjbeckett/artemis/commit/c8eaaed73508be60ebc16360f0df3a4c05a25d80) | Reimplement asynchronous service and platform ownership; focus callbacks alone do not promise continuous sync |
| Clipboard TLS correction | Vibemis [`8cf3e934f7890781c3ef798f4d531351711e72f1`](https://github.com/navyas321/vibemis/commit/8cf3e934f7890781c3ef798f4d531351711e72f1) | Adapt missing-pin refusal and paired-peer handling; include negative tests |
| Command-index safety, credential export, ownership | Vibemis [`21540ac8`](https://github.com/navyas321/vibemis/commit/21540ac842c210af2d4ea2dd00e871763980a000) | Preserve reviewed rules without transplanting unrelated managers |
| Remove blocking command probes | Vibemis [`37d07ddd`](https://github.com/navyas321/vibemis/commit/37d07ddd1fe9dcb2656bdbfebc67ee1e616a3252) | Do not probe guessed endpoints or synthesize commands |
| Keyboard mapping | Vibemis [`010d60c6`](https://github.com/navyas321/vibemis/commit/010d60c608a2fa3a971761a2185f592abdac3de5) | Reproduce layout failures and evaluate individual mappings before porting |
| Core commands | ClassicOldSong Common C pinned by Artemis, `LiSendExecServerCmd()` in `src/ControlStream.c` | Later additive port only, verified wire semantics and separate dependency review |
| HDR/AV1 renderer fixes | Current Moonlight renderer/probing implementation | Keep upstream; no old-fork shims without a demonstrated remaining issue |
| VRR/RTP/microphone/handheld UI | Vibemis-specific code | Do not port in initial milestones; independent architectural/hardware review required |

## Feature implementation matrix

Difficulty is qualitative: low means metadata/local presentation; medium means
bounded application integration; high means cross-thread/platform work, core change,
or absent host capability. Risk identifies the actual failure boundary, not an
invented schedule or probability. M = upstream Moonlight; A = Artemis; V = Vibemis.

| Feature | Existing implementation | Required changes | Difficulty | Risk | Priority |
| --- | --- | --- | --- | --- | --- |
| Host extension detection | Host fields; A/V partial/optimistic detection | Authenticated capability values, standard-host fallback | Medium | False capability/authorization | High |
| Virtual-display launch | Apollo and A/V parameters | Per-host options and driver state | Medium | Host display disruption | High |
| Resolution/refresh | M mode; Apollo extended parsing | Validated launch options; fractional timing investigation | Medium | Unsupported modes/timing | High |
| Live display/physical monitor switching | No general paired API verified | Further host evidence; hide unsupported actions | High | Session/topology disruption | Deferred |
| Text clipboard | Apollo Windows HTTPS; A/V clients | Async service, SDL adapter, consent and lifecycle | High | TLS/privacy/focus/stalls | High |
| Unicode typing | M UTF-8 text injection | Preserve; layout/dead-key/IME validation | Medium | Incorrect input | High |
| Image clipboard | Host text-only API | Host extension needed | High | Unsupported wire behavior | Deferred |
| Shortcuts and Windows keys | M capture/local command behavior | Configurable mapping, escape binding, compositor tests | Medium-high | Lost/stuck input | High |
| Desktop pointer/cursor | M absolute/relative modes | Clear settings and capture controls | Medium | Focus/capture regression | High |
| In-stream controls | M debug/status text and local actions | Lightweight interactive panel | High | Rendering/input interference | High |
| Desktop UI | M Qt Quick Controls shell | Incremental navigation/themes/accessibility | Medium | UI regressions | High |
| Profiles | M global preference model | Named session snapshots | Medium | State leakage | Later |
| File transfer | Apollo flags/stub; no complete native path | Optional existing SFTP integration | Medium | Service/credential assumptions | Later |
| H.264/HEVC/AV1 and 4:4:4 | M negotiation/probing | Preserve; verify actual hardware/profile | Existing | Software fallback/hardware limits | Critical |
| HDR/high refresh | M conditional platform support | Hardware/compositor validation | High validation effort | Driver/display behavior | Critical preservation |
| Enhanced VRR | V timing/worker changes | Separate measured investigation | High | Pacing regressions | Deferred |
| Multiple remote displays | One M active session; host-specific behavior | Separate host/topology design | High | Unsupported multi-session assumptions | Later |
| Host commands | Apollo control packet, A/V fork API | Additive core function and host-list-only UI | High | Wrong command/ack claims | Later |
| RPM/Flatpak/AppImage | M metadata/AppImage CI | Independent identity and packaging checks | Medium-high | Runtime/ID collisions | High |

YUV 4:4:4 is expected to improve fine colored text by retaining chroma resolution;
that is a technical inference, not a measured result. Compare syntax/text at native
scale. Do not equate NVENC encode capability with client decode support or enable
software decode for clarity without explicit user choice. See the
[NVIDIA decoder capability guide](https://docs.nvidia.com/video-technologies/video-codec-sdk/13.1/nvdec-video-decoder-api-prog-guide/index.html)
and M-video/M-session probes.

## Risks and mitigations

| Risk | Severity / reason | Mitigation |
| --- | --- | --- |
| Apollo API instability | High: unversioned optional fields, incomplete handlers | Freeze references; capability fixtures; fail closed and retain standard-host behavior |
| Qt/SDL lifecycle | High: GUI loop suspended, teardown races | Worker-owned network, SDL-owned clipboard, generation tags and bounded ownership |
| Wayland restrictions | High: compositor focus/security policy | GNOME/KDE tests, explicit limitations/manual actions, no privilege bypass |
| Decoder/pacing regression | Critical: core product characteristic | Preserve pipeline/pins, no readback, repeated hardware baseline comparisons |
| Qt/FFmpeg/libplacebo drift | High: optional features can disappear at build time | Record package versions and qmake output; keep current upstream compatibility |
| Old fork dependencies | High: broad downstream changes | Adapt narrow behavior; do not replace dependency sets or session architecture |
| Clipboard/input security | High: remote data, secrets, stuck keys | Pin trust, bound transfers, opt-in, no payload logs, reserved escape/release tests |
| Identity/packaging collisions | High: settings and installed paths derive from names | Early isolated metadata, consistent desktop IDs, coexistence tests |
| Migration/authentication errors | High: key/cert/UID and host pins form a bundle | Explicit validated atomic import into unused destination; explain shared host identity |
| Unsupported display controls | High: capture mode is not arbitrary host topology | Implement verified launch behavior only; separate future API investigation |
| License/provenance omissions | High for distribution | Per-file license/asset review, notices and corresponding-source/build material |

## Licensing observations

The inspected root licenses of all five repositories contain GPLv3 text. Moonlight
AppStream metadata declares `GPL-3.0+`; do not infer an identical later-version grant
for every downstream file solely from a root license document. Reuse must preserve
the applicable source notices, attribution, modification/provenance information,
and distribution obligations. Binary releases need the applicable license material
and access to corresponding source, including relevant modifications and build
material. Internal names do not need cosmetic changes to satisfy attribution.

Bundled dependencies/assets retain their own terms. h264bitstream has LGPLv2.1
license text; Qt/FFmpeg configurations and bundled rendering/font/controller data
need their own release inventory. No incompatible asset or source reuse is approved
by this report without the specific file review. Native distribution codec policy
is separate from copyright license compatibility.

Primary license evidence: [Moonlight](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/LICENSE),
[Apollo](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/LICENSE),
[Artemis](https://github.com/wjbeckett/artemis/blob/afe2de7f2b24a2f6161f5672e8aa38450fa793ef/LICENSE),
[Vibemis](https://github.com/navyas321/vibemis/blob/04beac9efeeafc89e9d66039840dec7936124c86/LICENSE),
[Common C](https://github.com/moonlight-stream/moonlight-common-c/blob/41cedb89d6b8592f99c09975c764ca3875034726/LICENSE.txt),
[h264bitstream](https://github.com/moonlight-stream/moonlight-qt/blob/ac296864ace96f51ad75f78b90644bdbca76c332/h264bitstream/LICENSE).

## Evidence limits and remaining investigations

- No Fedora compile, live host/client session, GPU test, or crash reproduction was
  performed. Actual hardware, drivers, network, compositor and Sunshine version
  must be captured in Milestone 0.
- Default branches and relevant fork history were inspected, not every historical
  tag/branch. Results are limited to the recorded revisions, not a protocol promise.
- No separate full Sunshine source audit or NVENC SDK integration was performed;
  standard-host compatibility must be checked against a recorded real host version.
- Live virtual-display changes, physical monitor selection, fractional pacing,
  image clipboard, enhanced VRR and native transfer remain unverified/deferred.
- Fedora package pages and accessible RPM Fusion package indexes were inspected;
  DNF resolution was not executed. RPM Fusion configuration/Fedora Quick Docs were
  access-protected. GNU web pages timed out; repository license texts were inspected.
- [Build verification](FEDORA_BUILD.md#verification-status) and
  [the implementation ledger](IMPLEMENTATION_PLAN.md#verification-ledger) distinguish
  evidence collected from work still requiring a suitable environment.
