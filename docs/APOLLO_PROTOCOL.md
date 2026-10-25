# Apollo protocol and client extensions

Status: verified source research plus approved client design. Milestone 2 implements
read-only capability metadata and an explicit HTTPS administration link; Task 3B adds opt-in text clipboard, focused GNOME automatic wiring and controller fixtures.
Primary-target automatic behavior is user-accepted; secondary-platform coverage remains a follow-up. Other session
extensions remain unimplemented. Host snapshot:
`ClassicOldSong/Apollo` master `adc5c5a0bd80831ce495434bb16aee2cd4175fb8`.
Client/fork revisions are in the [manifest](REPOSITORY_ANALYSIS.md#inspected-revisions).
Wire details below are source observations, not executed integration-test results.

## Host architecture and authentication boundaries

Apollo is a Sunshine-derived host. `src/nvhttp.cpp` implements GameStream discovery,
pairing, server information, apps, launch/resume and the clipboard HTTPS endpoint.
`src/rtsp.cpp` and `src/stream.cpp` establish/control media sessions. Host process,
display and encoder management are in `src/process.cpp`, `src/display_device.cpp`,
and `src/video.cpp`; Windows virtual displays depend on SudoVDA/driver state.

GameStream paired HTTPS uses the client's TLS certificate. The host associates the
verified certificate with a named client record (`get_verified_cert()` reads
`request->userp`). That record has permissions and a host-generated client UUID.
The launch session records that UUID; clipboard access checks it against active
RTSP sessions. It is not sufficient for some other client to be streaming.

`src/confighttp.cpp` is a separate administrative HTTPS/web interface. Its
`authenticate()` checks origin and administrative session-cookie state. Pairing
does not grant that administrative login. Initial Perigee exposes permissions and
a user-configured HTTPS administration link; it does not store admin credentials
or call administrative writes through GameStream authentication.

## Capability discovery

[Apollo `serverinfo()`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/nvhttp.cpp#L900)
adds the following to the familiar serverinfo response:

| Field | Verified meaning and exposure | Client rule |
| --- | --- | --- |
| `Permission` | Decimal uint32 mask for verified HTTPS client; plain HTTP emits `0` | Only authenticated metadata establishes a permission result |
| repeated `ServerCommand` | Ordered command names, emitted only with server-command permission | Preserve order; no locally synthesized fallback list |
| `VirtualDisplayCapable` | Emitted in the Windows HTTPS branch | Missing means unknown/unsupported, not a parsing error |
| `VirtualDisplayDriverReady` | Windows HTTPS driver state; host may emit true for clients without action permissions | Treat as host-provided availability information, not a bypass or independent proof of a working driver |
| `appversion`, `GfeVersion` | Emulated GameStream/GFE compatibility versions | Do not display these as the Apollo product release |
| `HttpsPort`, `ExternalPort` | Existing connection endpoint information | Reuse resolved host/port handling, including custom ports |

The inspected host does not emit `ApolloVersion`. Artemis parses such a field in
`NvComputer` but its clipboard/command managers also use optimistic support checks.
Do not port either as authoritative product detection.

Approved interpretation: record optional Apollo-compatible capabilities, their
authenticated provenance and authorization state. Distinguish absent, malformed,
unauthenticated and explicitly denied information. Plain Sunshine with no extension
fields must retain standard behavior and must not receive speculative requests.
A candidate clipboard capability requires opt-in, authenticated permissions and an
active session; endpoint success confirms availability. A version string is not a
substitute for those checks.

## Implemented metadata boundary

`app/backend/apollocapabilities.{h,cpp}` is a Qt Core value/parser component.
`NvHTTP` records successful reply provenance: HTTPS, same resolved host/port,
`/serverinfo`, and an exact nonempty peer/pinned certificate match. The parser also
requires paired status. This adds no requests and changes no existing TLS policy.
HTTP, missing pairing or unpinned replies cannot establish grants or expose command
names. Malformed fields remain invalid; authenticated zero is an explicit denial.
Unknown uint32 bits are preserved without inventing meanings. Repeated command
names retain wire order and duplicates; display flags accept true/false and 1/0.

Only direct, non-namespaced fields under a successful root are considered. Input,
field repetition and command lengths are bounded; malformed XML, DTD/entities,
duplicate singleton fields and uint32 overflow do not establish capabilities.
Snapshots replace previous values; offline polling clears authenticated grants,
commands and display values. Capabilities are never persisted. The UI renders a
plain-text snapshot and identifies the fields as Apollo-compatible, without claiming
a host product or version. Reopening the panel reads the latest model snapshot.

The separately configured per-host administration URL permits explicit HTTPS
host/port/path only, with no userinfo, query or fragment. Empty input removes it.
The client opens it only on user action in the external browser; administrative
login remains there. No inferred port or administrative API call is introduced.

See [Milestone 2 evidence](IMPLEMENTATION_PLAN.md#milestone-2-implementation-and-local-evidence-2026-10-25)
and [fixture commands](../tests/apollo-capabilities/README.md). Pure tests and local
HTTP polling are verified; real pinned TLS, Sunshine/Apollo grants/revocation and
interactive UI/browser acceptance remain pending.

## Permissions

Source: [`crypto::PERM`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/crypto.h#L45).
Values relevant to Perigee:

| Permission | Mask | Host behavior |
| --- | --- | --- |
| `input_controller` / `input_touch` / `input_pen` | `0x00000100` / `0x00000200` / `0x00000400` | Independent input permissions |
| `input_mouse` / `input_kbd` | `0x00000800` / `0x00001000` | Required mouse/keyboard input permissions |
| `clipboard_set` | `0x00010000` | Client-to-host clipboard |
| `clipboard_read` | `0x00020000` | Host-to-client clipboard |
| `file_upload` / `file_dwnload` | `0x00040000` / `0x00080000` | Defined bits; do not imply an implemented transfer path |
| `server_cmd` | `0x00100000` | Advertise/execute host server commands |
| `list` / `view` / `launch` | `0x01000000` / `0x02000000` / `0x04000000` | Application/session permissions |
| `_allow_view` | `view \| launch` | Clipboard handlers require at least one of these bits |

The first paired client receives full permissions; later default clients receive
view/list. A fresh Perigee client may need explicit launch, keyboard, mouse and
clipboard grants in Apollo's web interface. A UI toggle cannot grant permissions.

Administrative `POST /api/clients/update` accepts JSON including `uuid`, `name`,
`display_mode`, `perm`, client do/undo commands, legacy ordering, client-command
allowance and always-use-virtual-display. It requires administrative authentication
and content-type validation. No such writes are in the initial client scope.
[Implementation](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/confighttp.cpp#L897).

## Display and application launch extensions

[Host `make_launch_session()`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/nvhttp.cpp#L426)
reads the following parameters; Artemis `NvHTTP::startApp()` is a client reference.

| Parameter/data | Verified operation | Perigee scope |
| --- | --- | --- |
| `mode=WIDTHxHEIGHTxFPS` | Reads dimensions and floating-point refresh; FPS below 1000 is converted to a millihertz-style integer | Reuse standard dimensions/integer FPS initially; validate fractional behavior separately |
| `virtualDisplay=1` | Requests virtual display, also enabled by host per-client configuration | Add per-host launch option gated by verified capabilities and driver information |
| `scaleFactor` | Scales host rendering dimensions; default 100 | Label as render scaling, not verified Windows DPI configuration |
| app-list `UUID`, `IDX`, `ID` | Additional application identity/order plus standard numeric ID | Parse extensions without breaking standard lists/order |
| launch `appuuid` | Host can find an application using its UUID | Later capability-gated launch; numeric ID remains standard-host fallback |

Standard Moonlight already sends stream dimensions/refresh and can benefit from
host-configured virtual displays without Apollo UI. Apollo also creates displays
in host-managed circumstances. Creating/destroying one is host process behavior,
not a new client display-management service.

Do not claim that changing launch parameters reconfigures an active virtual display.
The inspected `resume()` path only prepares ordinary displays under specific
no-active-session conditions and excludes an already-virtual display. A controlled
reconnect is not proof of a successful host mode change. Physical display
enumeration/selection and multi-display topology lack a verified general paired
API. Current client architecture supports one active session per process.

Apollo has input-only/termination application behavior. Existing normal desktop
launch is the initial supported path; input-only apps need a separate session
design because the current client expects video. An app-list UUID alone does not
prove that every non-Apollo host accepts UUID launch.

## Text clipboard contract

Verified host functions: [`getClipboard()` / `setClipboard()`](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/nvhttp.cpp#L1526).
Routes are registered on the paired GameStream HTTPS server, not the web-admin API.

| Direction | Request | Success/body | Requirements |
| --- | --- | --- | --- |
| Host to client | `GET /actions/clipboard?type=text` | Text response, no JSON/XML envelope; empty text is valid | Verified client certificate, `_allow_view`, `clipboard_read`, matching active stream |
| Client to host | `POST /actions/clipboard?type=text` | Successful HTTP response with no required text body | Same active-stream/view requirements plus `clipboard_set`; UTF-8 text request body |

Use `Content-Type: text/plain; charset=utf-8` for POST. Host `type` must be exactly
`text`; other types return 400. Permission denial returns 401; missing matching
active stream returns 403; host set failure returns 500. Preserve HTTP status
separately from content so empty successful reads and network errors differ.

Windows implementation in `src/platform/windows/misc.cpp` bridges UTF-8 and
`CF_UNICODETEXT`, with newline handling. The inspected Linux host functions are
placeholders returning empty/failure; endpoint existence does not make Linux-host
clipboard functional. Windows Apollo is the initial host acceptance target.

### Approved client policy

- Per-host opt-in, disabled by default, both directions only for that active session.
  No automatic upload of clipboard contents present at connection establishment.
- Establish local/remote baseline revisions without replacing local contents on the
  initial poll. Manual receive is an explicit request to fetch/apply current remote
  text. Later eligible changes synchronize subject to focus and authorization.
- Automatic host polling starts at one second; request timeout is five seconds.
  Serialize network operations and coalesce pending writes to the latest value.
- Limit each transfer to 1 MiB of UTF-8 bytes. Bound remote data while receiving,
  not after an unlimited `readAll()`. A character count is not a byte limit.
- Keep hashes/content revisions only in memory; suppress echoes and stale replies.
  If local text changes after a read starts, that reply cannot overwrite the newer
  local revision. Invalidate old-session generations during disconnect/reconnect.
- Synchronize text clipboard only, not PRIMARY, images, HTML, or file URIs. Never
  log/persist payloads. Do not synthesize remote paste/key actions automatically.
- Capture TLS configuration and the pinned server certificate after pairing on the
  owning thread. New requests must require paired trust, refuse missing/mismatched
  pins, disallow redirects and insecure fallback, and avoid sensitive URL logging.
- Use asynchronous worker networking and SDL clipboard access on its owner thread
  during streaming. Do not add GUI-thread nested event loops from fork code.
- Stop on 401/403 until authorization/session state is refreshed. A definitive
  unsupported endpoint disables that capability for the current connection; do not
  keep probing or synthesize a different protocol. Back off transient failures.
- Measure SDL clipboard read latency. Manual-only fallback is required for a backend
  where focus/security restrictions or stalls prevent acceptable automatic reads.
  Display availability honestly rather than asserting seamless background sync.

Wayland access is compositor-controlled. The
[Clipboard portal](https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Clipboard.html)
extends compatible Remote Desktop/Input Capture sessions, not standalone access;
backend availability and active-session restrictions still apply. Do not request an
unrelated privileged session simply for clipboard access. Test actual GNOME/KDE.

Upstream already has `KeyComboPasteText` using `LiSendUtf8TextEvent()`. It remains
available to compatible standard hosts independently of this clipboard protocol.
It is text injection, not proof of bidirectional sync or complete IME support.

### Implemented text path and platform gate

Task 3B implements the exact raw GET/POST contract through `SessionServices`, with
strict UTF-8/1 MiB bounds, separate HTTP status, paired pin/client configuration,
redirect refusal and five-second asynchronous timeout. `ClipboardSync` owns
in-memory revisions/coalescing and denial/unsupported/backoff state. The product
requires explicit per-host consent and direction-specific grants, starts no
clipboard request at connect, and supports focused manual send/receive.

User-verified manual tests and sub-millisecond-resolution focused read timings
support wiring automatic retrieval on GNOME native Wayland. A worker-owned
one-second timer wakes the SDL owner through coalesced bridge ticks. Initial
local/remote baselines do not transfer existing contents, focus loss pauses
dispatch and invalidates pending reads, and denial stops polling until reconnect.
An unavailable/invalid or over-4-ms SDL read switches the session to manual mode.
The 4 ms budget is a chosen fallback threshold, not a compositor latency guarantee.
KDE, X11 and other backends remain manual-only. The user accepts integrated GNOME
automatic transfers, baseline preservation, focus/reconnect and permission behavior;
secondary-platform and controlled latency coverage remain follow-ups.
`SdlClipboard` uses SDL text APIs on their owner thread, records duration without
content, and never accesses PRIMARY or Qt's clipboard. See
[Task 3B evidence](IMPLEMENTATION_PLAN.md#task-3b-implementation-and-local-evidence-2026-10-25)
for tests, shortcuts and live prerequisites. No real clipboard endpoint has been
contacted by the agent. The user has verified GNOME manual Unicode transfers,
notices, focus/reconnect protection and permission revoke/restore.

## Host commands

[Apollo control handling](https://github.com/ClassicOldSong/Apollo/blob/adc5c5a0bd80831ce495434bb16aee2cd4175fb8/src/stream.cpp#L1004)
implements packet `0x3000`. It reads the first payload byte as an index into
`config::sunshine.server_cmds`, checks `server_cmd`, and starts that configured
command. It is not an arbitrary client-provided shell-command API.

The ClassicOldSong Common C revision pinned by Artemis exposes
`LiSendExecServerCmd(uint8_t)` in `src/Limelight.h` and `src/ControlStream.c`. It sends
`{ index, 0, 0, 0 }` reliably on `CTRL_CHANNEL_SERVERCTL` (`0x08`). Its `int` return
wraps a boolean send result: nonzero is sent, zero failed. Do not assume the usual
zero-success convention, or claim host execution completed; no such response was
verified. That function is absent in both the current Qt pin and upstream master.

Client rules for the later approved exception:

- Do not send until an additive core patch and distributable dependency revision
  have separate review. Preserve the current pin for initial integrations.
- Require an active matching session, authenticated permission and host list.
- Preserve host list order and use only its index in the uint8 range. Refuse missing,
  stale, out-of-range or unadvertised entries; refresh metadata before reuse.
- Never index a locally invented shutdown/restart/sleep list against the host list.
- Show sent/failed status only. Do not retry a potentially destructive command
  automatically after uncertainty or reconnect.
- Do not probe guessed HTTP endpoints or fall back to admin credentials.

## Client support comparison

Here, implemented means source exists, not that the implementation passed tests.

| Feature | Upstream Qt | Artemis `develop` | Vibemis inspected head | Preferred Perigee action |
| --- | --- | --- | --- | --- |
| Basic Apollo streaming/pairing | Standard protocol path | Inherited/modified | Inherited/modified | Preserve upstream and test |
| Extension fields | No Apollo capability layer | ApolloVersion expectation; permissions/commands parsed | Inherited plus fixes; optimistic detection still present | New conservative parser |
| Virtual display / scale options | No dedicated UI/query options | Implemented in launch | Inherited | Adapt verified parameters |
| Fractional refresh | Nominal integer stream configuration | Decimal launch option | Inherited/extended timing work | Separate timing validation |
| Text clipboard | Injection action only | HTTPS methods plus focus/local-change manager | Inherited with trust and lifecycle fixes | Reimplement service; adapt security rules |
| Permission visibility | No Apollo-specific panel | Manager/UI implemented | Inherited | Read-only capability presentation |
| Permission mutation | No paired-client permission control | No verified equivalent paired mutation path | No verified equivalent paired mutation path | Admin web link, no client grants |
| Commands | No required core API | Fork API and unsafe fallback possibilities | Host-index correction and send semantics | Later narrow extension only |
| UUID applications | Standard numeric ID path | Parses/sends extension | Inherited | Later extension with numeric fallback |
| Image/native file transfer | No implementation | No complete verified host-compatible implementation | No complete verified host-compatible implementation | Defer / optional existing SFTP |

See [source catalog and commits](REPOSITORY_ANALYSIS.md#source-reference-catalog).

## Unsupported and unverified behavior

Apollo packet `0x3001` (set clipboard) and `0x3002` (file-transfer nonce request) have
handlers that log/check permission but do not perform those operations. They must
not be advertised as working features. Use the text HTTPS endpoint, not `0x3001`.
Image MIME support in Qt or the local portal does not add remote host image support;
portal file transfer is not a GameStream file-transfer protocol.

The following require new evidence before implementation: live virtual-display
reconfiguration, physical monitor selection, fractional frame pacing correctness,
simultaneous remote displays, images, native transfer, Linux-host clipboard, and
VRR requests specific to Vibepollo. No normal Sunshine feature claim is inferred
from a flag or a comment in either fork.

## Verification requirements

Fixtures: standard-host serverinfo without extensions, HTTP Permission=0,
authenticated zero and partial masks, malformed values, duplicate/ordered commands,
Windows display flags, and absent product-version fields. Test real TLS mismatch,
missing client identity, 400/401/403/404/500, empty text, invalid/oversize UTF-8,
disconnects, old-generation replies, and local/remote edits during a request.

Real acceptance requires the recorded Windows Apollo version, granted/revoked
permissions, a matching active stream, Fedora GNOME/KDE clipboard focus behavior,
and unchanged Sunshine operation. These tests are pending; use the
[verification strategy](IMPLEMENTATION_PLAN.md#verification-strategy) and record
results in [Fedora evidence](FEDORA_BUILD.md#baseline-result-record).
