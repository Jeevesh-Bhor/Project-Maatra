# Project Maatra

**Phonetic Marathi typing for Linux.** Type Marathi the way it sounds on an
ordinary QWERTY keyboard and get Devanagari — the same experience as Google
Indic Keyboard on a phone, built as a native [Fcitx5](https://fcitx-im.org/)
input method for the desktop.

```
mala kalat nahi      →   मला कळत नाही
maharashtra          →   महाराष्ट्र
hemant               →   हेमंत
2026                 →   २०२६
```

A *maatra* (मात्रा) is the vowel sign that changes how a consonant is read.
Working out which one you meant — and which retroflex, which anusvara, which
schwa — from plain Latin letters is most of what this engine does.

Package name: `fcitx5-marathi-transliterate`. Fully offline; nothing you type
ever leaves your machine.

---

## Features

- **Natural spelling.** No special scheme to learn. `pani` → पाणी, `kuthe` →
  कुठे, `shala` → शाळा; retroflexes, long vowels and anusvaras are inferred.
- **Ranked candidate popup.** When the spelling is ambiguous, every plausible
  reading is offered, best guess first. Pick with a number key or the arrows.
- **Sentence context.** A word-pair language model built from ~19 GB of Marathi
  text decides between look-alikes: `kalat` is काळात on its own but कळत after
  मला.
- **Learns from you.** Pick a non-default candidate once and it moves to the
  top next time. Learned data lives only on your disk.
- **Devanagari digits.** `1 2 3` → १ २ ३ while Marathi is active.
- **Months, days, capitals.** `january` → जानेवारी, `Mumbai` → मुंबई.
- **One shortcut.** Ctrl+Space (or Super+Space) flips between Marathi and
  your normal keyboard in any app — GTK, Qt, Electron, terminal.
- **Runs everywhere Fcitx5 does.** Native packages for Fedora, RHEL, openSUSE,
  Debian, Ubuntu and Arch families — see [Supported distributions](#supported-distributions).

## Supported distributions

Maatra runs on any Linux distribution that ships Fcitx5 (5.0 or newer), and is
shipped as a native package for every major family. Each package is built and
unit-tested in a clean container of its target distro before release.

| Distribution | Also covers | Package | Install |
|---|---|---|---|
| **Fedora** 43+ | Nobara, Ultramarine | `.rpm` | `sudo dnf install ./fcitx5-marathi-transliterate-*.fc44.x86_64.rpm` |
| **RHEL** 9+ | AlmaLinux, Rocky, CentOS Stream (EPEL enabled) | `.rpm` | `sudo dnf install ./fcitx5-marathi-transliterate-*.el9.x86_64.rpm` |
| **openSUSE** | Tumbleweed, Leap, Slowroll | `.rpm` | `sudo zypper install ./fcitx5-marathi-transliterate-*.x86_64.rpm` |
| **Debian** 12+ | Devuan, MX Linux, Kali | `.deb` | `sudo apt install ./fcitx5-marathi-transliterate_*_amd64.deb` |
| **Ubuntu** 24.04+ | Mint, Pop!_OS, elementary, Zorin, KDE neon, Kubuntu | `.deb` | `sudo apt install ./fcitx5-marathi-transliterate_*_amd64.deb` |
| **Arch** | Manjaro, EndeavourOS, CachyOS, Garuda | `.pkg.tar.zst` | `sudo pacman -U ./fcitx5-marathi-transliterate-*-x86_64.pkg.tar.zst` |
| Gentoo, NixOS, Void, Alpine, Solus | — | build from source | see below |

| Works with | |
|---|---|
| **Desktops** | GNOME, KDE Plasma, XFCE, Cinnamon, MATE, Budgie, LXQt, Sway, Hyprland, i3 |
| **Display servers** | Wayland and X11 |
| **App toolkits** | GTK 3/4, Qt 5/6, Electron/Chromium, terminals, Wayland-native apps |
| **Architectures** | x86_64 (prebuilt); aarch64 builds from the same sources |

**Not** available as Flatpak, Snap or AppImage: those are sandboxed *app*
formats, and an input method is a plugin that must load inside the system's
Fcitx5 process. A native package is the only way to install one.

### Why there is one package per distro, not one per format

The *file format* is shared within a family — Debian and Ubuntu both use the
same `.deb`; Fedora, RHEL and openSUSE all use the same `.rpm`. What makes a
package "for Ubuntu" or "for openSUSE" is what was baked in when it was built:

- **Library versions it was compiled against.** The Debian 12 `.deb` is built
  on Fcitx5 5.0.21 / glibc 2.36 and declares those as minimums, so it also
  installs on newer Ubuntu; the Ubuntu 24.04 `.deb` is built on Fcitx5 5.1.7 /
  glibc 2.38 and will *not* install on Debian 12. Newer can run older, not the
  reverse.
- **What the dependencies are called.** Fedora's GTK/Qt bridges are
  `fcitx5-gtk` and `fcitx5-qt`; openSUSE splits them into `fcitx5-gtk3`,
  `fcitx5-gtk4`, `fcitx5-qt6`. The Devanagari font is
  `google-noto-sans-devanagari-vf-fonts` on Fedora, `…-fonts` on RHEL and
  `noto-sans-devanagari-fonts` on openSUSE. Fedora needs a separate
  `fcitx5-autostart`; the others bundle autostart into `fcitx5`. A Fedora RPM
  on openSUSE fails with "nothing provides fcitx5-gtk" — not because the
  binary wouldn't run, but because that name doesn't exist there.

So Debian/Ubuntu differ only in version floors (one `debian/` directory, no
conditionals), while Fedora/RHEL/openSUSE differ in dependency names (one
`.spec` with `%if` blocks per distro). Pick the package built for your distro
and the package manager resolves everything correctly.

## Install

Grab the package for your distro from the table above and follow the four
short steps in [INSTALL.md](INSTALL.md): install → log out and back in → add
the input method → type. Debian/Ubuntu users run one extra command
(`im-config -n fcitx5`) so the session starts Fcitx5.

**Building from source** (Fedora shown; Debian/Arch dependency names are in
INSTALL.md §C):

```bash
sudo dnf install fcitx5 fcitx5-devel fcitx5-configtool fcitx5-gtk fcitx5-qt \
                 fcitx5-autostart gtest-devel cmake gcc-c++

cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
ctest --test-dir build        # optional: unit tests
sudo cmake --install build
```

Restart Fcitx5 (`fcitx5-remote -r`) and add **Marathi (Transliteration)** as
an input method in `fcitx5-configtool`.

## Typing guide

Just type. If the first result isn't what you meant, the popup has the rest.

| You type      | You get      | What was inferred                          |
|---------------|--------------|--------------------------------------------|
| `mala`        | मला          | short vs long `a`                          |
| `nahi`        | नाही         | long vowels                                |
| `pani`        | पाणी         | retroflex ण                                |
| `kuthe`       | कुठे         | retroflex ठ                                |
| `maharashtra` | महाराष्ट्र   | retroflex ष + conjunct                     |
| `hemant`      | हेमंत        | anusvara (हेमन्त offered as alternative)   |
| `mumbai`      | मुंबई        | anusvara + split diphthong                 |
| `karto`       | करतो         | schwa deletion                             |
| `neelesh`     | नीलेश        | `ee` / `oo` are long ई / ऊ                 |

**Forcing a retroflex.** If the engine guesses dental where you wanted
retroflex, double the letter:

| Type | Get | Type | Get |
|------|-----|------|-----|
| `t`  | त   | `tt`  | ट  |
| `th` | थ   | `tth` | ठ  |
| `d`  | द   | `dd`  | ड  |
| `dh` | ध   | `ddh` | ढ  |
| `n`  | न   | `nn`  | ण  |
| `l`  | ल   | `ll`  | ळ  |
| `s`  | स   | `ss`  | ष  |

Doubling any *other* consonant produces a real geminate (`kka` → क्क).

**Other useful spellings:** `sh` → श, `ch` → च, `chh` → छ, `f` → फ़, `z` → ज़,
`x` → क्स, `ai` → ऐ/ै, `au` → औ/ौ.

## Keys

| Key                          | Action                                                     |
|------------------------------|------------------------------------------------------------|
| `Ctrl+Space` / `Super+Space` | Switch Marathi ↔ normal keyboard (Super+Space: see INSTALL.md) |
| `1` – `9`                    | Pick that candidate                                        |
| `↑` / `↓`                    | Move the highlight                                         |
| `Space`                      | Commit the highlighted candidate and a space               |
| `Enter`                      | Commit the highlighted candidate, no space                 |
| `Esc`                        | Commit the raw Latin as typed (English words, URLs, names) |
| `Backspace`                  | Edit the Latin before committing                           |
| digits (no word in progress) | Devanagari digits ०–९                                      |

## How it works

Three layers, each only kicking in when the one before it is unsure:

1. **Rule engine** — deterministic phonetic composition of Latin into
   Devanagari: consonant and vowel tables, halant insertion, schwa handling.
   Always produces *something*, even for gibberish.
2. **Dictionary + language model** — a 100k-word Marathi dictionary with corpus
   frequencies and a bigram model (1.96M word pairs). The engine enumerates the
   readings your spelling could mean, keeps the ones that are real words, and
   ranks them: **context first** (which reading follows the previous word most
   often), then overall frequency, then simplicity.
3. **Learning** — your explicit picks are appended to a small per-user log and
   re-ranked to the top on later occurrences of the same spelling.

The data behind layer 2 comes from openly licensed sources — see
[LICENSES.md](LICENSES.md).

## Learning data

Stored at `~/.local/share/fcitx5/marathitranslit/learned.tsv`. Used only
on-device. To start over:

```bash
rm ~/.local/share/fcitx5/marathitranslit/learned.tsv
```

## Appearance

The popup is drawn by Fcitx5's classic UI. The package ships a roomy dark
theme, **Maatra Dark** (`maatra-dark`), with full-width highlighting and a
larger Devanagari font; enable it via `fcitx5-configtool` → Addons → Classic
User Interface, or see INSTALL.md for the config-file version.

## Known limitations

- Standalone postpositions absent from the dictionary (e.g. ला) fall back to
  the plain phonetic reading; pick the intended form from the popup and it
  will be learned.
- Vocalic ऋ (as in कृष्ण) is not yet supported.
- On Wayland the popup is positioned by the compositor and cannot be dragged.

## Project layout

```
engine/       core transliterator (C++20, no Fcitx5 dependency) + CLI + tests
fcitx5/       the Fcitx5 addon, input-method manifests, and the popup theme
packaging/    rpm/ spec (Fedora, RHEL, openSUSE), arch/ PKGBUILD, build-all.sh
debian/       Debian/Ubuntu packaging
```

The engine can be exercised without a GUI:

```bash
echo "mala kalat nahi" | ./build/engine/mr-translit-cli
```
