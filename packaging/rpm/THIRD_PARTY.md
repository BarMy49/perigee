# Native Linux development package notices

Perigee derives from Moonlight Qt at
ac296864ace96f51ad75f78b90644bdbca76c332 under GPL-3.0-or-later.
See LICENSE, ASSET_NOTICES.md and SOURCE_SNAPSHOT.json in this package.
The source snapshot records actual file hashes, root revision and dependency pins;
it includes the build scripts and complete initialized submodule source.

The native binary statically incorporates:

- Moonlight Common C: GPL-3.0-or-later; LICENSE.txt, source copyright notices.
- ENet inside Common C: MIT; enet/LICENSE.
- nanors inside Common C: MIT, copyright Joseph Calderon; nanors/LICENSE.
- qmdnsengine: MIT, copyright Nathan Osman; LICENSE.txt.
- h264bitstream: LGPL-2.1-or-later; LICENSE and source copyright notices.
- SDL_GameControllerDB data: Zlib, copyright Sam Lantinga; LICENSE.

License texts retain their source-relative names under
/usr/share/licenses/perigee. System Qt, SDL, FFmpeg, OpenSSL, Opus, VA-API,
VDPAU, libplacebo, DRM, Wayland and X11 libraries are dynamically linked and
remain owned by their system packages. Those packages supply their own notices.
No codec patent clearance or compatibility with every FFmpeg build is asserted.

The embedded ModeSeven.ttf matches the author's original Windows font byte for
byte (SHA256 afb1601d2b8d368900f3181d972d44584da964b1f99f6580ec81f9ce1b0906f8).
Andrew C. Bulhak's README states that the font is freely distributable. The complete
README is installed as ModeSeven-README; the custom spec license identifier
LicenseRef-ModeSeven-Font refers to these terms. Original archive:
https://dev.null.org/fonts/mode7win.zip, linked from https://dev.null.org/fonts/.

Distribution remains gated by the complete release asset/source inventory and
live compatibility/package acceptance. This initial inventory does not assert
codec patent clearance or completed Fedora repository review. Do not publish this
development package as a release.
