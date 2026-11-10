# Compiled translation checks

This Qt Core/Test target loads every inherited QM catalog and compares each
finished non-plural TS entry with `QTranslator::translate()`, including all 121
Perigee keys and inherited entries. This detects stale QM files after TS edits.
It contacts no host, opens no window and reads no credentials. Build after
recompiling the catalogs as described in [the translation guide](../../app/languages/README.md):

```bash
mkdir -p ../perigee-build/tests/translations
cd ../perigee-build/tests/translations
qmake6 ../../../perigee/tests/translations/translations.pro
make -j2
./tst_translations -o results.txt,txt
```

Source-key coverage and placeholder validation run separately from the source root:

```bash
python3 scripts/check-translations.py
```

`--lupdate PATH` selects an extracted/nonstandard Qt tool. `--catalog-dir PATH`
permits checking a disposable catalog copy, including failure-path checks.
These checks verify completeness, lookup contexts and compilation, not native
linguistic accuracy. The importer target also tests Polish CLI output without a
GUI platform; the notification target covers Unicode CPU surfaces and timing.
