# Apollo command extension: review proposal only

The patch in this directory is **not applied**, not linked into Perigee and not a
replacement dependency revision. The application has no active command sender or
command UI. Its base is the existing Common C pin
`04053843c90c4268583f2496b997d54d59f03957`; dependency pins remain unchanged.

The proposal adds one public function and the named channel constant, preserving
all packet tables and existing input/media paths. It sends packet `0x3000`, four
bytes `{index, 0, 0, 0}`, reliably on channel `0x08`. Return 1 means sent, 0 means
refused/failed; it does not acknowledge execution. Calls require a matching active
Apollo session, authenticated permission and the current advertised command list
at the application boundary. The proposed core function rejects inactive,
interrupted, stopping, legacy and unencrypted connections. Callers must serialize
calls against stop/cleanup on the existing SDL owner; it is not a new lifetime API.

Wire provenance is the previously inspected ClassicOldSong Common C revision
[`ad329b240f18826f320ce6a99226b36354b86b59`](https://github.com/ClassicOldSong/moonlight-common-c/blob/ad329b240f18826f320ce6a99226b36354b86b59/src/ControlStream.c)
and the Apollo host control handler recorded in [the protocol](../../docs/APOLLO_PROTOCOL.md#host-commands).
The candidate adapts that wire behavior to the retained upstream core rather than
porting the fork's other changes. The existing Common C GPL license and
corresponding-source obligations remain; no third-party notice is removed.

`app/backend/servercommandrequest.h` prepares only a bounded four-byte value after
checking active generation, authenticated permission, unchanged ordered host list
and uint8 index. There is no invented fallback list, network endpoint or retry.
It does not dispatch anything. Its fixtures cover indices 0/1/255, invalid bounds,
revocation, inactive sessions and stale/reordered lists.

After building the ordinary debug application, run from the source root:

```bash
python3 scripts/check-common-c-proposal.py --build-dir /tmp/perigee-6a-build
```

This checks the patch against the clean pinned core, applies it only in temporary
storage, compiles the candidate control source and links an inactive-session smoke
check against the existing Common C archive. It must refuse indices 0 and 255.
It does not open a socket or send a host command. Active ENet transmission, shutdown
races, actual host permission/list handling and live command acceptance remain
unverified. Unit payload fixtures are not those transport tests.

Before activation, ADR-005 requires separate patch review and a reproducible,
distributable dependency revision. Publishing/committing a fork and changing the
pin need user authorization. No target repository/revision is currently supplied.
On 2026-11-10 the user explicitly chose “Defer commands; advance to Milestone 7.”
Commands remain disabled. This is not authorization to publish a fork, change the
pin or activate the proposal; the review and reproducible-source prerequisites
still apply to any later command implementation.

The proposed Milestone 7 CI review snapshot excludes the unapplied patch and
scripts/check-common-c-proposal.py. They remain in this local development checkout;
the checker command above is for that checkout only. The review snapshot retains
this decision record and the disabled application policy fixtures, without
publishing a Common C fork or activating commands. Older private development
source archives may contain the unapplied proposal; they have not been published.
