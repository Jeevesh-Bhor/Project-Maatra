# Data source licensing audit

## Marathi dictionary (`engine/src/mr_words.txt` — words)

- **aspell-mr** (GNU Aspell Marathi Dictionary Package), Fedora package `aspell-mr`. License: GPL-2.0-or-later (per `/usr/share/licenses/aspell-mr/COPYING` at install time). Word list extracted via `aspell -d mr dump master`.
- **hunspell-mr** (Marathi hunspell dictionaries), Fedora package `hunspell-mr`. Base word forms extracted from `/usr/share/hunspell/mr_IN.dic` (affix flags stripped). License: per Fedora packaging (MPL/GPL/LGPL tri-license typical of hunspell dictionaries — verify `rpm -q --license hunspell-mr` before redistribution).

Merged and deduplicated: 100,917 unique Devanagari words.

## Word frequency counts (`engine/src/mr_words.txt` — frequency column)

- **cfilt/IITB-IndicMonoDoc** dataset on Hugging Face (`https://huggingface.co/datasets/cfilt/IITB-IndicMonoDoc`), Marathi (`mr`) split. License: **CC-BY-4.0**. Attribution required on redistribution.
- The full Marathi split was used (~19GB, all 15 shards), giving corpus-frequency coverage for 91.6% of the dictionary. Frequency of 0 means "known dictionary word, not seen in this sample" (not "invalid word").

## Word bigram model (`engine/src/mr_bigrams.txt`)

- Word-pair (bigram) co-occurrence counts computed from the same IITB-IndicMonoDoc sample (CC-BY-4.0). Restricted to pairs whose second word is "confusable" (collides with another dictionary word under the engine's toggle rules), kept when count >= 20. ~1.96M pairs. Used for sentence-context reranking (Layer 2 language model), e.g. मला कळत vs काळात.

Sourced 2026-09-13 for the fcitx5-marathi-transliterate Phase 2 dictionary/frequency layer.
