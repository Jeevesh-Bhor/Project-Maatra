#!/usr/bin/env bash
# Build every distribution package in a clean container of its target distro.
#
#   packaging/build-all.sh            # build all
#   packaging/build-all.sh deb arch   # build a subset: fedora | deb | ubuntu | arch | suse
#
# Needs podman (or docker: PODMAN=docker). Output lands in dist/<target>/.
# The source tarball is taken from the working tree (not git HEAD) so you can
# test uncommitted changes; for a release, tag first and let the .spec/PKGBUILD
# fetch the GitHub tag tarball instead.
set -euo pipefail

PODMAN=${PODMAN:-podman}
ROOT=$(cd "$(dirname "$0")/.." && pwd)
VERSION=$(sed -n 's/^project(.*VERSION \([0-9.]*\).*/\1/p' "$ROOT/CMakeLists.txt")
NAME=fcitx5-marathi-transliterate
TARDIR=Project-Maatra-$VERSION          # matches GitHub's tag-tarball layout
WORK=$ROOT/dist/.work
mkdir -p "$WORK"

echo "== source tarball $TARDIR.tar.gz"
rm -rf "$WORK/$TARDIR"; mkdir -p "$WORK/$TARDIR"
tar -C "$ROOT" --exclude=./build --exclude='./build-*' --exclude=./dist --exclude=./.git \
    -cf - . | tar -C "$WORK/$TARDIR" -xf -
tar -C "$WORK" -czf "$WORK/$TARDIR.tar.gz" "$TARDIR"

run() { # run <target> <image> <script>
    local target=$1 image=$2 script=$3 out="$ROOT/dist/$1"
    mkdir -p "$out"
    echo "== $target ($image)"
    $PODMAN run --rm -e VERSION="$VERSION" -e NAME="$NAME" -e TARDIR="$TARDIR" \
        -v "$WORK:/work:ro,Z" -v "$out:/out:Z" "$image" bash -euo pipefail -c "$script"
    ls -1 "$out"
}

build_fedora() { run fedora registry.fedoraproject.org/fedora:44 '
    dnf -y -q install rpm-build rpmdevtools cmake gcc-c++ fcitx5-devel gtest-devel
    rpmdev-setuptree
    cp /work/$TARDIR.tar.gz ~/rpmbuild/SOURCES/
    cp /work/$TARDIR/packaging/rpm/$NAME.spec ~/rpmbuild/SPECS/
    rpmbuild -bb ~/rpmbuild/SPECS/$NAME.spec 2>&1 | grep -E "tests passed|^Wrote:|error" 
    cp ~/rpmbuild/RPMS/*/$NAME-$VERSION-*.rpm /out/
    rpm -qp --requires /out/$NAME-$VERSION-1.fc44.x86_64.rpm | grep -E "^fcitx5|fonts"
'; }

build_suse() { run suse registry.opensuse.org/opensuse/tumbleweed:latest '
    zypper -q -n install rpm-build cmake gcc-c++ fcitx5-devel gtest tar gzip
    mkdir -p ~/rpmbuild/{SOURCES,SPECS}
    cp /work/$TARDIR.tar.gz ~/rpmbuild/SOURCES/
    cp /work/$TARDIR/packaging/rpm/$NAME.spec ~/rpmbuild/SPECS/
    rpmbuild -bb --define "_topdir $HOME/rpmbuild" ~/rpmbuild/SPECS/$NAME.spec 2>&1 | grep -E "tests passed|^Wrote:|error"
    cp ~/rpmbuild/RPMS/*/$NAME-$VERSION-*.rpm /out/
    rpm -qp --requires /out/$NAME-$VERSION-*.rpm | grep -E "^fcitx5|fonts"
'; }

deb_script='
    export DEBIAN_FRONTEND=noninteractive
    apt-get -qq update
    apt-get -qq install -y --no-install-recommends build-essential debhelper cmake \
        libfcitx5core-dev libfcitx5config-dev libfcitx5utils-dev libgtest-dev >/dev/null
    cp -r /work/$TARDIR /build && cd /build
    dpkg-buildpackage -us -uc -b 2>&1 | grep -E "tests passed|dpkg-deb: building|error" 
    cp ../${NAME}_${VERSION}_*.deb /out/
    dpkg-deb -I /out/${NAME}_${VERSION}_*.deb | grep -E "Depends|Version"
    dpkg-deb -c /out/${NAME}_${VERSION}_*.deb | grep -E "\.so$|addon/|inputmethod/"
'
build_deb()    { run debian docker.io/library/debian:bookworm-slim "$deb_script"; }
build_ubuntu() { run ubuntu docker.io/library/ubuntu:24.04 "$deb_script"; }

build_arch() { run arch docker.io/library/archlinux:base-devel '
    pacman -Syu --noconfirm --needed cmake fcitx5 gtest >/dev/null
    useradd -m builder
    cp -r /work/$TARDIR/packaging/arch /home/builder/pkg
    cp /work/$TARDIR.tar.gz /home/builder/pkg/
    chown -R builder /home/builder/pkg
    cd /home/builder/pkg && su builder -c "makepkg -f --noconfirm --nodeps 2>&1 | grep -E \"tests passed|Finished making|ERROR\""
    cp /home/builder/pkg/$NAME-$VERSION-*.pkg.tar.zst /out/
    tar -tf /out/$NAME-$VERSION-*.pkg.tar.zst | grep -E "\.so$|addon/|inputmethod/"
'; }

targets=("$@"); [ ${#targets[@]} -eq 0 ] && targets=(fedora deb ubuntu arch suse)
for t in "${targets[@]}"; do "build_$t"; done
echo "== done: $(find "$ROOT/dist" -maxdepth 2 -type f \( -name '*.rpm' -o -name '*.deb' -o -name '*.pkg.tar.zst' \) | sed "s|$ROOT/||")"
