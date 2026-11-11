# Memory-constrained build execution

Use one compiler job. `scripts/limit-build-memory.py --report NEW_REPORT -- COMMAND`
runs a native/Flatpak build under temporary user-systemd resource limits and
follows descendant Flatpak scopes for io.github.BarMy49.Perigee. Scope ownership
is determined from the launched process tree; unrelated running applications are
not limited or stopped. No service files or IDE settings are installed/changed.

The controller/compiler scopes have MemoryHigh=640M, MemoryMax=896M and
MemorySwapMax=128M. With --flatpak, the builder parent has MemoryHigh=96M, MemoryMax=256M and no
swap, keeping its reclaimable cache from competing with a compiler. Aggregate
memory.current plus swap is sampled at approximately 0.2-second intervals and the
build is stopped if samples exceed 896 MiB, leaving headroom below a 1 GiB budget.
Reports identify per-scope sample peaks and combined sample peak. Samples are not
continuous peak measurements. Kernel per-scope limits remain in force between
samples. The helper terminates only its own verified scopes on guard failure.

A real SDK preflight confirmed that limits follow Flatpak's reassigned scope;
a harmless exit-7 check confirmed status propagation. Do not rely on an enclosing
systemd-run scope alone: Flatpak can move its process into a new application scope.
The helper requires a user systemd/cgroup-v2 session. It fails rather than silently
running an owned Flatpak scope when limits cannot be applied.

The first corrected-manifest clean build stopped safely at a sampled aggregate
985931776 bytes (about 940 MiB), below 1 GiB but above the conservative guard.
No source/optimization change was made. Its builder parent had been allowed to
retain substantial cache. The successful retry lowered that parent-only budget and resumed
the identical manifest from completed module caches. Compiler settings and the
1 GiB budget are unchanged. Native builds omit --flatpak so their compiler can
use the normal 896 MiB RAM/128 MiB swap scope. Logs/reports belong outside the IDE project, under
/home/barmy/.cache/perigee-build/flatpak-clean-20261110 for this run.

Keep SDKs, source mirrors and build output outside the project to avoid IDE
indexing. Avoid parallel package builds or test compilation. Do not close user
applications, increase the IDE heap, disable memory limits, lower optimization or
change media code merely to make a package build pass.

Podman container checks use one compiler job and cgroup-v2 limits. Verify actual
memory.max=939524096 and memory.swap.max=134217728 before compiling. In this local
Podman/systemd combination, --memory=896m --memory-swap=1g unexpectedly produced
a 1 GiB swap limit in the kernel rather than a 1 GiB combined limit. The running
build's own transient scope was corrected with MemorySwapMax=128M; no unrelated
scope was changed. Observed swap was zero when this was discovered. CLI argument
inspection alone is insufficient resource evidence. For a new container, use an
entry shell waiting for a readiness file created by the controller only after
verifying/correcting that container's own limits. A self SIGSTOP from namespace
PID 1 is not a reliable barrier: it was ignored during the RPM build's dependency
setup. Its limits were corrected before compilation; lifetime kernel swap peak
at that point was 128 MiB. Container verification remains separate from GPU/
streaming acceptance.


The follow-up Fedora CI configuration serializes matrix jobs and passes Docker
--memory=896m --memory-swap=1g --cpus=1. A first step asserts memory.max=939524096,
memory.swap.max=134217728 and cpu.max="100000 100000". It fails closed if the hosted
runner does not apply these values. The previous successful remote run predates
these explicit hosted limits; its result is not evidence of their enforcement.
