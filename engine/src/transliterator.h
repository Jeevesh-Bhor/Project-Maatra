#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace marathi {

// Layer 1 (deterministic phonetic rules) + Layer 2 (real-word dictionary
// filtering over a small candidate lattice, ranked by corpus unigram frequency
// and previous-word bigram context). No Fcitx5 dependency — usable standalone
// (CLI, unit tests) or linked into the addon.
class Transliterator {
public:
    // Loads rules/dictionary/bigrams from the paths baked in at build time
    // (MARATHI_RULES_PATH, MARATHI_WORDS_PATH, MARATHI_BIGRAMS_PATH). The
    // dictionary and bigram model are optional: if a file can't be opened,
    // that layer is simply skipped.
    Transliterator();
    // Loads from explicit file paths (dictionaryPath/bigramsPath may be empty).
    explicit Transliterator(const std::string& rulesPath,
                             const std::string& dictionaryPath = "",
                             const std::string& bigramsPath = "");

    // Transliterate a complete, finished word (used on commit). Returns the
    // single best reading. `previousWord` is the Devanagari word just committed
    // before this one (empty at sentence start); when present, bigram context
    // reranks the candidates so e.g. "kalat" after "मला" prefers कळत.
    [[nodiscard]] std::string transliterate(std::string_view latin,
                                            const std::string& previousWord = "") const;

    // Ranked list of plausible Devanagari readings for the typed Latin, best
    // first, deduplicated. Ranking: bigram context (if previousWord given) >
    // corpus unigram frequency > fewer edits. The plain Layer-1 composition is
    // always included as a fallback so the list is never empty.
    [[nodiscard]] std::vector<std::string> generateCandidates(
        std::string_view latin, const std::string& previousWord = "") const;

    // Transliterate a buffer that may still be growing (used for live preedit).
    // Deliberately cheap: Layer 1 composition only, no dictionary lattice
    // search, so keystroke-by-keystroke preview stays fast.
    [[nodiscard]] std::string transliteratePartial(std::string_view latin) const;

private:
    struct Token {
        bool isConsonant;
        std::string key;         // latin key (vowels only; empty for consonants)
        std::string glyphBase;   // consonant base glyph, or independent vowel glyph
        std::string matraGlyph;  // only meaningful for vowel tokens
        // When true and this consonant is immediately followed by another
        // consonant, assemble() renders it as anusvara (ं) instead of
        // glyphBase + halant -- e.g. "न्त" vs "ंत" in हेमन्त/हेमंत. Only
        // meaningful for nasal consonants (न, म, ण); set per-candidate by
        // the lattice search, never on the base tokenization.
        bool asAnusvara = false;
        // When true and this consonant is immediately followed by another
        // consonant, assemble() renders it as a bare (schwa) consonant
        // instead of glyphBase + halant -- i.e. treats a dropped medial
        // vowel in casual typing ("karto") as implicitly present ("करतो").
        // Ignored if asAnusvara is also set (anusvara wins).
        bool insertSchwaAfter = false;
    };

    std::map<std::string, std::string> words_;             // whole-word overrides
    std::map<std::string, std::string> consonants_;         // latin -> base glyph (schwa included)
    std::map<std::string, std::string> vowelsIndependent_;  // latin -> independent vowel glyph
    std::map<std::string, std::string> vowelsMatra_;        // latin -> dependent matra (may be empty)
    std::unordered_map<std::string, uint64_t> dictionary_;  // real Devanagari word -> corpus frequency
    // Bigram counts keyed by "prevWord\x1fcandidate" -> co-occurrence count.
    std::unordered_map<std::string, uint64_t> bigrams_;

    size_t maxConsonantKeyLen_ = 1;
    size_t maxVowelKeyLen_ = 1;

    void loadRulesFromFile(const std::string& path);
    void loadDictionaryFromFile(const std::string& path);
    void loadBigramsFromFile(const std::string& path);
    void parseRules(std::istream& in);
    [[nodiscard]] uint64_t bigramCount(const std::string& prev, const std::string& cand) const;

    std::vector<Token> tokenize(std::string_view latin) const;
    std::string assemble(const std::vector<Token>& tokens) const;

    // "a"->"aa", "i"->"ii", "u"->"uu"; empty if key isn't a togglable short vowel.
    static std::string longVowelAltKey(const std::string& key);

    // Longest-match lookup at position `pos` in `s` against `table` (up to maxLen chars).
    static std::pair<std::string, std::string> longestMatch(
        std::string_view s, size_t pos, const std::map<std::string, std::string>& table,
        size_t maxLen);
};

} // namespace marathi
