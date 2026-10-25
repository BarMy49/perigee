# Session service lifecycle checks

This standalone Qt Core/Network/Qt Test and SDL2 target exercises the Task 3A
worker and event bridge independently of the application and any real host.
`openssl` must be available on PATH: the suite generates temporary, disposable
self-signed client/server identities. No production credentials are read and no
pairing flow is performed. The application does not depend on this target.

From the source root:

```bash
mkdir -p ../perigee-build/tests/session-services
cd ../perigee-build/tests/session-services
qmake6 ../../../perigee/tests/session-services/session-services.pro
make -j2
./tst_sessionservices -o results.txt,txt
```

The fixtures cover owned/bounded payloads, coalesced pointer-free SDL wakeups,
protection of upstream SDL_USEREVENT messages, generation rejection, cancellation
racing a producer, failed wakeups, disabled/invalid startup, worker progress with
no GUI Qt event pumping, pinned local TLS, successful empty responses, redirects,
oversized receiving, timeouts, wrong pins, connection failure, 200 rapid restarts,
and 30 shutdowns with replies pending. Timeout completion and worker shutdown also
run without pumping the GUI event loop. Loopback `/fixture` is a test-server path;
no such endpoint is assumed on Apollo/Sunshine and the app issues no service GETs.

Optional memory/lifetime verification:

```bash
valgrind --tool=memcheck --leak-check=full \
  --show-leak-kinds=definite,indirect \
  --errors-for-leak-kinds=definite,indirect --error-exitcode=99 \
  --log-file=valgrind.log ./tst_sessionservices -o results-valgrind.txt,txt
```

The bridge follows the application's one-active-session contract. All
`SessionServices` owner methods run on its creating (SDL/GUI) thread. Workers own
network objects and timers, receive values only, and never wait for GUI slots.
Call `stop()` before SDL/video teardown; it invalidates/drains messages before
joining the independently stopping worker. These fixtures call no clipboard or window APIs. The app adapter is tested separately
on a focused SDL window; no OS clipboard content is read by this target.

Services remain off by default. Task 3B adds per-host opt-in manual operations and
focused GNOME automatic wiring. The fixtures cover the
exact clipboard URL/method/raw body, UTF-8/size limits, empty reads, revisions,
latest-write coalescing, automatic baselines/echo suppression and denial stop.
Additional checks cover exact platform gating, worker timer wakeups without GUI
event pumping, tick coalescing, focus-loss invalidation, manual fallback, and
receive-only/send-only grants. Linking the SDL adapter permits platform-policy
checks; these fixtures still never read or write the OS clipboard.
These tests do not establish live Apollo/Sunshine session compatibility,
clipboard ownership/latency, media performance, or compositor behavior. See the
[implementation plan](../../docs/IMPLEMENTATION_PLAN.md#task-3a-session-service-bridge).
