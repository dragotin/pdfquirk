#! /bin/bash

set -e
set -o pipefail

if [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... bash $0"
    exit 2
fi

set -x

# use RAM disk if possible
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" pdfquirk-build-XXXXXX)"

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT="$(readlink -f "$(dirname "$(dirname "$0")")")"
OLD_CWD="$(readlink -f .)"

pushd "$BUILD_DIR"

# configure build for AppImage release
cmake "$REPO_ROOT" -G Ninja -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo

ninja -v -j"$(nproc)"

# copy files into AppDir using build system
env DESTDIR=AppDir ninja install

# get linuxdeploy and its qt plugin
wget https://github.com/TheAssassin/linuxdeploy/releases/download/continuous/linuxdeploy-"$ARCH".AppImage
wget https://github.com/TheAssassin/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-"$ARCH".AppImage
wget -c https://raw.githubusercontent.com/TheAssassin/linuxdeploy-plugin-conda/master/linuxdeploy-plugin-conda.sh
chmod +x linuxdeploy*.{AppImage,sh}

# if deskew is installed, we deploy it using linuxdeploy's -e
if deskew_path="$(which deskew 2>/dev/null)"; then
    extra_ld_args=(-e "$deskew_path")
else
    echo "Warning: deskew not found in \$PATH, cannot include"
fi

# using git to get a usable version number
pushd "$REPO_ROOT"
git fetch -a
VERSION="$(git describe --tags)"
export VERSION
popd

# build AppImage
export UPD_INFO="gh-releases-zsync|dragotin|pdfquirk|continuous|PDFQuirk\*-$ARCH.AppImage.zsync"

# we use linuxdeploy-plugin-conda to install a Python environment in <AppDir>/usr/conda and install
# img2pdf from PyPI to it
# to call it calculate the path to the main binary, then add ../conda/bin/python -m img2pdf
export PIP_REQUIREMENTS="img2pdf"
# Pillow needs to be installed using a conda package because those are relocatable
export CONDA_PACKAGES="Pillow"

./linuxdeploy-"$ARCH".AppImage --appdir AppDir --plugin conda --plugin qt --output appimage "${extra_ld_args[@]}"

mv PDFQuirk*.AppImage* "$OLD_CWD"/
