# Perigee asset provenance

Perigee is based on Moonlight Qt. Existing upstream and dependency notices remain
in the source tree. This file records application/interface artwork provenance;
it does not replace the complete release license and corresponding-source review.

## Approved Perigee application icon

`app/res/io.github.BarMy49.Perigee.svg` is original geometric artwork approved by
the user on 2026-11-10. An elliptical orbit, a body at one focus and a warm
closest-approach marker represent Perigee. No external artwork, Moonlight geometry
or fonts were copied. It uses the project's GPL-3.0-or-later terms; see LICENSE.
The installed filename/application ID is unchanged. Package/GUI verification is
recorded separately; approval is not a claim that every older artifact contains it.

## Retained upstream Moonlight icon

`app/res/moonlight.svg` remains byte-identical to the Moonlight Qt baseline
`ac296864ace96f51ad75f78b90644bdbca76c332`. It was last changed by Jorys Paulin
in commit `3316fdb80e0c4ebb19f947a6981086c6a62eef29` (2018-12-26),
“Updated Moonlight icon”. Before the approved orbit icon, Perigee used an attributed
temporary copy under its independent resource/installed name. Older development
artifacts retain that artwork. It is not claimed as original Perigee work or an
endorsement by Moonlight.

The reviewed upstream source includes GPL version 3 in LICENSE and declares
GPL-3.0+ in AppStream metadata. The retained SVG has no separate license/copyright
exception in the inspected tree/history. Upstream licensing, attribution and
corresponding-source obligations continue to apply to that retained artwork.

Upstream source: <https://github.com/moonlight-stream/moonlight-qt/tree/ac296864ace96f51ad75f78b90644bdbca76c332>.

## Light-mode interface icon variants

Task 5D adds `-light.svg` variants of the already-vendored resources
`desktop_windows-48px`, `baseline-lock-24px`, `baseline-help_outline-24px`,
`baseline-error_outline-24px`, `baseline-check_circle_outline-24px`,
`baseline-warning-24px` and `warning_FILL1_wght300_GRAD200_opsz24`.
Their originals match Moonlight Qt baseline
`ac296864ace96f51ad75f78b90644bdbca76c332` at the same `app/res/` paths.
Only the foreground fill changes from white to `#212121`; geometry and existing
source resources are retained. These are color adaptations of existing interface
assets, not original Perigee artwork. Existing upstream licensing, attribution
and corresponding-source obligations continue to apply. No external asset or
new asset dependency was introduced.


## Embedded ModeSeven font

`app/ModeSeven.ttf` is byte-identical to `MODES___.TTF` from the author's
[original Windows archive](https://dev.null.org/fonts/mode7win.zip), linked on
[Andrew Bulhak's font page](https://dev.null.org/fonts/), inspected 2026-11-10.
Both SHA256 hashes are
`afb1601d2b8d368900f3181d972d44584da964b1f99f6580ec81f9ce1b0906f8`.
The archive README explicitly permits free distribution; its full original text
is preserved in [ModeSeven-README](packaging/rpm/ModeSeven-README) and installed
with native RPM license material. The font remains upstream artwork and is not
claimed as original Perigee work.


The development AppImage flow also rasterizes the approved Perigee SVG
into a 256×256 PNG at build time for .DirIcon. This is a format conversion of the
approved artwork, with its recorded provenance and project license.
The generated PNG stays in the AppDir and is not a new repository artwork source.
