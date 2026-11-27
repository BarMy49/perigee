# AppImage source distribution review

This record covers the Ubuntu-built AppImage and its matching local source material.
It does not publish a package or certify binary reproducibility. The Fedora-built
AppImage has a different dependency set and must not use this source bundle.

## Artifact and source association

The user-tested Fedora 45 artifact has SHA256
`c2402b88e0a3f596d199b5f8fbf3cde4ebebf089fee417a1fae6b667fcdece1e`.
Its local source review archive is `Perigee-c2402b88-source-review.tar`, SHA256
`7d7527258ccd8ab167a9a1358e23c7ff2ca0c6591eb0cb5cf32a6acbcf258199`.
Both are retained under `appimage-controlled-20261111` in the private build cache.
The source archive contains 376 entries and has passed a streamed hash round trip.
Its manifest and SHA256SUMS identify every selected file. Do not reuse this artifact
association for a newly compiled image until its actual dependency/source inventory
has been compared and any changes have matching sources.

| Material | Location inside source archive | Verification |
| --- | --- | --- |
| Application and pinned submodules | application/ | Committed 82fa8811 source snapshot, build scripts and workflow |
| Custom dependencies, including static/header inputs | custom/ | 16 exact Git origins/revisions, archives and working-tree patches |
| Ubuntu/LunarG library sources | system/ | 43 exact source versions, .dsc descriptors, upstream and Debian packaging archives |
| Embedded runtime and static dependencies | runtime/ | Runtime revision, Alpine recipe snapshot and six library source archives |
| Packaged notices and ownership maps | notices/ | Exact final AppDir license tree and ELF/source manifests |
| Repository verification | repository-evidence/ | APT indexes, public keys and release signature records |
| Payload comparison | evidence/ | Final inventory, comparison and source summary |

All 73 recorded system binary package identities map to the retained 43 exact
source groups. The 16 custom component notice sets are compared directly with
root-relative paths in their source archives. Package copyright files may describe
more source files than the linked binaries use; retained notices are not evidence
that every described license applies to a bundled object. Runtime source mapping
and its original notices are documented in [RUNTIME_NOTICES.md](RUNTIME_NOTICES.md).
The application source also retains original upstream, submodule, font and asset
notices; see [asset provenance](../../ASSET_NOTICES.md).

## Rebuild material and limits

Extract into a new directory, then run `sha256sum --check SHA256SUMS` there before
using source files. The application archive contains the final
`.github/workflows/package-linux-appimage.yml`, `scripts/build-appimage.sh`, pinned packaging
tool digests and the controlled build flags. Use the digest-pinned Ubuntu base,
one compiler/LTO/compression job, and an enforced combined 2 GiB ceiling.

Custom archive members have a `source/` prefix. Restore each component at the path
recorded in custom/manifest.json, removing that prefix; nested submodules must be
restored separately at their recorded paths. The retained working-tree patches
record the existing libplacebo adaptation and generated SDL_ttf build files.
Do not apply the libplacebo patch twice: the normal workflow applies it itself.
The normal workflow fetches exact Git revisions and asserts them before compilation.
Archives omit Git metadata, so they cannot directly satisfy its `git rev-parse`
checks or its Git-based notice collector. For an archive-based rebuild, verify the
archive manifest and use the already verified notices/dependencies bundle through
`PERIGEE_APPIMAGE_NOTICE_BUNDLE`. Such a rebuild requires explicit adaptation of
the checkout/assertion steps; it is not claimed as an executed offline workflow.

System sources retain their distribution packaging and patches. Unpack each group
with its .dsc descriptor using dpkg-source. Source uploader signatures and bitwise
reproduction of distribution binaries have not been verified. The system notice
collector checks the actual staged ELF build IDs against the SDK before packaging.
If package repositories supply different versions on a later build, retain their
matching sources and regenerate the inventory rather than reusing this association.

The runtime recipe and Alpine aports snapshot contain its static library build
material and patches. Upstream recipes use parallel jobs; adapt those job counts
before running under the project resource limit. The existing runtime asset is
pinned and its code bytes were compared; a fresh runtime build is not claimed.

## Distribution handoff

A release must provide the matching application/dependency/runtime source material
and notices alongside the binary through a durable, identified download location.
The earlier archive was local review material. For the authorized release, record
the final binary and source hashes together, verify retrieval from GitHub, and retain the exact
build recipe, dependency versions and inventories. Do not point a new binary at an
older artifact's sources or rely only on mutable upstream download links.

The build recipe, source mapping and notice-byte checks are technical evidence.
They do not replace review of applicable file-level licensing or establish a
reproducible binary. Remaining changes discovered by the clean final build must be
reflected in this record before distribution.


## Clean-build acceptance update

The clean 82fa8811 recipe completed without retries. Its AppImage SHA256 is
4ceee61c1ae51c3901b3927b56fb72853ddd23b5f795e41e41811e1ee2085675.
The user accepts video, audio, keyboard/mouse and reconnect with both Apollo and
Sunshine on Fedora 45. All 47 application-linked cases and isolated local GUI/
portable checks pass. System package versions and custom source/notice records
match the earlier bundle; two ELF hashes differ, so bitwise reproducibility is
not claimed. The matching source archive is Perigee-4ceee61c-source-review.tar
under appimage-clean-final-20261111 in the build cache. Its manifest binds the
new artifact, notices, clean recipe, resource evidence and test results.
The release archive Perigee-6.2.0-appimage-source.tar updates the README for the
clean run, adds user acceptance evidence, and preserves the verified sources.
Its SHA256 is e8ce14fdcb76148de15b96a4e230550f5c4c16bf12c74535aeecda0560d11fe0.
