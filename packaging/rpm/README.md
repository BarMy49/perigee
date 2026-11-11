# Native Fedora development RPM

This is a local development package, not a published release or Fedora repository
submission. Fedora 44 GNOME Wayland remains the target; first packaging validation
runs on the available Fedora 45 environment. Version 6.2.0 is inherited from
`app/version.txt`; release `0.1` marks this packaging prototype. No update feed is
configured and commands remain disabled with the original Common C pin.

## Source and build

The source snapshot is self-contained, including the pinned initialized submodules.
No network retrieval occurs in the spec. It retains source and dependency notices;
`SOURCE_SNAPSHOT.json` records revisions, local tracked changes, explicit untracked
inclusions and every source file hash. The snapshot is of actual working files,
not an assertion that HEAD alone contains every feature.

From the repository root, with documented build dependencies plus rpm-build,
redhat-rpm-config, appstream and desktop-file-utils available:

```bash
python3 scripts/create-source-archive.py \
    --output /tmp/perigee-rpmbuild/SOURCES/perigee-6.2.0.tar.gz
rpmbuild -ba --define '_topdir /tmp/perigee-rpmbuild' \
    --define '_tmppath /tmp' packaging/rpm/perigee.spec
```

The archive command refuses to overwrite an existing file. Tracked working changes
are included. Any required untracked files must be reviewed and included explicitly
with repeated `--include-untracked relative/path` arguments, including the spec and
packaging/check scripts if they are not yet tracked. Never include IDE data, local
profiles, credentials or build output. Modified/missing/unpinned submodules and
symlinked source files fail closed. Repeating with unchanged inputs produces
byte-identical source archives; binary RPM reproducibility remains a separate gate.
Explicit untracked YAML workflows directly under .github/workflows may be included;
other hidden untracked files and IDE data remain rejected. This allows a development
snapshot to retain its reviewed CI build instructions before they are committed.

`%build` applies Fedora build flags, preserves qmake and builds release mode.
`%install` uses INSTALL_ROOT, installs only Perigee identity files and copies the
applicable static dependency/font notices. The end of `%install` checks the exact staged file list before RPM adds debug
material; `%check` validates desktop/AppStream metadata. Shared-library requirements remain
automatically generated; QML/runtime Wayland/SVG packages are explicit requirements.
The spec has no scriptlets, Moonlight Conflicts/Obsoletes, service or settings files.

A manual native stage can be checked separately:

```bash
mkdir -p /tmp/perigee-native-build
cd /tmp/perigee-native-build
qmake6 /home/barmy/Projects/perigee/moonlight-qt.pro PREFIX=/usr
CCACHE_DISABLE=1 make -j1 release
make -C app -f Makefile.Release INSTALL_ROOT=/tmp/perigee-native-stage install
python3 /home/barmy/Projects/perigee/scripts/check-linux-stage.py /tmp/perigee-native-stage
```

Use `--rpm` for the extended RPM payload with notices and snapshot metadata.
The checker rejects extra files, missing files, symlinks and mismatched desktop
identity, except RPM-generated build-ID links resolving to the client itself. It does not verify installed shell grouping, driver behavior or streaming.

## Remaining acceptance and release gates

- Real system install/upgrade/uninstall, desktop caches, Moonlight coexistence and
  installed launcher behavior require explicit user authorization. A disposable
  root test does not close these gates.
- Fedora 44 clean build/dependency resolution and representative GNOME/KDE
  GPU/audio/input/streaming coverage remain pending.
- Full source/asset review, screenshots, original final branding, reproducible
  binary builds and source publication must precede a public release.
- AppImage/Flatpak and upstream merge maintenance are independent remaining tasks.
  The [AppImage adaptation](../appimage/README.md) uses Perigee names and retains
  the inherited X11/XWayland-only build; its acceptance is recorded separately.

See [third-party notices](THIRD_PARTY.md), [asset provenance](../../ASSET_NOTICES.md)
and the [implementation plan](../../docs/IMPLEMENTATION_PLAN.md).


## Local evidence, 2026-11-10

Release compilation and native staging pass. A source RPM and x86_64 main,
debuginfo and debugsource RPMs were built under `/tmp/perigee-rpmbuild`.
The final source snapshot was overlaid onto the clean-compiled source tree and
`rpmbuild -ba --noprep` reran build/install/check/package; a clean final-source
Fedora 44 rebuild remains pending. Tools were extracted into `/tmp` without a
system install. Helper paths were relocated locally; the ELF attribute file was
loaded explicitly after inspection caught missing automatic dependencies in an
intermediate candidate. The final candidate includes 75 requirements, passes the
16-file main payload check, has no scriptlets and no Moonlight ownership overlap.
Exact local command arguments: `/tmp/perigee-m7-rpm-command.json`.

A dependency/conflict transaction test against a copy of the system RPM database
passes with digest verification, unsigned local packages accepted and disk-space
checks disabled for the sandbox's read-only system mount. Real install/upgrade/
uninstall has not run. A disposable-root attempt failed because chroot is not
permitted, including outside the sandbox. That failure is not a lifecycle pass.
See Task 7A in the plan for hashes, log paths and remaining acceptance.


## Installed lifecycle follow-up

The user installed the reviewed RPM in their terminal. Unsandboxed `rpm -V perigee`
passes with root ownership. Both installed desktop launchers start on GNOME Wayland
with disposable profiles and coexist with separate app IDs/configuration files;
two production settings files remain unchanged across the launches. The user then
removed Perigee. Its installed files are absent, Moonlight verification is unchanged,
the removed launcher no longer resolves, and Moonlight still launches afterward.
Both user configurations remain present. Visual shell grouping, upgrade, media and
Fedora 44 clean-environment acceptance remain pending. No release was published.

## Upgrade candidate and dependency follow-up

The spec accepts `--define 'perigee_rpm_release 0.2'` for a local upgrade candidate;
its default remains 0.1. Use one build job and one LTO job when building with the
[1 GiB resource budget](../BUILD_RESOURCES.md). A higher package release is a test
vehicle, not a public release or an assertion of changed application features.

The Fedora 44 container exposed an undeclared build dependency: libplacebo's
libav integration includes libavformat/avformat.h. The spec and Fedora CI recipe
now explicitly require pkgconfig(libavformat). The retry retains optimization,
media code and dependency pins.


The local 0.2 candidate now passes build/install/check/package under the one-job
1 GiB guard (508112896-byte sampled peak, zero swap). It retains 75 generated and
explicit requirements, passes the exact 16-file payload check, has no scriptlets,
Conflicts or Obsoletes, and passes the copied-RPM-database transaction dry run.
An inventory is /tmp/perigee-rpm-upgrade-inventory.json. Build arguments, log and
memory report are /home/barmy/.cache/perigee-build/rpm-upgrade-command.json,
rpm-upgrade-build.log and rpm-upgrade-memory.json.

Candidate: /tmp/perigee-rpmbuild/RPMS/x86_64/perigee-6.2.0-0.2.fc45.x86_64.rpm.
SHA256: 09e92cee6d1867bddeb115c4757fddd435033f326af83b0918b8ba346b4d6313.
The real upgrade is pending password entry: sudo -n true requires a password.
The reviewable /tmp/perigee-rpm-upgrade-check.sh verifies both artifact hashes,
installs 0.1, upgrades to 0.2, verifies files, removes Perigee, and compares Moonlight
verification and three profile hashes. Each root RPM transaction uses a transient
896 MiB RAM/128 MiB swap scope. It checks dependencies, does not copy credentials,
and makes no persistent service/global policy change. Run it as the desktop user:

```bash
bash /tmp/perigee-rpm-upgrade-check.sh
```

No system installation has been performed for this new candidate. This remains
a Fedora 45 RPM; the successful Fedora 44 source builds/checks are recorded in
../../tests/README.md and do not claim a clean Fedora 44 RPM construction pass.


The user subsequently completed the real upgrade test successfully. Evidence in
/tmp/perigee-rpm-upgrade-result-fUvM2Z records exit 0 and the expected 0.2 NEVRA;
Moonlight verification and all three profile snapshots compare identical. Perigee
is absent afterward and Moonlight remains installed. The pending password/upgrade
status above is superseded. Clean Fedora 44 RPM construction is the next check.

The Fedora CI draft now has a separate RPM lane using normal rpmbuild, one make/LTO
job, a complete committed source snapshot, final payload/metadata checks and a
[file/notice/ELF inventory](../INVENTORY.md). It uploads inventory JSON only;
no RPM release or remote workflow run is performed by this local implementation.

## Clean Fedora 44 package result

Normal clean rpmbuild now passes on Fedora 44, including main/source/debuginfo/
debugsource RPMs. One make job and -flto=1 compiler flags retain Fedora optimization
and hardening. Kernel limits are verified at 896 MiB RAM/128 MiB swap, with no OOM
kills. The final 16-file payload and 75 requirements pass; there are no scriptlets,
Conflicts/Obsoletes or Moonlight ownership overlaps. Source RPM archive/spec bytes
match the frozen build inputs. Actual container dependency/install/verify/remove
passes; no Fedora 44 host installation, GPU or compositor acceptance is inferred.

Evidence and artifacts: /home/barmy/.cache/perigee-build/fedora44-rpm-20261110.
Main RPM: work/rpmbuild/RPMS/x86_64/perigee-6.2.0-0.1.fc44.x86_64.rpm.
SHA256: 32f418787b5b663d370fb984b8d3d1bf2ab15c2bb9e74767f20327408b3f1e3a.
verification.json retains all four RPM hashes; input/run-build.sh retains commands.
The build uses the recorded 619-file source snapshot before the subsequent CI/
inventory-tool updates. App code/dependency pins are unchanged. Full commands,
resource details and limitations are in Task 7A/7B/7D of the implementation plan.


## Clean approved-icon Fedora 44 candidate, 2026-11-11

A fresh 0.4 build uses committed b684374f source (634 files), the prior successful
Fedora 44 container's dependency snapshot and no network/package updates. One
make/LTO job runs under a verified common controller/container limit: 896 MiB RAM,
128 MiB swap, one CPU. Highest combined sample is 800784384 bytes (about 764 MiB),
zero OOM/kill events. Build/lifecycle reports are under
/home/barmy/.cache/perigee-build/fedora44-rpm-orbit-20261111.

Main RPM: retry/work/rpmbuild/RPMS/x86_64/perigee-6.2.0-0.4.fc44.x86_64.rpm.
SHA256: 3ed8ea1c96e5528d8796c28652e837627b7be6b998af6e26c3375b2755e2579a.
Source RPM SHA256: bc9d30dbd1e15787e60623d1f6a13aa35bb59b819e21d05922d9a899af71d9cf.
Debuginfo/debugsource packages are retained alongside the main artifact.
Exact payload, approved SVG/notices, source/spec equality, 75 requirements,
no scripts/conflicts/obsoletes/Moonlight path overlap and disposable native Fedora
44 dependency transaction/install/verify/remove pass. Real compositor/GPU/host
behavior is not inferred.

Perigee-Fedora44-native-check.tar in that root is a portable, checksummed target
kit: main/source RPMs, memory guard and run-fedora44-rpm-check.sh. Copy/extract it
on the user's Fedora 44 GNOME Wayland machine and run that script as the desktop
user. It requests sudo for the verified unsigned development package transactions,
retains dependency checks, caps launch, then removes the app and checks preservation
around install/removal. No credentials are copied and no pairing is requested.
It refuses another installed native Perigee version. Missing dependencies are a
reported failure, never grounds to use --nodeps/--force. Target results remain
pending; the container does not replace this check. No release is published.

The user now explicitly accepts this exact Fedora 44 0.4 candidate's install,
startup/icon, video/audio/input, reconnect and removal on real GNOME Wayland.
This is target user-reported acceptance; exact codec/GPU paths and performance
are separate unreported details. Target evidence files were not directly inspected.
