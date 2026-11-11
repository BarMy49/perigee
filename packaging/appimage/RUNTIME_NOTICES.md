# Embedded AppImage runtime notices

These files retain upstream notices for the pinned type2 runtime
fedfad51acc2860b2916a1eef32ba4a500ff31ef and its statically linked libraries.
No runtime source, binary or dependency pin is changed. LICENSE is the unmodified
upstream file; SOURCE_NOTICE preserves the complete runtime.c leading copyright/
license comment. The runtime Makefile and build log also link mimalloc, which the
upstream top-level LICENSE does not list; its original MIT license is retained.

| Component | Source/package version | Retained material |
| --- | --- | --- |
| type2-runtime | fedfad51acc2860b2916a1eef32ba4a500ff31ef | [LICENSE](runtime/LICENSE), [SOURCE_NOTICE](runtime/SOURCE_NOTICE) |
| libfuse | 3.15.0 with upstream runtime mount patch | [LICENSE](runtime/libfuse/LICENSE), [LGPL2.txt](runtime/libfuse/LGPL2.txt), [GPL2.txt](runtime/libfuse/GPL2.txt) |
| squashfuse | 0.5.2 | [LICENSE](runtime/squashfuse/LICENSE) |
| musl | Alpine 1.2.5-r11 | [COPYRIGHT](runtime/musl/COPYRIGHT) |
| zlib | Alpine 1.3.2-r1 | [LICENSE](runtime/zlib/LICENSE) |
| zstd | Alpine 1.5.6-r2 | [LICENSE](runtime/zstd/LICENSE), [COPYING](runtime/zstd/COPYING) |
| mimalloc2 | Alpine 2.1.7-r0 | [LICENSE](runtime/mimalloc/LICENSE) |

[sources.json](runtime/sources.json) records source URLs/hashes, selected notice paths,
Alpine recipe snapshot, image digest and the matching
[upstream build run](https://github.com/AppImage/type2-runtime/actions/runs/39320578745).
Versions come from its x86_64 build log. The immediately preceding Alpine aports
snapshot has matching package versions; all 21 referenced archive/local-patch
SHA512 checks pass. The runtime recipe separately pins libfuse/squashfuse archives
by SHA256, and includes its libfuse patch. Full source archives and the complete
aports snapshot are retained outside Git under appimage-runtime-source-20261111
in the private build cache. No upstream source script was executed.

The exact runtime asset SHA256 is
617aa77314d57c8b8e4fae9c3f0b35af7b30d5529cb69e98f64f92510ed98a46.
The packaged prefix differs only in its 16-byte .digest_md5 section. Restoring
those bytes yields this exact asset hash; executable code is unchanged. This is
binary correspondence and source/notice retention, not a reproduced build or
complete legal/build-environment clearance. The original upstream scripts use
parallel builds; do not execute them unchanged under the one-job resource limit.
