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

Maatra runs on any Linux distribution that ships Fcitx5 (5.0 or newer) and is
released as a native package for every major family. Each package is built and
unit-tested in a clean container of its target distribution.

| Distribution | Also covers | Package |
|---|---|---|
| **Fedora** 43+ | Nobara, Ultramarine | `.rpm` |
| **RHEL** 9+ | AlmaLinux, Rocky, CentOS Stream (with EPEL) | `.rpm` |
| **openSUSE** | Tumbleweed, Leap, Slowroll | `.rpm` |
| **Debian** 12+ | Devuan, MX Linux, Kali | `.deb` |
| **Ubuntu** 24.04+ | Mint, Pop!_OS, elementary, Zorin, KDE neon, Kubuntu | `.deb` |
| **Arch** | Manjaro, EndeavourOS, CachyOS, Garuda | `.pkg.tar.zst` |
| Gentoo, NixOS, Void, Alpine, Solus | — | build from source |

| Works with | |
|---|---|
| **Desktops** | GNOME, KDE Plasma, XFCE, Cinnamon, MATE, Budgie, LXQt, Sway, Hyprland, i3 |
| **Display servers** | Wayland and X11 |
| **App toolkits** | GTK 3/4, Qt 5/6, Electron/Chromium, terminals, Wayland-native apps |
| **Architectures** | x86_64 (prebuilt); aarch64 builds from the same sources |

Not available as Flatpak, Snap or AppImage — those are sandboxed *application*
formats, and an input method is a plugin that must load inside the system's
Fcitx5 process.

Use the package built for your own distribution. A Debian `.deb` and an Ubuntu
`.deb` share a format but are compiled against different library versions; a
Fedora `.rpm` and an openSUSE `.rpm` name their dependencies differently.
[INSTALL.md](INSTALL.md#why-one-package-per-distribution) explains the details.

## Install

1. Download the package for your distribution from the
   [Releases page](https://github.com/Jeevesh-Bhor/Project-Maatra/releases).
2. Install it with your package manager:

   ```bash
   sudo dnf install ./fcitx5-marathi-transliterate-*.rpm                 # Fedora / RHEL
   sudo zypper install ./fcitx5-marathi-transliterate-*.rpm              # openSUSE
   sudo apt install ./fcitx5-marathi-transliterate_*.deb                 # Debian / Ubuntu
   sudo pacman -U ./fcitx5-marathi-transliterate-*.pkg.tar.zst           # Arch
   ```

   Debian and Ubuntu only: also run `im-config -n fcitx5`.
3. Log out and back in.
4. Open **Fcitx5 Configuration**, add **Marathi (Transliteration)**, and press
   **Ctrl+Space** in any text field.

[INSTALL.md](INSTALL.md) walks through each step with step-by-step detail,
plus optional settings and troubleshooting.

## Uninstall

```bash
sudo dnf remove fcitx5-marathi-transliterate       # Fedora / RHEL
sudo zypper remove fcitx5-marathi-transliterate    # openSUSE
sudo apt remove fcitx5-marathi-transliterate       # Debian / Ubuntu
sudo pacman -R fcitx5-marathi-transliterate        # Arch
```

Your learned words stay in `~/.local/share/fcitx5/marathitranslit/`; delete
that directory for a full reset. See [INSTALL.md](INSTALL.md#uninstalling)
for reverting optional settings.

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
User Interface (see [INSTALL.md](INSTALL.md#larger-popup-with-the-maatra-dark-theme)).

## Known limitations

- Standalone postpositions absent from the dictionary (e.g. ला) fall back to
  the plain phonetic reading; pick the intended form from the popup and it
  will be learned.
- Vocalic ऋ (as in कृष्ण) is not yet supported.
- On Wayland the popup is positioned by the compositor and cannot be dragged.

## Development

```
engine/       core transliterator (C++20, no Fcitx5 dependency) + CLI + tests
fcitx5/       the Fcitx5 addon, input-method manifests, and the popup theme
packaging/    RPM spec, Arch PKGBUILD, Debian files, container build script
```

Building from source, running the tests, and producing the packages are
covered in [packaging/README.md](packaging/README.md).

## License

Maatra is free software under the [GNU GPL v3 or later](LICENSE). The bundled
dictionary and language-model data come from openly licensed sources
(GPL and CC-BY-4.0) and are credited in [LICENSES.md](LICENSES.md).
