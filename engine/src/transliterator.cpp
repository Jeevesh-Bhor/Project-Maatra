#include "transliterator.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

// Data directories baked at build time: the installed location first (used by
// the packaged addon), then the source tree (used during development). See
// engine/CMakeLists.txt.
#ifndef MARATHI_DATADIR
#define MARATHI_DATADIR ""
#endif
#ifndef MARATHI_SRCDIR
#define MARATHI_SRCDIR ""
#endif

namespace marathi {

namespace {

std::string trim(std::string s) {
    const char* ws = " \t\r\n";
    auto start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

bool isVowelStart(char c) {
    return c == 'a' || c == 'i' || c == 'u' || c == 'e' || c == 'o';
}

// ASCII-only lowercase (input is Latin keystrokes).
std::string toLower(std::string_view s) {
    std::string out(s);
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return out;
}

// Return the first existing "<dir>/<file>" among the candidate dirs, else "".
// Source dir is tried first: on the dev machine it exists and is always the
// freshest copy; on an end-user machine that absolute path is absent, so the
// installed data dir is used instead.
std::string findData(const std::string& file) {
    for (const char* dir : {MARATHI_SRCDIR, MARATHI_DATADIR}) {
        if (dir && *dir) {
            std::string p = std::string(dir) + "/" + file;
            std::error_code ec;
            if (std::filesystem::exists(p, ec)) return p;
        }
    }
    return {};
}

} // namespace

Transliterator::Transliterator()
    : Transliterator(findData("marathi_rules.txt"), findData("mr_words.txt"),
                     findData("mr_bigrams.txt")) {}

Transliterator::Transliterator(const std::string& rulesPath, const std::string& dictionaryPath,
                               const std::string& bigramsPath) {
    loadRulesFromFile(rulesPath);
    if (!dictionaryPath.empty()) {
        loadDictionaryFromFile(dictionaryPath);
    }
    if (!bigramsPath.empty()) {
        loadBigramsFromFile(bigramsPath);
    }
}

void Transliterator::loadRulesFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Transliterator: could not open rules file: " + path);
    }
    parseRules(in);
}

void Transliterator::loadDictionaryFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return; // dictionary is optional; silently fall back to Layer 1 only
    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        auto tab = trimmed.find('\t');
        std::string word = (tab == std::string::npos) ? trimmed : trimmed.substr(0, tab);
        uint64_t freq = 0;
        if (tab != std::string::npos) {
            try {
                freq = std::stoull(trimmed.substr(tab + 1));
            } catch (...) {
                freq = 0;
            }
        }
        if (!word.empty()) dictionary_[word] = freq;
    }
}

void Transliterator::loadBigramsFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return; // bigram model is optional
    // Format per line: prevWord<TAB>nextWord<TAB>count
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto t1 = line.find('\t');
        if (t1 == std::string::npos) continue;
        auto t2 = line.find('\t', t1 + 1);
        if (t2 == std::string::npos) continue;
        std::string prev = line.substr(0, t1);
        std::string next = line.substr(t1 + 1, t2 - t1 - 1);
        uint64_t count = 0;
        try {
            count = std::stoull(line.substr(t2 + 1));
        } catch (...) {
            continue;
        }
        if (prev.empty() || next.empty()) continue;
        bigrams_[prev + '\x1f' + next] = count;
    }
}

uint64_t Transliterator::bigramCount(const std::string& prev, const std::string& cand) const {
    if (prev.empty() || bigrams_.empty()) return 0;
    auto it = bigrams_.find(prev + '\x1f' + cand);
    return it == bigrams_.end() ? 0 : it->second;
}

void Transliterator::parseRules(std::istream& in) {
    enum class Section { None, Words, Consonants, VowelsIndependent, VowelsMatra };
    Section section = Section::None;
    std::string line;

    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') continue;

        if (trimmed.front() == '[') {
            if (trimmed == "[WORDS]") section = Section::Words;
            else if (trimmed == "[CONSONANTS]") section = Section::Consonants;
            else if (trimmed == "[VOWELS_INDEPENDENT]") section = Section::VowelsIndependent;
            else if (trimmed == "[VOWELS_MATRA]") section = Section::VowelsMatra;
            else section = Section::None;
            continue;
        }

        auto arrow = trimmed.find("->");
        if (arrow == std::string::npos) continue;
        std::string key = trim(trimmed.substr(0, arrow));
        std::string value = trim(trimmed.substr(arrow + 2));
        if (key.empty()) continue;

        switch (section) {
            case Section::Words:
                words_[key] = value;
                break;
            case Section::Consonants:
                consonants_[key] = value;
                maxConsonantKeyLen_ = std::max(maxConsonantKeyLen_, key.size());
                break;
            case Section::VowelsIndependent:
                vowelsIndependent_[key] = value;
                maxVowelKeyLen_ = std::max(maxVowelKeyLen_, key.size());
                break;
            case Section::VowelsMatra:
                vowelsMatra_[key] = value;
                maxVowelKeyLen_ = std::max(maxVowelKeyLen_, key.size());
                break;
            case Section::None:
                break;
        }
    }
}

std::pair<std::string, std::string> Transliterator::longestMatch(
    std::string_view s, size_t pos, const std::map<std::string, std::string>& table,
    size_t maxLen) {
    size_t remaining = s.size() - pos;
    for (size_t len = std::min(maxLen, remaining); len >= 1; --len) {
        std::string key(s.substr(pos, len));
        auto it = table.find(key);
        if (it != table.end()) {
            return {key, it->second};
        }
    }
    return {"", ""};
}

std::vector<Transliterator::Token> Transliterator::tokenize(std::string_view latin) const {
    std::vector<Token> tokens;
    size_t pos = 0;
    while (pos < latin.size()) {
        char c = latin[pos];
        if (isVowelStart(c)) {
            auto [key, glyph] = longestMatch(latin, pos, vowelsIndependent_, maxVowelKeyLen_);
            if (key.empty()) {
                tokens.push_back({false, "", std::string(1, c), ""});
                pos += 1;
                continue;
            }
            auto matraIt = vowelsMatra_.find(key);
            std::string matra = (matraIt != vowelsMatra_.end()) ? matraIt->second : glyph;
            tokens.push_back({false, key, glyph, matra});
            pos += key.size();
        } else {
            auto [key, glyph] = longestMatch(latin, pos, consonants_, maxConsonantKeyLen_);
            if (key.empty()) {
                tokens.push_back({true, "", std::string(1, c), ""});
                pos += 1;
                continue;
            }
            tokens.push_back({true, key, glyph, ""});
            pos += key.size();
        }
    }
    return tokens;
}

std::string Transliterator::assemble(const std::vector<Token>& tokens) const {
    std::string out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].isConsonant) {
            bool hasNext = (i + 1) < tokens.size();
            if (hasNext && !tokens[i + 1].isConsonant) {
                out += tokens[i].glyphBase;
                out += tokens[i + 1].matraGlyph;
                ++i;
            } else if (hasNext && tokens[i + 1].isConsonant) {
                if (tokens[i].asAnusvara) {
                    out += "ं"; // anusvara replaces the nasal letter entirely
                } else if (tokens[i].insertSchwaAfter) {
                    out += tokens[i].glyphBase; // bare consonant: implicit schwa, no halant
                } else {
                    out += tokens[i].glyphBase;
                    out += "्"; // halant / virama
                }
            } else {
                out += tokens[i].glyphBase;
            }
        } else {
            out += tokens[i].glyphBase;
        }
    }
    return out;
}

std::string Transliterator::longVowelAltKey(const std::string& key) {
    if (key == "a") return "aa";
    if (key == "i") return "ii";
    if (key == "u") return "uu";
    return "";
}

namespace {
bool isNasalConsonantGlyph(const std::string& glyph) {
    return glyph == "न" || glyph == "म" || glyph == "ण";
}

// Retroflex counterpart for a dental/plain consonant, or "" if none. Lets
// natural single-letter typing (pani, koni, kalat) reach the retroflex word
// (पाणी, कोणी, कळत) without forcing the user to double the letter, since the
// dictionary+frequency filter only picks the retroflex reading when it's a
// real, common word.
std::string retroflexAlt(const std::string& glyph) {
    if (glyph == "न") return "ण";
    if (glyph == "त") return "ट";
    if (glyph == "थ") return "ठ";
    if (glyph == "द") return "ड";
    if (glyph == "ध") return "ढ";
    if (glyph == "ल") return "ळ";
    if (glyph == "श") return "ष"; // palatal sha -> retroflex sha (महाराष्ट्र, संतोष)
    return "";
}
} // namespace

std::string Transliterator::transliterate(std::string_view latin,
                                          const std::string& previousWord) const {
    auto candidates = generateCandidates(latin, previousWord);
    return candidates.empty() ? std::string() : candidates.front();
}

std::vector<std::string> Transliterator::generateCandidates(
    std::string_view latinIn, const std::string& previousWord) const {
    if (latinIn.empty()) return {};

    // Case-insensitive: users capitalize sentence starts and names ("Mala",
    // "January"); the rule table is all lowercase.
    std::string key = toLower(latinIn);
    std::string_view latin = key;

    auto wordIt = words_.find(key);
    if (wordIt != words_.end()) return {wordIt->second};

    auto baseTokens = tokenize(latin);
    std::string fallback = assemble(baseTokens);

    // Three independent sources of spelling ambiguity, each resolved the same
    // way (try both readings, prefer whichever is a real, more frequent word):
    //   1. Vowel length: mala -> मल vs माला; nahi's final 'i' -> हि vs ही.
    //   2. Nasal-before-stop: हेमन्त (conjunct) vs हेमंत (anusvara) -- Marathi
    //      orthography allows either for a homorganic nasal immediately
    //      before the following consonant, and usage varies word to word.
    //   3. Dropped medial schwa: casual typing often omits the vowel between
    //      two consonants entirely ("karto" for करतो) rather than writing it
    //      out ("karato") -- try treating any such gap as an implicit schwa.
    //   4. Retroflex: single-letter typing reaching ण/ट/ठ/ड/ढ/ळ/ष.
    //   5. Diphthong split: "ai"/"au" are greedily read as ऐ/औ, but in words
    //      like mumbai/bhai/dubai they are two vowels (b + ई) -> मुंबई.
    enum class ToggleKind {
        LongVowel, Anusvara, InsertSchwa, Retroflex,
        SplitDiphthong,      // ai -> a + ई   (mumbai -> मुंबई)
        SplitDiphthongLong   // ai -> आ + ई   (bhai -> भाई)
    };
    struct TogglePoint {
        size_t index;
        ToggleKind kind;
    };
    std::vector<TogglePoint> toggles;
    for (size_t i = 0; i < baseTokens.size(); ++i) {
        const auto& tok = baseTokens[i];
        if (!tok.isConsonant && !longVowelAltKey(tok.key).empty()) {
            toggles.push_back({i, ToggleKind::LongVowel});
        } else if (!tok.isConsonant && (tok.key == "ai" || tok.key == "au")) {
            // Both variants; Long is pushed after Short so it wins if both
            // bits are set (the dictionary filters either way).
            toggles.push_back({i, ToggleKind::SplitDiphthong});
            toggles.push_back({i, ToggleKind::SplitDiphthongLong});
        } else if (tok.isConsonant) {
            if (!retroflexAlt(tok.glyphBase).empty()) {
                toggles.push_back({i, ToggleKind::Retroflex});
            }
            if (i + 1 < baseTokens.size() && baseTokens[i + 1].isConsonant) {
                if (isNasalConsonantGlyph(tok.glyphBase)) {
                    toggles.push_back({i, ToggleKind::Anusvara});
                }
                toggles.push_back({i, ToggleKind::InsertSchwa});
            }
        }
    }

    // Cap the search space: pathological/gibberish input (many consonants in
    // a row) would otherwise blow up 2^n. Real words never need this many
    // simultaneous toggles, so just return the plain composition.
    constexpr size_t kMaxToggles = 15;
    if (toggles.empty() || dictionary_.empty() || toggles.size() > kMaxToggles) {
        return {fallback};
    }

    size_t n = toggles.size();
    size_t variantCount = size_t(1) << n; // small in practice: word length bounds n

    // Enumerate every reading, keep the dictionary-valid ones with their corpus
    // frequency and how many toggles they needed. Ranked so that higher
    // frequency wins first (a rare zero-toggle word like "नहि" must not beat a
    // common two-toggle word like "नाही"); fewer toggles breaks ties among
    // words the small frequency sample never saw (freq == 0).
    struct Ranked {
        std::string text;
        uint64_t bigram;   // co-occurrence count with previousWord (context)
        uint64_t freq;     // standalone corpus frequency
        int toggleCount;
    };
    std::vector<Ranked> hits;
    for (uint32_t mask = 0; mask < variantCount; ++mask) {
        auto tokens = baseTokens;
        for (size_t bit = 0; bit < n; ++bit) {
            if (!(mask & (1u << bit))) continue;
            auto& tok = tokens[toggles[bit].index];
            if (toggles[bit].kind == ToggleKind::LongVowel) {
                std::string altKey = longVowelAltKey(tok.key);
                auto indepIt = vowelsIndependent_.find(altKey);
                auto matraIt = vowelsMatra_.find(altKey);
                if (indepIt != vowelsIndependent_.end()) tok.glyphBase = indepIt->second;
                if (matraIt != vowelsMatra_.end()) tok.matraGlyph = matraIt->second;
            } else if (toggles[bit].kind == ToggleKind::Anusvara) {
                tok.asAnusvara = true;
            } else if (toggles[bit].kind == ToggleKind::Retroflex) {
                tok.glyphBase = retroflexAlt(tok.glyphBase);
            } else if (toggles[bit].kind == ToggleKind::SplitDiphthong) {
                // "ai" -> a (schwa) + independent ई ; "au" -> a + independent ऊ.
                // After a consonant the 'a' is the inherent vowel (no matra),
                // so the whole contribution is just the independent long vowel.
                if (tok.key == "ai") {
                    tok.matraGlyph = "ई";
                    tok.glyphBase = "अई";
                } else {
                    tok.matraGlyph = "ऊ";
                    tok.glyphBase = "अऊ";
                }
            } else if (toggles[bit].kind == ToggleKind::SplitDiphthongLong) {
                // Same split but with a long आ: "ai" -> आ + ई (bhai -> भाई).
                if (tok.key == "ai") {
                    tok.matraGlyph = "ाई";
                    tok.glyphBase = "आई";
                } else {
                    tok.matraGlyph = "ाऊ";
                    tok.glyphBase = "आऊ";
                }
            } else {
                tok.insertSchwaAfter = true;
            }
        }
        std::string candidate = assemble(tokens);
        auto dictIt = dictionary_.find(candidate);
        if (dictIt == dictionary_.end()) continue;
        uint64_t bg = bigramCount(previousWord, candidate);
        hits.push_back({std::move(candidate), bg, dictIt->second, __builtin_popcount(mask)});
    }

    // Context first: a candidate that actually follows the previous word in the
    // corpus outranks a globally-more-frequent word that doesn't (this is what
    // makes "मला kalat" -> कळत even though काळात is more frequent overall).
    // Then standalone frequency, then fewer edits.
    std::stable_sort(hits.begin(), hits.end(), [](const Ranked& a, const Ranked& b) {
        if (a.bigram != b.bigram) return a.bigram > b.bigram;
        if (a.freq != b.freq) return a.freq > b.freq;
        return a.toggleCount < b.toggleCount;
    });

    std::vector<std::string> result;
    auto pushUnique = [&result](const std::string& s) {
        if (std::find(result.begin(), result.end(), s) == result.end()) {
            result.push_back(s);
        }
    };
    for (const auto& h : hits) pushUnique(h.text);
    // Always offer the plain phonetic composition too, so the user can reach
    // it even when the dictionary's ranking guessed wrong (and so the list is
    // never empty when there were no dictionary hits at all).
    pushUnique(fallback);
    return result;
}

std::string Transliterator::transliteratePartial(std::string_view latinIn) const {
    std::string key = toLower(latinIn);
    auto wordIt = words_.find(key);
    if (wordIt != words_.end()) return wordIt->second;
    return assemble(tokenize(key));
}

} // namespace marathi
