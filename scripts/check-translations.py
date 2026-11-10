#!/usr/bin/env python3
"""Check post-Moonlight translation keys in every inherited Qt catalog."""

import argparse
import io
import pathlib
import re
import shutil
import subprocess
import tarfile
import tempfile
import xml.etree.ElementTree as ET


BASELINE = "ac296864ace96f51ad75f78b90644bdbca76c332"
ROOT = pathlib.Path(__file__).resolve().parents[1]


def messages(path):
    result = {}
    for context in ET.parse(path).getroot().findall("context"):
        for message in context.findall("message"):
            key = (context.findtext("name"), message.findtext("source"), message.findtext("comment", ""))
            if key in result:
                raise ValueError(f"Duplicate message in {path.name}: {key}")
            result[key] = message
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lupdate", help="Path to the Qt lupdate executable")
    parser.add_argument("--catalog-dir", type=pathlib.Path, default=ROOT / "app/languages")
    args = parser.parse_args()
    tool = args.lupdate or next((shutil.which(name) for name in ("lupdate6", "lupdate-qt6", "lupdate") if shutil.which(name)), None)
    if not tool:
        candidate = pathlib.Path("/usr/lib64/qt6/bin/lupdate")
        tool = str(candidate) if candidate.exists() else None
    if not tool:
        parser.error("Qt Linguist tools are required; install qt6-linguist or pass --lupdate PATH")
    with tempfile.TemporaryDirectory(prefix="perigee-translations-") as directory:
        temporary = pathlib.Path(directory)
        archive = subprocess.check_output(["git", "archive", BASELINE, "app"], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(archive)) as snapshot:
            snapshot.extractall(temporary / "baseline", filter="data")
        for name, source in (("baseline", temporary / "baseline/app"), ("current", ROOT / "app")):
            subprocess.run([tool, str(source), "-no-obsolete", "-ts", str(temporary / (name + ".ts"))],
                           check=True, stdout=subprocess.DEVNULL)
        required = messages(temporary / "current.ts").keys() - messages(temporary / "baseline.ts").keys()
    catalogs = sorted(args.catalog_dir.glob("qml_*.ts"))
    expected = {path.name for path in (ROOT / "app/languages").glob("qml_*.ts")}
    errors = []
    if {path.name for path in catalogs} != expected:
        errors.append("Catalog set differs from the inherited Moonlight locales")
    for path in catalogs:
        catalog = messages(path)
        for key in sorted(required):
            message = catalog.get(key)
            translation = None if message is None else message.find("translation")
            forms = [] if translation is None else translation.findall("numerusform")
            texts = ["".join(form.itertext()) for form in forms] if forms else ["" if translation is None else "".join(translation.itertext())]
            if translation is None or translation.get("type") in ("unfinished", "obsolete", "vanished") or any(not text.strip() for text in texts):
                errors.append(f"{path.name}: untranslated {key[0]} / {key[1]}")
            elif any(sorted(re.findall(r"%(?:L?\d+|n)", key[1])) != sorted(re.findall(r"%(?:L?\d+|n)", text)) for text in texts):
                errors.append(f"{path.name}: placeholder mismatch {key[0]} / {key[1]}")
    for error in errors:
        print(error)
    print(f"{len(required)} post-baseline keys, {len(catalogs)} locales, {len(errors)} errors")
    return bool(errors)


if __name__ == "__main__":
    raise SystemExit(main())
