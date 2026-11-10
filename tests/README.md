# Focused Linux checks

`scripts/check-linux-ci.sh DEBUG_BUILD_DIRECTORY RESULT_DIRECTORY` runs the
standalone capability, service, display, importer, profile, integration-policy,
shortcut and compiled-catalog checks, followed by translation coverage, grid QML,
and the application-linked theme/favorites checks. The result directory must be
new; each suite writes a separate text result. Fixtures use disposable state and
synthetic hosts/identities. No production credentials or live host session is used.

Build the application debug objects first using the existing qmake workflow.
The runner links the actual application objects for theme/favorites; a release-only
build cannot substitute for those debug objects. Qt Quick uses the software test
backend, offscreen Qt and dummy SDL. That verifies UI/policy fixtures, not GPU,
compositor, audio/input or streaming behavior. Individual suite READMEs describe
what each fixture can establish.

```bash
PERIGEE_BUILD_JOBS=1 \
  scripts/check-linux-ci.sh /absolute/debug-build /absolute/new-results
```

The default build-job count is one. `PERIGEE_LUPDATE` can select a reviewed local
Qt Linguist executable if it is not at Fedora's standard Qt 6 location. SDL/Qt and
application dependencies are documented in docs/FEDORA_BUILD.md; service TLS tests
also need the openssl command. The app binary path is supplied to importer fixtures.
Do not run a second compilation alongside a memory-constrained Flatpak build.

`.github/workflows/check-linux.yml` builds debug and release independently in
Fedora 44 containers. Debug runs the focused runner; release validates the staged
Perigee-only payload. Only synthetic text results are retained as CI artifacts.
The workflow uses the container's standard Fedora library providers; it does not
establish RPM Fusion codec coverage or hardware acceptance. Remote execution and
a local full-run result must be recorded before calling this workflow verified.


## Executed runner result, 2026-11-10

The complete script passes on the available Fedora 45 machine using existing
debug application objects, PERIGEE_BUILD_JOBS=1, a reviewed local Qt 6 lupdate,
and a 1 GiB RAM-plus-swap scope. All 271 cases in 11 suites pass: capabilities 57,
services 22, display 17, importer 35, profiles 8, integration policies 9, shortcut
policy/modifiers 6, catalogs 32, grids 25, application UI 47 and favorites 13.
Translation coverage reports 163 keys across 30 locales, zero errors. Results are
in /home/barmy/.cache/perigee-build/ci-20261110; its sibling .log records the run.
This validates the local runner, not the unrun Fedora 44 GitHub/container job.

## Clean Fedora 44 result, 2026-11-10

Separate release/debug application builds and the complete runner pass in a
rootless Fedora 44 container using one job. Release staging and all 271 cases in
the same 11 suites pass; translation coverage remains 163 keys/30 locales/zero
errors. Standard Fedora library providers were used. The initial release attempt
found a missing libavformat development dependency; the retry includes the explicit
provider now present in the CI/RPM recipes. No application source was changed.

Evidence root: /home/barmy/.cache/perigee-build/fedora44-20261110. build.log records
the dependency failure; build-retry.log records success. work/results contains
the individual Qt Test reports; work/environment.txt records OS/package versions.
verification-summary.json records the source digest and container image digest
sha256:acb5e2218545b173cb73206435c64cb43d493df508b9d875c2dff1c44c82867f.
The source snapshot is the clean Flatpak snapshot recorded in its README. A minimal
Git object store containing only the public baseline snapshot supplies git archive
for the translation comparison; no production repository metadata was mounted.

Kernel accounting records a RAM peak of 896 MiB, a sampled RAM-plus-swap peak of
939917312 bytes (about 896.4 MiB), and zero OOM kills. See
[resource execution](../packaging/BUILD_RESOURCES.md) for the observed Podman swap
limit mismatch and correction. GitHub-hosted execution, target hardware and codec
coverage remain separate gates; this is not a GPU/compositor/streaming pass.


The CI draft now also has an independent Fedora 44 RPM lane and native/RPM
inventory reports. Normal clean Fedora 44 rpmbuild, exact final payload, generated
requirements, source-RPM input equality and disposable-container install/verify/
remove pass locally; see packaging/rpm/README.md and the plan. These packaging
checks do not rerun or replace the 271 application fixtures recorded above.
The new inventory/provenance tools pass real ELF/build-ID, outside-symlink,
no-overwrite, invalid build-ID, exact retained-notice and non-directory fixtures.
Explicit untracked CI YAML inclusion also passes deterministic source-snapshot
and IDE/Git exclusion fixtures. Remote GitHub execution remains pending.
