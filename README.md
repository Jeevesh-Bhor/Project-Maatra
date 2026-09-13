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

## Install

**Fedora users:** install the `.rpm` and follow the four short steps in
[INSTALL.md](INSTALL.md) (install → log out/in → add the input method → type).

**Building from source:**

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
engine/    core transliterator (C++20, no Fcitx5 dependency) + CLI + tests
fcitx5/    the Fcitx5 addon, input-method manifests, and the popup theme
*.spec     Fedora RPM packaging
```

The engine can be exercised without a GUI:

```bash
echo "mala kalat nahi" | ./build/engine/mr-translit-cli
```
