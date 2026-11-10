# Milestone 6 integration policies

Nine cases exercise strict SFTP destinations and missing handlers, canonical UUIDs,
unique numeric/UUID mappings, pinned HTTPS origins, cached app persistence, and
host-command preparation bounds/permissions/generations. Commands produce only
four-byte fixture values; no transport is activated. SFTP dispatch uses a callback,
not a real external handler. No host, SSH credentials or streaming are used.

```bash
mkdir -p /tmp/perigee-m6-options
cd /tmp/perigee-m6-options
qmake6 /home/barmy/Projects/perigee/tests/milestone6-options/milestone6-options.pro
CCACHE_DISABLE=1 make -j2
./tst_milestone6options
```

See [the unapplied core proposal](../../patches/common-c/README.md) for the separate
inactive compile check. Commands were explicitly deferred by the user; these tests
do not satisfy active transport or live-host acceptance.
