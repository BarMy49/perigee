# Perigee translations

Perigee uses Moonlight Qt's existing 30 `qml_*.ts` catalogs, matching `.qm`
resources and language selector. Reuse existing translations and contexts when
wording matches; brand-only changes can reuse the upstream wording with the
product name updated. Preserve upstream translations and attribution.

Every added or changed user-facing string must use `qsTr()`, `tr()` or an explicit
`QCoreApplication::translate()` context and be translated in all supported
catalogs in the same task. This includes CLI help/errors, early startup errors
and in-stream notices. Leave protocol values, settings keys, executable names,
command switches and diagnostic log messages unchanged. Preserve `%1`/`%2`
placeholders, markup, shortcut names and command examples.

Qt Linguist tools are development tools (`qt6-linguist` on Fedora). Extract/update:

```bash
/usr/lib64/qt6/bin/lupdate app -ts app/languages/qml_*.ts
```

Translate the new/changed entries, reusing existing Moonlight terminology, then
compile all catalogs and check coverage before rebuilding the application:

```bash
for catalog in app/languages/qml_*.ts; do
    /usr/lib64/qt6/bin/lrelease "$catalog" -qm "${catalog%.ts}.qm"
done
python3 scripts/check-translations.py
```

The checker needs Python 3.12+, Git's pinned Moonlight baseline object and
`lupdate` (or `--lupdate PATH`). It extracts current and frozen baseline source
keys into temporary directories and rejects missing, unfinished or empty
post-baseline translations and placeholder mismatches across the inherited locale
set. It cannot infer that an unwrapped English literal is user-facing; source
review must still enforce translation wrappers. Commit TS and QM changes together.

Task 5B adds the `AppView` Favorite label in all 30 locales, bringing the current
post-baseline source-key count to 122. Its new translations carry the same
machine-assisted provenance note as the catch-up entries below.
Task 5D adds Appearance, Dark and Light in the SettingsView context, bringing the
current count to 125; all 30 catalogs and compiled resources include these labels.

Perigee's initial catch-up contains 121 new/renamed keys per locale. Exact or
brand-only matches reuse upstream translations. Other entries were generated with
machine assistance and selectively reviewed by the coding assistant, especially
Polish terminology and display headings. Translator comments identify provenance;
these are not claims of native-speaker review across all languages. Corrections
should retain the source context and placeholders.

The normal application selects catalogs through the inherited language setting.
Headless import and lock-refusal output use the system locale without opening a
GUI or loading a destination's credentials. The import command never changes its
command-line switch spellings. Timed stream notices use Qt's font fallback and
text shaping in bounded CPU surfaces prepared before streaming, only when
clipboard integration is enabled. The existing SDL overlay slot receives an
independent cached-surface copy on a state change; text shaping and cold font
loads stay out of the active clipboard path. No decoder or pacing change is needed.

See [compiled-catalog tests](../../tests/translations/README.md) and
[notification tests](../../tests/overlay-notifications/README.md).

Tasks 5G/5H add shortcut editing and shared `StreamControls` action labels in all
30 TS/QM catalogs, bringing coverage to 146 post-baseline keys. The manual-fallback
notice now points to the fixed Ctrl+Alt+Shift+O panel instead of editable J/K keys.
These translations use the same machine-assisted provenance and are not claims
of native-speaker review. Cached tools at
`../perigee-build/translation-tools/usr/lib64/qt6/bin/{lupdate,lrelease}` were used
on this checkout without installing packages. Qt Linguist does not recognize ckb
as an update target; its missing entries were populated explicitly and its QM was
compiled and checked with the other locales. No source catalog was omitted.


## Milestone 6 verification (2026-11-10)

Milestone 6 adds profile, SFTP and UUID-launch labels and errors in all 30 TS/QM
catalogs. Coverage is now 163 post-baseline keys, zero errors; compiled catalog
checks pass 32 cases. New translations are machine-assisted and retain locale
review as a quality follow-up. Qt 6.11.2 Linguist tools were extracted locally to
`/tmp/perigee-translation-tools/usr/lib64/qt6/bin` without system installation;
ckb messages were supplied explicitly because lupdate does not recognize that
translation target. Regenerate QMs with lrelease after TS edits.
