#include "marathi_engine.h"

#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>

#include <pango/pangocairo.h>

#include <fcitx-utils/standardpath.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fcitx {

namespace {

// A candidate that commits its own Devanagari text back through the engine.
class MarathiCandidate : public CandidateWord {
public:
    MarathiCandidate(MarathiEngine* engine, std::string text, const std::string& padding)
        : CandidateWord(Text(text)), engine_(engine), value_(std::move(text)) {
        setPadding(this, padding);
    }

    void select(InputContext* ic) const override {
        engine_->commitCandidate(ic, value_);
    }

private:
    // Older Fcitx5, as on Debian 12, has no setComment. There the popup just
    // keeps a variable width. A template, so the check compiles on either.
    template <typename Candidate>
    static void setPadding(Candidate* candidate, const std::string& padding) {
        if constexpr (requires { candidate->setComment(Text(padding)); }) {
            candidate->setComment(Text(padding));
        }
    }

    MarathiEngine* engine_;
    std::string value_;
};

// A blank, unnumbered row that keeps the popup's height when words run out.
class FillerCandidate : public CandidateWord {
public:
    FillerCandidate() { setCustomLabel(Text("")); }
    void select(InputContext*) const override {}
};

// The popup is exactly as tall as its rows and as wide as its widest row, and
// Fcitx5 has no setting for either. ScrollingCandidateList fixes the height by
// always reporting kVisibleRows rows. WidthPadder fixes the width.
constexpr int kVisibleRows = 5;

// Row width, in em, available for a word. Fits 99.99% of running text and about
// 99% of dictionary words; a wider word still stretches the popup.
constexpr double kWordWidthEm = 6.0;

// Pads each row to the same width with invisible spaces in its comment, which
// is display-only. The theme must set CommentTextSizeFactor=100, as the padding
// is measured at the candidate font size.
class WidthPadder {
public:
    WidthPadder() {
        context_ = pango_font_map_create_context(pango_cairo_font_map_get_default());
        PangoFontDescription* font = pango_font_description_from_string(popupFont().c_str());
        pango_context_set_font_description(context_, font);
        pango_font_description_free(font);
        layout_ = pango_layout_new(context_);
        rowWidth_ = kWordWidthEm * measure("\u2003");
        spaceWidth_ = measure(kSpace);
    }
    ~WidthPadder() {
        g_object_unref(layout_);
        g_object_unref(context_);
    }
    WidthPadder(const WidthPadder&) = delete;
    WidthPadder& operator=(const WidthPadder&) = delete;

    std::string paddingFor(const std::string& word) {
        if (spaceWidth_ <= 0) return {};
        int count = static_cast<int>(std::lround((rowWidth_ - measure(word)) / spaceWidth_));
        std::string padding;
        for (int i = 0; i < count; ++i) padding += kSpace;
        return padding;
    }

private:
    static constexpr const char* kSpace = "\u200A"; // hair space: fine-grained

    int measure(const std::string& text) {
        pango_layout_set_text(layout_, text.c_str(), static_cast<int>(text.size()));
        int width = 0;
        pango_layout_get_size(layout_, &width, nullptr);
        return width;
    }

    // The font the popup draws with, from Classic UI's config.
    static std::string popupFont() {
        std::ifstream config(StandardPath::global().userDirectory(StandardPath::Type::PkgConfig) +
                             "/conf/classicui.conf");
        std::string line;
        while (std::getline(config, line)) {
            if (line.rfind("Font=", 0) != 0) continue;
            std::string font = line.substr(5);
            font.erase(std::remove(font.begin(), font.end(), '"'), font.end());
            return font;
        }
        return "Sans 10"; // Classic UI's default
    }

    PangoContext* context_ = nullptr;
    PangoLayout* layout_ = nullptr;
    double rowWidth_ = 0;
    double spaceWidth_ = 0;
};

// A fixed window sliding down a single list, one row at a time. Fcitx5's own
// list can only page. Keyboard moves wrap around; the mouse wheel scrolls.
class ScrollingCandidateList : public CandidateList,
                               public PageableCandidateList,
                               public CursorMovableCandidateList {
public:
    ScrollingCandidateList(MarathiEngine* engine, const std::vector<std::string>& words) {
        setPageable(this);
        setCursorMovable(this);
        static WidthPadder padder;
        for (const auto& word : words) {
            words_.push_back(
                std::make_unique<MarathiCandidate>(engine, word, padder.paddingFor(word)));
        }
        for (int i = 1; i <= kVisibleRows; ++i) {
            labels_.emplace_back(std::to_string(i) + ". ");
        }
    }

    const Text& label(int idx) const override { return labels_.at(idx); }
    const CandidateWord& candidate(int idx) const override {
        int row = offset_ + idx;
        if (row < total()) return *words_[row];
        return blank_;
    }
    int size() const override { return kVisibleRows; }
    int cursorIndex() const override { return cursor_ - offset_; }
    CandidateLayoutHint layoutHint() const override { return CandidateLayoutHint::Vertical; }

    bool hasPrev() const override { return offset_ > 0; }
    bool hasNext() const override { return offset_ + kVisibleRows < total(); }
    void prev() override {
        if (!hasPrev()) return;
        --offset_;
        cursor_ = std::min(cursor_, offset_ + kVisibleRows - 1);
    }
    void next() override {
        if (!hasNext()) return;
        ++offset_;
        cursor_ = std::max(cursor_, offset_);
    }
    bool usedNextBefore() const override { return false; }

    void prevCandidate() override { moveCursor(cursor_ == 0 ? total() - 1 : cursor_ - 1); }
    void nextCandidate() override { moveCursor(cursor_ + 1 == total() ? 0 : cursor_ + 1); }

private:
    int total() const { return static_cast<int>(words_.size()); }
    void moveCursor(int cursor) {
        cursor_ = cursor;
        if (cursor_ < offset_) offset_ = cursor_;
        if (cursor_ >= offset_ + kVisibleRows) offset_ = cursor_ - kVisibleRows + 1;
    }

    std::vector<std::unique_ptr<MarathiCandidate>> words_;
    std::vector<Text> labels_;
    FillerCandidate blank_;
    int offset_ = 0;
    int cursor_ = 0;
};

} // namespace

MarathiEngine::MarathiEngine(Instance* instance) : instance_(instance) {}

void MarathiEngine::keyEvent(const InputMethodEntry&, KeyEvent& event) {
    if (event.isRelease()) {
        return;
    }
    auto ic = event.inputContext();
    auto key = event.key();

    // Ctrl+Space: toggle global Latin passthrough mode.
    if (key.check(FcitxKey_space, KeyState::Ctrl)) {
        if (!buffer_.empty()) {
            commitSelected(ic);
        }
        passthrough_ = !passthrough_;
        event.filterAndAccept();
        return;
    }

    if (passthrough_) {
        return; // let every key through untouched while in passthrough mode
    }

    // Other modifier combos (Ctrl+C, etc.): flush the pending word, then let
    // the shortcut reach the application unmodified.
    if (key.states().testAny(KeyStates{KeyState::Ctrl, KeyState::Alt, KeyState::Super})) {
        if (!buffer_.empty()) {
            commitSelected(ic);
        }
        return;
    }

    const bool composing = !buffer_.empty();

    // Escape: drop the conversion, commit the raw Latin as typed (useful for
    // English words / names inside Marathi text).
    if (key.check(FcitxKey_Escape)) {
        if (composing) {
            std::string raw = buffer_;
            clearState(ic);
            ic->commitString(raw);
            lastCommittedWord_.clear(); // English word breaks Marathi context
            event.filterAndAccept();
        }
        return;
    }

    if (key.check(FcitxKey_BackSpace)) {
        if (composing) {
            buffer_.pop_back();
            updateUI(ic);
            event.filterAndAccept();
        }
        return;
    }

    // Candidate navigation only matters while composing.
    if (composing) {
        auto candidateList = ic->inputPanel().candidateList();

        // Down / Tab move the highlight forward, Up / Shift+Tab move it back
        // (Fcitx5's own default bindings). Tab is deliberately swallowed here:
        // letting it through would commit the word and then move keyboard
        // focus out of the text field. Space / Enter commit the highlighted
        // candidate, which is the top pick unless the user moved it.
        const bool shiftTab = key.check(FcitxKey_Tab, KeyState::Shift) ||
                              key.sym() == FcitxKey_ISO_Left_Tab;
        const bool next = key.check(FcitxKey_Down) || key.check(FcitxKey_Tab);
        const bool prev = key.check(FcitxKey_Up) || shiftTab;
        if (next || prev) {
            if (auto* movable = candidateList ? candidateList->toCursorMovable() : nullptr) {
                if (next) {
                    movable->nextCandidate();
                } else {
                    movable->prevCandidate();
                }
                setPreedit(ic, highlightedText(ic));
                ic->updateUserInterface(UserInterfaceComponent::InputPanel);
            }
            event.filterAndAccept();
            return;
        }

        // Number keys select the corresponding visible row.
        if (key.isDigit() && candidateList) {
            int idx = key.digit() - 1; // '1' -> 0
            if (idx >= 0 && idx < kVisibleRows) {
                candidateList->candidate(idx).select(ic);
                event.filterAndAccept();
                return;
            }
        }

        if (key.check(FcitxKey_Return)) {
            commitSelected(ic);
            lastCommittedWord_.clear(); // newline ends the sentence/context
            event.filterAndAccept();
            return;
        }

        if (key.check(FcitxKey_space)) {
            // One commit, not two: Chromium-based browsers can keep only the
            // last commit of a key press, which would lose the word and leave
            // just the space.
            commitSelected(ic, " ");
            event.filterAndAccept();
            return;
        }
    }

    // A Latin letter (composing or starting a new word).
    if (key.isLAZ() || key.isUAZ()) {
        buffer_ += Key::keySymToUTF8(key.sym());
        updateUI(ic);
        event.filterAndAccept();
        return;
    }

    // A digit outside of composing: emit the Devanagari numeral (०-९). While
    // composing, digits select candidates (handled above); in passthrough
    // mode they stay ASCII (handled by the early return).
    if (!composing && key.isDigit()) {
        int d = key.digit();
        if (d >= 0 && d <= 9) {
            static const char* const kDevanagariDigits[] = {
                "०", "१", "२", "३", "४", "५", "६", "७", "८", "९"};
            ic->commitString(kDevanagariDigits[d]);
            event.filterAndAccept();
            return;
        }
    }

    // Anything else (punctuation, standalone digits, etc.): flush the pending
    // word, then let the key through so it appears literally. Punctuation is a
    // phrase boundary, so drop the bigram context.
    if (composing) {
        commitSelected(ic);
    }
    lastCommittedWord_.clear();
}

std::string MarathiEngine::highlightedText(InputContext* ic) const {
    auto candidateList = ic->inputPanel().candidateList();
    if (candidateList && candidateList->cursorIndex() >= 0 &&
        candidateList->cursorIndex() < candidateList->size()) {
        return candidateList->candidate(candidateList->cursorIndex()).text().toString();
    }
    if (candidateList && candidateList->size() > 0) {
        return candidateList->candidate(0).text().toString();
    }
    return transliterator_.transliterate(buffer_, lastCommittedWord_);
}

void MarathiEngine::updateUI(InputContext* ic) {
    if (buffer_.empty()) {
        clearState(ic);
        return;
    }

    auto candidates = transliterator_.generateCandidates(buffer_, lastCommittedWord_);
    // Float the user's previously-chosen readings for this Latin key to the top.
    learn_.rerank(buffer_, candidates);

    topCandidate_ = candidates.empty() ? std::string() : candidates.front();
    ic->inputPanel().setCandidateList(std::make_unique<ScrollingCandidateList>(this, candidates));

    // Preedit shows the currently highlighted candidate (top by default).
    setPreedit(ic, candidates.empty()
                       ? transliterator_.transliterate(buffer_, lastCommittedWord_)
                       : candidates.front());
    ic->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void MarathiEngine::setPreedit(InputContext* ic, const std::string& text) {
    Text preedit(text, TextFormatFlag::Underline);
    preedit.setCursor(static_cast<int>(text.size()));
    ic->inputPanel().setClientPreedit(preedit);
    ic->updatePreedit();
}

void MarathiEngine::commitSelected(InputContext* ic, const std::string& suffix) {
    commitCandidate(ic, highlightedText(ic), suffix);
}

void MarathiEngine::commitCandidate(InputContext* ic, const std::string& text,
                                    const std::string& suffix) {
    // If the user committed something other than the current top candidate,
    // that's an explicit correction — remember it so it ranks first next time.
    if (!buffer_.empty() && !topCandidate_.empty() && text != topCandidate_) {
        learn_.record(buffer_, text);
    }
    ic->commitString(text + suffix);
    lastCommittedWord_ = text; // becomes bigram context for the next word
    clearState(ic);
}

void MarathiEngine::clearState(InputContext* ic) {
    buffer_.clear();
    ic->inputPanel().reset();
    ic->updatePreedit();
    ic->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void MarathiEngine::reset(const InputMethodEntry&, InputContextEvent& event) {
    buffer_.clear();
    lastCommittedWord_.clear();
    auto ic = event.inputContext();
    ic->inputPanel().reset();
    ic->updatePreedit();
    ic->updateUserInterface(UserInterfaceComponent::InputPanel);
}

AddonInstance* MarathiEngineFactory::create(AddonManager* manager) {
    return new MarathiEngine(manager->instance());
}

} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::MarathiEngineFactory)
