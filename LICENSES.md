# Licenses

## Maatra

Copyright © 2026 the Maatra contributors.

The Maatra source code — the transliteration engine (`engine/`), the Fcitx5
addon and theme (`fcitx5/`), and the packaging — is licensed under the
**GNU General Public License, version 3 or (at your option) any later
version**. The full text is in [LICENSE](LICENSE).

## Third-party data shipped with Maatra

Maatra includes a Marathi dictionary and a statistical language model built
from the sources below. They are redistributed under their own licenses, all
of which are compatible with GPL-3.0-or-later. Their notices must be kept
with any redistribution of Maatra or of the data files.

### Marathi word list — `engine/src/mr_words.txt` (word column)

Merged and deduplicated from two dictionaries (100,917 unique words):

- **GNU Aspell Marathi Word List** (`aspell-mr`)
  Copyright © 2005 Swapnil Hajare and Swapnil Sant, janabhaaratii, NCST.
  Licensed under the GNU General Public License, version 2 or (at your option)
  any later version, per the upstream `Copyright` file. (Fedora's package
  metadata records it as GPL-2.0-only; the upstream notice's "or any later
  version" clause governs, making it compatible with GPL-3.)
  Extracted with `aspell -d mr dump master`.

- **Marathi Hunspell dictionary** (`hunspell-mr`, `mr_IN.dic`)
  Licensed under the GNU General Public License, version 3 or (at your option)
  any later version (per `rpm -q --qf '%{LICENSE}' hunspell-mr`).
  Base word forms extracted with affix flags stripped.

### Word frequencies and bigram model — `engine/src/mr_words.txt` (count column), `engine/src/mr_bigrams.txt`

Derived from the Marathi split of **IITB-IndicMonoDoc**, published by CFILT,
Indian Institute of Technology Bombay:
<https://huggingface.co/datasets/cfilt/IITB-IndicMonoDoc>

Licensed under **Creative Commons Attribution 4.0 International
(CC-BY-4.0)**: <https://creativecommons.org/licenses/by/4.0/>.
Attribution is required on redistribution; this notice satisfies it.

The files contain only aggregate statistics computed from the corpus — per-word
occurrence counts and word-pair co-occurrence counts (pairs kept when the
second word is ambiguous under the engine's rules and the count is ≥ 20;
~1.96 M pairs). No text from the corpus is included.

Statistics computed 2026-09-13 from the complete Marathi split (~19 GB).
