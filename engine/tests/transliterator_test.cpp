#include "transliterator.h"

#include <algorithm>

#include <gtest/gtest.h>

namespace {

// The dictionary + bigram model are large and read-only, so load them once for
// the whole suite instead of per-test (keeps the run fast).
class TransliteratorTest : public ::testing::Test {
protected:
    static const marathi::Transliterator& engine() {
        static const marathi::Transliterator instance;
        return instance;
    }
    const marathi::Transliterator& t = engine();
};

TEST_F(TransliteratorTest, DictionaryResolvesSchwaVsLongAAmbiguityMala) {
    // "मल" (pure schwa reading) isn't a real word. Of the real-word variants,
    // "मला" ("to me") is a far more common pronoun than "माला" ("garland"),
    // so frequency ranking should prefer it -- this is also what a
    // frequency/corpus-driven system like Google's keyboard produces.
    EXPECT_EQ(t.transliterate("mala"), "मला");
}

TEST_F(TransliteratorTest, DictionaryResolvesFinalShortIVsLongIAmbiguityNahi) {
    EXPECT_EQ(t.transliterate("nahi"), "नाही");
}

TEST_F(TransliteratorTest, DictionaryResolvesGenericWordNotHardcoded) {
    // "कम" isn't a real word but "काम" is — this must come purely from the
    // 70k-word dictionary lookup, not a hand-written override.
    EXPECT_EQ(t.transliterate("kam"), "काम");
}

TEST_F(TransliteratorTest, AmbiguousInputPrefersMostFrequentRealWord) {
    // "kalat" has several real readings once retroflex/vowel toggles are
    // considered: काळात (in the era, very common), कळत (to understand),
    // कलत (rare). Frequency ranking picks the most common, काळात; the others
    // remain available in the candidate list. (Picking कळत specifically would
    // need sentence/bigram context — Layer 3, out of scope here.)
    EXPECT_EQ(t.transliterate("kalat"), "काळात");
    auto cands = t.generateCandidates("kalat");
    EXPECT_NE(std::find(cands.begin(), cands.end(), "कळत"), cands.end());
}

TEST_F(TransliteratorTest, ConjunctMechanicStillFormsHalantWhenDictionaryPrefersIt) {
    // "shabda" -> शब्द (word) is a real, common conjunct; "शबद" (its
    // schwa-inserted alternative) isn't a word at all, so the halant
    // conjunct mechanic must still win here.
    EXPECT_EQ(t.transliterate("shabda"), "शब्द");
}

TEST_F(TransliteratorTest, BareConjunctPrefersRealWordOverRuleArtifact) {
    // "kta"/"tra" alone are not words anyone actually means to type; both
    // "त्र" (rare, mostly a compounding suffix) and "क्त" are dictionary
    // misses, while their dropped-schwa readings are real, far more common
    // words ("तर" = "then", freq in the tens of thousands). Prefer the word.
    EXPECT_EQ(t.transliterate("tra"), "तर");
}

TEST_F(TransliteratorTest, RetroflexLa) {
    EXPECT_EQ(t.transliterate("vall"), "वळ");
}

TEST_F(TransliteratorTest, RetroflexNa) {
    // Doubling still forces the retroflex, as a selectable reading, even for a
    // non-word: "nn" now reads as a geminate first.
    auto cands = t.generateCandidates("nann");
    EXPECT_NE(std::find(cands.begin(), cands.end(), "नण"), cands.end());
}

TEST_F(TransliteratorTest, NasalConjunctVsAnusvaraSpellingVariant) {
    // हेमन्त (explicit conjunct) and हेमंत (anusvara) are both valid Marathi
    // spellings of the same word; modern usage overwhelmingly prefers
    // anusvara, which is what the dictionary+frequency data should surface.
    EXPECT_EQ(t.transliterate("hemant"), "हेमंत");
}

TEST_F(TransliteratorTest, DroppedMedialSchwaKarto) {
    // Casual typing commonly omits the vowel between two consonants
    // ("karto") rather than spelling it out ("karato").
    EXPECT_EQ(t.transliterate("karto"), "करतो");
}

TEST_F(TransliteratorTest, DroppedMedialSchwaKarte) {
    EXPECT_EQ(t.transliterate("karte"), "करते");
}

TEST_F(TransliteratorTest, RetroflexAspiratedTha) {
    EXPECT_EQ(t.transliterate("kutthe"), "कुठे");
}

TEST_F(TransliteratorTest, RetroflexReachableFromNaturalSingleLetterTyping) {
    // The user should not have to double retroflex consonants for common
    // words: the dictionary+frequency filter recovers the retroflex reading
    // from plain phonetic typing.
    EXPECT_EQ(t.transliterate("pani"), "पाणी");   // न -> ण
    EXPECT_EQ(t.transliterate("koni"), "कोणी");   // न -> ण
    EXPECT_EQ(t.transliterate("shala"), "शाळा");  // ल -> ळ
    EXPECT_EQ(t.transliterate("kuthe"), "कुठे");  // थ -> ठ
}

TEST_F(TransliteratorTest, RetroflexSibilantFromNaturalTyping) {
    // ष (retroflex sha) reachable from plain "sh" typing for common loan/
    // tatsama words.
    EXPECT_EQ(t.transliterate("maharashtra"), "महाराष्ट्र");
    EXPECT_EQ(t.transliterate("santosh"), "संतोष");
    EXPECT_EQ(t.transliterate("vishay"), "विषय");
}

TEST_F(TransliteratorTest, PlainWordGaraj) {
    EXPECT_EQ(t.transliterate("garaj"), "गरज");
}

TEST_F(TransliteratorTest, DoubledEIsLongIConvention) {
    // "ee" is the common English-transliteration digraph for long dependent-ii,
    // not two short 'e's (matches how names like "Neelesh" are typically spelled).
    EXPECT_EQ(t.transliterate("neelesh"), "नीलेश");
}

TEST_F(TransliteratorTest, DoubledOIsLongUConvention) {
    EXPECT_EQ(t.transliterate("poonam"), "पूनम");
}

TEST_F(TransliteratorTest, BigramContextOverridesGlobalFrequency) {
    // Standalone, "kalat" -> काळात (globally most frequent reading).
    EXPECT_EQ(t.transliterate("kalat"), "काळात");
    // After "मला", the corpus strongly favours कळत ("मला कळत" is common), so
    // context should flip the top pick even though काळात is more frequent
    // overall. This is the flagship "मला कळत नाही" case.
    EXPECT_EQ(t.transliterate("kalat", "मला"), "कळत");
}

TEST_F(TransliteratorTest, CaseInsensitive) {
    // Capitalized sentence starts and names must not leak raw ASCII.
    EXPECT_EQ(t.transliterate("Mala"), t.transliterate("mala"));
    EXPECT_EQ(t.transliterate("MALA"), t.transliterate("mala"));
    EXPECT_EQ(t.transliterate("Mumbai"), "मुंबई");
}

TEST_F(TransliteratorTest, EnglishMonthAndDayLoanwords) {
    // Standard Marathi spellings of English month/day names don't follow from
    // the English spelling phonetically, so they come from word overrides.
    EXPECT_EQ(t.transliterate("january"), "जानेवारी");
    EXPECT_EQ(t.transliterate("January"), "जानेवारी");
    EXPECT_EQ(t.transliterate("august"), "ऑगस्ट");
    EXPECT_EQ(t.transliterate("december"), "डिसेंबर");
    EXPECT_EQ(t.transliterate("monday"), "सोमवार");
    EXPECT_EQ(t.transliterate("sunday"), "रविवार");
}

TEST_F(TransliteratorTest, DiphthongVsTwoVowels) {
    // "ai" is the diphthong ऐ in some words but two vowels in others; the
    // dictionary decides.
    EXPECT_EQ(t.transliterate("hai"), "है");        // diphthong kept
    EXPECT_EQ(t.transliterate("gaurav"), "गौरव");    // "au" diphthong kept
    EXPECT_EQ(t.transliterate("mumbai"), "मुंबई");   // a + ई
    EXPECT_EQ(t.transliterate("bhai"), "भाई");       // आ + ई
    EXPECT_EQ(t.transliterate("aai"), "आई");
}

TEST_F(TransliteratorTest, NumberWords) {
    EXPECT_EQ(t.transliterate("ek"), "एक");
    EXPECT_EQ(t.transliterate("don"), "दोन");
    EXPECT_EQ(t.transliterate("teen"), "तीन");
    EXPECT_EQ(t.transliterate("daha"), "दहा");
}

TEST_F(TransliteratorTest, GenerateCandidatesReturnsRankedAlternatives) {
    auto cands = t.generateCandidates("mala");
    ASSERT_GE(cands.size(), 2u);
    EXPECT_EQ(cands.front(), "मला");            // most frequent reading first
    // The alternative long-vowel reading should also be offered.
    EXPECT_NE(std::find(cands.begin(), cands.end(), "माला"), cands.end());
    // No duplicates.
    auto sorted = cands;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_EQ(std::unique(sorted.begin(), sorted.end()), sorted.end());
}

TEST_F(TransliteratorTest, GenerateCandidatesNeverEmptyForGibberish) {
    auto cands = t.generateCandidates("bcdfg");
    ASSERT_FALSE(cands.empty()); // always at least the plain composition
}

TEST_F(TransliteratorTest, GeminateConjunctFallsThroughForNonRetroflexLetters) {
    // 'kk' is not a retroflex-eligible pair, so it should fall through to the
    // ordinary consonant+consonant halant rule instead of a special glyph.
    EXPECT_EQ(t.transliterate("kka"), "क्क");
}

TEST_F(TransliteratorTest, DoubledLettersReadAsGeminates) {
    EXPECT_EQ(t.transliterate("uttar"), "उत्तर");
    EXPECT_EQ(t.transliterate("shuddha"), "शुद्ध");
    EXPECT_EQ(t.transliterate("prasiddha"), "प्रसिद्ध");
}

TEST_F(TransliteratorTest, DoubledLettersOfferBothReadings) {
    // पत्ता (address) and पट्टा (belt) are both real words typed the same way.
    auto cands = t.generateCandidates("patta");
    EXPECT_NE(std::find(cands.begin(), cands.end(), "पत्ता"), cands.end());
    EXPECT_NE(std::find(cands.begin(), cands.end(), "पट्टा"), cands.end());
}

TEST_F(TransliteratorTest, AnusvaraBeforeNa) {
    EXPECT_EQ(t.transliterate("tyanna"), "त्यांना");
    EXPECT_EQ(t.transliterate("tyanni"), "त्यांनी");
    EXPECT_EQ(t.transliterate("yani"), "यांनी");
}

TEST_F(TransliteratorTest, FinalAnusvaraIsOffered) {
    auto cands = t.generateCandidates("kela");
    EXPECT_EQ(cands.front(), "केला");
    EXPECT_NE(std::find(cands.begin(), cands.end(), "केलं"), cands.end());
}

TEST_F(TransliteratorTest, ZAndFMeanPlainLetters) {
    EXPECT_EQ(t.transliterate("zala"), "झाला");
    EXPECT_EQ(t.transliterate("fakt"), "फक्त");
    EXPECT_EQ(t.transliterate("roz"), "रोज");
}

TEST_F(TransliteratorTest, LettersThatHadNoKey) {
    EXPECT_EQ(t.transliterate("gyan"), "ज्ञान");
    EXPECT_EQ(t.transliterate("dnyan"), "ज्ञान");
    EXPECT_EQ(t.transliterate("mrutyu"), "मृत्यू");
    EXPECT_EQ(t.transliterate("rushi"), "ऋषी");
    EXPECT_EQ(t.transliterate("karanarya"), "करणाऱ्या");
    EXPECT_EQ(t.transliterate("dukh"), "दुःख");
    EXPECT_EQ(t.transliterate("bank"), "बँक");
}

TEST_F(TransliteratorTest, LooseReadingsDoNotBeatDirectOnes) {
    EXPECT_EQ(t.transliterate("asa"), "असा");
    EXPECT_EQ(t.transliterate("che"), "चे");
    EXPECT_EQ(t.transliterate("khara"), "खरा");
}

TEST_F(TransliteratorTest, DoubledAspiratedRetroflex) {
    EXPECT_EQ(t.transliterate("vitthal"), "विठ्ठल");
    EXPECT_EQ(t.transliterate("vithal"), "विठ्ठल");
    EXPECT_EQ(t.transliterate("atthavis"), "अठ्ठावीस");
    EXPECT_EQ(t.transliterate("athavis"), "अठ्ठावीस");
}

TEST_F(TransliteratorTest, LongWordsAreNotGivenUpOn) {
    EXPECT_EQ(t.transliterate("mukhyamantryanna"), "मुख्यमंत्र्यांना");
    EXPECT_EQ(t.transliterate("sanskrutimadhye"), "संस्कृतीमध्ये");
    EXPECT_EQ(t.transliterate("sangitalan"), "सांगितलं");
}

TEST_F(TransliteratorTest, LoanwordVowels) {
    // Both spellings are in use; the more common one ranks first.
    auto mobile = t.generateCandidates("mobail");
    EXPECT_EQ(mobile.front(), "मोबाईल");
    EXPECT_NE(std::find(mobile.begin(), mobile.end(), "मोबाइल"), mobile.end());
    auto download = t.generateCandidates("daunlod");
    EXPECT_NE(std::find(download.begin(), download.end(), "डाउनलोड"), download.end());
    EXPECT_EQ(t.transliterate("tomato"), "टोमॅटो");
}

TEST_F(TransliteratorTest, AnusvaraWithTheNasalLeftOut) {
    EXPECT_EQ(t.transliterate("kiva"), "किंवा");
    EXPECT_EQ(t.transliterate("mubai"), "मुंबई");
    EXPECT_EQ(t.transliterate("sagitale"), "सांगितले");
    EXPECT_EQ(t.transliterate("dogar"), "डोंगर");
    EXPECT_EQ(t.transliterate("natar"), "नंतर");
}

TEST_F(TransliteratorTest, AnusvaraOnAFinalConsonant) {
    for (auto [typed, word] : {std::pair{"hot", "होतं"}, {"moth", "मोठं"}, {"sagal", "सगळं"}}) {
        auto cands = t.generateCandidates(typed);
        EXPECT_NE(std::find(cands.begin(), cands.end(), word), cands.end()) << typed;
    }
}

TEST_F(TransliteratorTest, LeftOutAnusvaraIsOfferedNotForced) {
    // Both are real words; the reading that matches what was typed stays first.
    auto cands = t.generateCandidates("tyachya");
    EXPECT_EQ(cands.front(), "त्याच्या");
    EXPECT_NE(std::find(cands.begin(), cands.end(), "त्यांच्या"), cands.end());
}

TEST_F(TransliteratorTest, WordOverrideKeepsOtherReadings) {
    auto cands = t.generateCandidates("june");
    EXPECT_EQ(cands.front(), "जून");
    EXPECT_NE(std::find(cands.begin(), cands.end(), "जुने"), cands.end());
}

} // namespace
