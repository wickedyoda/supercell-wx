#!/usr/bin/env python3
"""Download and extract Qt for Android packages straight from the Qt OSS repo.

aqtinstall cannot resolve Android architectures for Qt >= 6.8 (metadata layout
change), and the archive names in the 6.7.x tree carry a build-id prefix, so
this script lists each package directory and pulls the .7z archives itself.
"""
import os
import pathlib
import re
import shutil
import subprocess
import sys
import urllib.request
import xml.etree.ElementTree as ET

BASE = "https://download.qt.io/online/qtsdkrepository/linux_x64/android"
VERSION_DIR = os.environ["QT_VERSION_DIR"]   # e.g. qt6_7_3/qt6_673
ABI = os.environ.get("QT_ABI", "arm64_v8a")
DEST = os.environ["QT_DEST"]

COMPONENTS = ["qtbase", "qtsvg", "qtdeclarative", "qttools", "qttranslations",
              "qtmultimedia", "qtpositioning", "qtserialport", "qtimageformats",
              "qtshadertools", "qt5compat"]

repo = f"{BASE}/{VERSION_DIR}_{ABI}"
with urllib.request.urlopen(f"{repo}/Updates.xml") as r:
    root = ET.fromstring(r.read())

pkgs = {p.findtext("Name"): p for p in root.findall("PackageUpdate")}
os.makedirs("/tmp/qt-dl", exist_ok=True)
os.makedirs(DEST, exist_ok=True)

wanted = [n for n in pkgs if n.endswith(f"_{ABI}") or n == f"qt.qt6.673.android"]
if not wanted:
    print("no packages matched", file=sys.stderr)
    sys.exit(2)

downloaded = 0
for name in sorted(wanted):
    listing_url = f"{repo}/{name}/"
    with urllib.request.urlopen(listing_url) as r:
        html = r.read().decode("utf-8", "replace")
    files = sorted(set(re.findall(r'href="([^"]*\.7z)"', html)))
    for f in files:
        comp = re.sub(r"^.*?\d{12}", "", f).split("-")[0]
        if comp not in COMPONENTS:
            continue
        local = f"/tmp/qt-dl/{f}"
        if not os.path.exists(local) or os.path.getsize(local) == 0:
            print(f"downloading {f}", flush=True)
            urllib.request.urlretrieve(listing_url + f, local)
        subprocess.run(["7z", "x", "-y", f"-o{DEST}", local],
                       check=True, stdout=subprocess.DEVNULL)
        downloaded += 1
# The archives contain "<version>/<abi>/..."; flatten so that DEST is the Qt
# installation root (bin/, lib/, include/ ...) as CMake and androiddeployqt
# expect.
for candidate in sorted(pathlib.Path(DEST).iterdir()):
    if candidate.is_dir() and re.fullmatch(r"\d+\.\d+\.\d+", candidate.name):
        inner = candidate / f"android_{ABI}"
        if not inner.is_dir():
            inner = candidate / ABI
        if inner.is_dir():
            for item in inner.iterdir():
                target = pathlib.Path(DEST) / item.name
                if not target.exists():
                    item.rename(target)
            shutil.rmtree(candidate, ignore_errors=True)
        break

config = pathlib.Path(DEST) / "lib" / "cmake" / "Qt6" / "Qt6Config.cmake"
if not config.is_file():
    print(f"ERROR: {config} is missing; Qt extraction did not produce a usable install", file=sys.stderr)
    sys.exit(3)

print(f"DONE archives={downloaded} root={DEST}")
