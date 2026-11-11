# Package inventory checks

Run the inventory against an installed payload or AppDir, using a new output path:

```bash
python3 scripts/package-inventory.py --tree /absolute/package-tree --output /absolute/inventory.json
```

The tool hashes regular files, records symlink targets without following them,
and identifies retained notice/source metadata. For every ELF file it reads
DT_NEEDED, SONAME and GNU build IDs with readelf; packaged executables are never
run. The report retains the application's direct dependency list for comparison
with earlier reports. ELF metadata is read in the C locale.

`bundled_elf_providers` maps recorded basenames/SONAMEs to files in the tree.
`elf_requirements_outside_tree` lists names without such a recorded provider.
These are inventory clues, not loader-resolution or ABI checks. Native packages
use host libraries and Flatpak uses a separate runtime; an external requirement
is not automatically a missing library. Runtime closure, plugins loaded through
dlopen, search paths, optional drivers and hardware acceptance need separate checks.

The report does not infer a library's license from its name or declare release
clearance. Review exact source versions/build flags, retained copyright/license
texts, static dependencies and corresponding source/build material. AppImage
bundled libraries/runtime need their own provenance and notices. Flatpak module
notices do not substitute for runtime provenance. Use the native RPM, AppImage
and Flatpak READMEs and ASSET_NOTICES.md for current evidence and open gates.

Fedora CI now generates native-stage and final-RPM-payload inventories. It retains
JSON reports, not binary packages or profiles. The RPM lane uses a complete
committed checkout, initialized submodules, one make/LTO job, a self-contained
source archive, normal rpmbuild policy checks and the exact final payload check.
Remote execution remains pending until the workflow is committed and run with
authorization. Local container results are recorded separately.

Fixtures cover a real ELF/build ID, a symlink pointing outside the tree, notice
collection, non-directory rejection and no-overwrite behavior. Source archive
fixtures verify deterministic output, explicit untracked workflow inclusion,
and continued IDE/Git exclusion. Only explicit YAML files directly under
.github/workflows are allowed as hidden untracked source additions; arbitrary
hidden files, nested hidden directories and IDE data remain rejected.

## Installed RPM provenance

On the Fedora build machine, match inventoried GNU build IDs to the installed RPM
database and retain RPM-declared public license files in a new review directory:

```bash
python3 scripts/package-rpm-provenance.py --inventory /absolute/inventory.json \
  --output /absolute/provenance.json --notices /absolute/new-notice-directory
```

The tool reads readelf/RPM metadata and hashes; it does not execute bundled code
or retrieve sources. Build-ID correspondence is weaker than byte identity:
linuxdeploy can modify RPATH while retaining a build ID. The report records byte
equality separately, raw RPM license fields, exact source-RPM names, retained
notice hashes and source packages without retained notices. Symlinked or nonpublic
notice paths are not copied. All matching source/build material and additional
static-dependency notices still need review before distribution.

The local AppImage audit matches 315 ELF files to 155 installed RPMs representing
145 source RPMs. The two unmatched ELFs are the repository-built client and
libva-probe. Only seven matched files are byte-identical; this is build-ID provenance,
not a binary reproducibility result. It retains 179 RPM-declared license files;
14 source RPMs have no retained license text and need follow-up. Audit output is
/home/barmy/.cache/perigee-build/appimage-rpm-provenance-20261110-v2.json and its
notice directory is appimage-rpm-notices-20261110-v3 beside it. The audit ran in a
64 MiB/no-swap scope; no accepted artifact was changed. Source acquisition and
complete license clearance are still open, particularly for the broad Fedora 45
host-library development AppImage. The inherited controlled dependency build
remains the intended AppImage release path.

The inherited Build workflow formerly ran Windows/macOS/Steam Link packaging on
every push despite their unverified identity. It is now manual, with legacy targets
off by default. Push/PR checks use the Perigee Fedora debug/release/RPM workflow;
manual packaging can still exercise the independent AppImage workflow. GitHub
execution needs an authorized commit/push; no remote action has occurred.
The Fedora checkout retains full history so translation checks can archive the
recorded public Moonlight baseline, which a default shallow checkout could omit.
The proposed CI review snapshot includes required implementation prerequisites
and excludes IDE/runtime state plus the deferred Common C patch/local checker.


## Legacy and split-package notice follow-up

The helper now retains RPM-owned public COPYING/LICENSE/NOTICE/COPYRIGHT candidates
and AUTHORS/README attribution documents when older packages lack %license flags.
Every retained file records its basis; attribution-only documents do not satisfy
license-candidate coverage. --notice-package may name an installed split/common
package only when its exact source RPM matches an inventoried library source.
Unrelated providers fail closed. Real libXau/GCC fixtures verify legacy discovery,
source association, exact bytes, no-overwrite and unrelated-provider rejection.

The expanded local report is appimage-rpm-provenance-split-notices-20261110.json
under the private build-cache root. It retains 328 files from 155 ELF-owner RPMs
and three notice-provider RPMs. Source-RPM inspection retains 13 more candidates
for Firebird, numactl and xcb-util-keysyms under appimage-source-notices-20261110.
All 145 source groups now have retained license candidates across these records;
that does not establish full static-dependency coverage or complete license review.
Only three full source RPMs were acquired; remaining source acquisition/publication
is open. Their raw Koji copies are unsigned, with header/payload digest inspection
recorded separately. No accepted AppImage was modified by this audit.

## Exact source RPM retention

The refreshed AppImage audit has 132 source groups. Generate an exact plan from
that audit on the same machine, where the recorded owner RPMs are still installed:

```bash
python3 scripts/retain-rpm-sources.py --provenance /absolute/rpm-provenance.json \
  --directory /absolute/new-source-cache
```

The default only writes/verifies the plan. Add `--fetch` to retrieve sources
serially. Use the memory wrapper and a fresh report filename for every invocation:

```bash
python3 scripts/limit-build-memory.py --report /absolute/source-fetch-memory.json -- \
  taskset -c 0 python3 scripts/retain-rpm-sources.py \
    --provenance /absolute/rpm-provenance.json --directory /absolute/source-cache --fetch
```

Installed NEVRA/source/vendor must exactly match the input inventory. Supported
origins are Fedora's versioned Koji package archive and RPM Fusion's free Fedora
release/update source repositories. Unknown vendors are rejected. The helper
checks source-package identity and SHA256/SHA512 header/payload digests, retains
file SHA256/size/URL/effective URL and independent publisher-signature results,
and never executes source scripts or changes the installed RPM database. RPM
source headers may report a build architecture; SOURCEPACKAGE=1 is checked rather
than assuming that the ARCH field equals src.

A matching cache can resume: existing sources are rehashed and digest/identity
checked against their receipts. Different plans, incomplete cache entries,
symlinked roots/subdirectories/receipts/sources and source hashes inconsistent
with their receipts are rejected.
Files and receipts are never overwritten. Each acquisition attempt writes a new
summary beneath attempts/. A failed download remains pending while other sources
continue; any pending item produces a nonzero exit. HTTPS redirects remain HTTPS.

The current source cache is
/home/barmy/.cache/perigee-build/appimage-sources-20261111. Its plan and per-source
receipts cover the refreshed artifact's RPM-derived library sources. This does
not cover the AppImage runtime, Perigee/static dependencies, Flatpak modules/runtime,
additional source inputs referenced by a spec, the full build environment or
complete license review. Source RPM presence and embedded digest checks do not
prove that an inventoried binary was reproduced from it. Unsigned Koji sources
remain explicitly unsigned; no key import or package-verification policy override
is performed.

Real numactl source fixtures verify valid unsigned digests, wrong identity,
truncated payload, cache revalidation and symlink rejection. Additional checks
cover unsafe names/vendors, idempotent/no-overwrite plans, changed plans and stale
installed-source metadata. Download outcomes and full-source availability are
recorded separately from these fixtures. Official background:
[Fedora source RPM retrieval](https://fedoraproject.org/wiki/Building_a_custom_kernel/Source_RPM),
[Koji build terminology](https://koji.readthedocs.io/en/latest/using_the_koji_build_system.html).


The first serial acquisition retains 131 sources and reports one libyuv TLS
failure. Resume revalidates all 131 receipts, fetches libyuv, and exits zero:
132 source RPMs, 1701024835 bytes. All source identities/header/payload digests
pass; zero publisher signatures are verified. No signature verification policy
is relaxed. Combined sample peaks are 671088640 bytes and 671076352 bytes
(about 640 MiB), zero swap; kernel limits/OOM counters during acquisition are
retained separately. Reports, receipts, fixture-results.json and completion.json
are under appimage-sources-20261111 in the private build cache.

The embedded AppImage launcher is separately traced to its pinned release asset,
source revision, matching upstream x86_64 build log and six static dependencies.
See [runtime source/notices](appimage/RUNTIME_NOTICES.md). This closes the previously
unrecorded runtime/mimalloc notice material, without claiming complete release
license clearance. The 299 RPM-derived notices remain external review material;
full distribution/source hosting and final notice review remain release gates.
