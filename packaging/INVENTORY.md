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
