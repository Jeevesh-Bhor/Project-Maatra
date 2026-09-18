# Building and packaging Maatra

This is for developers and packagers. End users should follow
[INSTALL.md](../INSTALL.md).

## Layout

```
packaging/rpm/*.spec     Fedora, RHEL and openSUSE — one spec with %if blocks per distro
packaging/arch/PKGBUILD  Arch Linux (AUR-ready; fetches the GitHub tag tarball)
debian/                  Debian and Ubuntu (dh 13, native source format)
packaging/build-all.sh   builds every format in a clean container of its target distro
```

## Build all packages

Needs `podman` (or `docker` with `PODMAN=docker`). Each target pulls its
distro image, installs build dependencies, builds, runs the unit tests, and
drops the package in `dist/<target>/`:

```bash
packaging/build-all.sh                # fedora deb ubuntu arch suse
packaging/build-all.sh deb arch       # a subset
```

The tarball is taken from the working tree, so uncommitted changes are
included. For a release, tag first (`git tag v1.0.1`) — the spec and PKGBUILD
download `https://github.com/Jeevesh-Bhor/Project-Maatra/archive/refs/tags/v<version>.tar.gz`,
so once the tag is pushed, `rpmbuild` and `makepkg` work standalone on any
machine. Attach the files from `dist/*/` to the GitHub Release.

RHEL is not built by the script (the `fcitx5` packages come from EPEL); run
the Fedora recipe against an `almalinux:9` image with EPEL enabled, or
`rpmbuild -bb packaging/rpm/fcitx5-marathi-transliterate.spec` on the target.

## Build one package natively

```bash
# Fedora / RHEL / openSUSE
rpmdev-setuptree
cp packaging/rpm/*.spec ~/rpmbuild/SPECS/
tar --transform 's,^,Project-Maatra-1.0.1/,' --exclude=build --exclude=dist --exclude=.git \
    -czf ~/rpmbuild/SOURCES/Project-Maatra-1.0.1.tar.gz .
rpmbuild -bb ~/rpmbuild/SPECS/fcitx5-marathi-transliterate.spec

# Debian / Ubuntu
sudo apt install build-essential debhelper cmake libfcitx5core-dev \
                 libfcitx5config-dev libfcitx5utils-dev libgtest-dev
dpkg-buildpackage -us -uc -b          # .deb lands in the parent directory

# Arch
cd packaging/arch && makepkg -si
```

## Develop and install from source

Build dependencies (Fedora names; Debian/Arch equivalents are above):

```bash
sudo dnf install fcitx5 fcitx5-devel fcitx5-configtool fcitx5-gtk fcitx5-qt \
                 fcitx5-autostart gtest-devel cmake gcc-c++
```

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
ctest --test-dir build                # unit tests (~1 min; loads the 71 MB bigram file)
sudo cmake --install build
fcitx5-remote -r                      # restart Fcitx5 so it sees the addon
```

The addon installs to the directory Fcitx5 exports (`FCITX_INSTALL_ADDONDIR`),
so the same CMake works on `lib64`, multiarch and plain `lib` layouts. Never
set `FCITX_ADDON_DIRS` to point at the build tree — it *replaces* Fcitx5's
search path and breaks every other addon.

To go back to a packaged install afterwards, remove the hand-installed files
first so package ownership stays clean:

```bash
sudo xargs rm -f < build/install_manifest.txt
```

The engine runs without a GUI, which is the quickest way to check a
transliteration change:

```bash
printf 'mala kalat nahi\nmaharashtra\n' | ./build/engine/mr-translit-cli
```

## Package differences per distro

The RPM spec has three dependency branches because the same things are named
differently: Fedora `fcitx5-gtk`/`fcitx5-qt`/`fcitx5-autostart`/
`google-noto-sans-devanagari-vf-fonts`; RHEL the same minus autostart and
with `…-devanagari-fonts`; openSUSE `fcitx5-gtk3`/`fcitx5-gtk4`/`fcitx5-qt6`/
`noto-sans-devanagari-fonts` and `gtest` instead of `gtest-devel`.

Debian and Ubuntu share one `debian/` with no conditionals; the only
difference between the two `.deb` files is the library version floors
`dpkg-shlibdeps` computes at build time (Fcitx5 5.0.21 / glibc 2.36 on
Debian 12, Fcitx5 5.1.7 / glibc 2.38 on Ubuntu 24.04). The Debian-built
`.deb` installs on Ubuntu; the reverse does not.

## Regenerating the dictionary data

`engine/src/mr_words.txt` (dictionary + unigram counts) and
`engine/src/mr_bigrams.txt` (word-pair counts) are generated from the
`aspell-mr` and `hunspell-mr` word lists and the Marathi split of
[IITB-IndicMonoDoc](https://huggingface.co/datasets/cfilt/IITB-IndicMonoDoc)
(~19 GB). They are committed, so a normal build never needs the corpus. See
[LICENSES.md](../LICENSES.md) for sources and terms.
