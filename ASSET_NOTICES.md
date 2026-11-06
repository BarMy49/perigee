# Perigee asset provenance

Perigee is based on Moonlight Qt. Existing upstream and dependency notices remain
in the source tree. This file records the temporary application icon used during
the identity migration; it does not replace a complete release license inventory.

`app/res/io.github.BarMy49.Perigee.svg` is a byte-for-byte copy of the upstream
Moonlight Qt `app/res/moonlight.svg` at source revision
`ac296864ace96f51ad75f78b90644bdbca76c332`. The icon was last changed by Jorys Paulin
in commit `3316fdb80e0c4ebb19f947a6981086c6a62eef29` (2018-12-26),
“Updated Moonlight icon”. It is retained as upstream artwork, not claimed as
original Perigee artwork or an endorsement by Moonlight.

The reviewed source includes the root GPL version 3 license in `LICENSE` and
declares `GPL-3.0+` in its AppStream metadata. The SVG has no separate license or
copyright exception, and no separate icon license was found in the inspected
source tree/history. It is reused under the upstream project's GPL terms, with
the license, attribution and corresponding source retained. The distinct installed
basename prevents replacing Moonlight's icon. Original Perigee artwork remains a
separate task before final distribution branding.

Upstream source: <https://github.com/moonlight-stream/moonlight-qt/tree/ac296864ace96f51ad75f78b90644bdbca76c332>.
Only the icon's installed/resource name changes; its SVG content is unchanged.

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
