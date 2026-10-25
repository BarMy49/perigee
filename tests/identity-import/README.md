# Explicit identity import checks

This Linux-only standalone Qt Test target requires anonymous-file support and
`/proc/self/fd`. It exercises the Task 1C importer and shared profile
store. It uses synthetic credentials and disposable files, never production
Moonlight profiles or live hosts. No application build depends on this target.

From the source root, after building `app/perigee`:

```bash
mkdir -p ../perigee-build/tests/identity-import
cd ../perigee-build/tests/identity-import
qmake6 ../../../perigee/tests/identity-import/identity-import.pro
make -j2
PERIGEE_TEST_BINARY=/home/barmy/Projects/perigee-build/debug/app/perigee ./tst_identityimport
```

The CLI tests deliberately use an invalid GUI platform and empty display variables
so success requires the early headless import branch. The publication interruption
check kills a child with synthetic credentials staged in an anonymous file, checks
that no profile or named credential file survived, and reacquires the stale lock.
Real pairing/authentication remains outside this suite and was skipped by user
direction. A readable, matching synthetic tuple is not evidence of host acceptance.
