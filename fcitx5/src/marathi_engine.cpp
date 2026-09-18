#include "marathi_engine.h"

#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>

#include <memory>
#include <vector>

namespace fcitx {

namespace {

// A candidate that commits its own Devanagari text back through the engine.
class MarathiCandidate : public CandidateWord {
public:
    MarathiCandidate(MarathiEngine* engine, std::string text)
        : CandidateWord(Text(text)), engine_(engine), value_(std::move(text)) {}

    void select(InputContext* ic) const override {
        engine_->commitCandidate(ic, value_);
    }

private:
    MarathiEngine* engine_;
    std::string value_;
};

constexpr int kPageSize = 9;

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
            if (auto* movable = dynamic_cast<CommonCandidateList*>(candidateList.get())) {
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

        // Number keys 1-9 select the corresponding candidate on the page.
        if (key.isDigit() && candidateList) {
            int idx = key.digit() - 1; // '1' -> 0
            if (idx >= 0 && idx < candidateList->size()) {
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
            commitSelected(ic);
            ic->commitString(" ");
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

    auto list = std::make_unique<CommonCandidateList>();
    list->setPageSize(kPageSize);
    list->setLayoutHint(CandidateLayoutHint::Vertical);
    list->setSelectionKey(Key::keyListFromString("1 2 3 4 5 6 7 8 9"));
    for (const auto& c : candidates) {
        list->append<MarathiCandidate>(this, c);
    }
    if (!candidates.empty()) {
        list->setGlobalCursorIndex(0);
    }
    ic->inputPanel().setCandidateList(std::move(list));

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

void MarathiEngine::commitSelected(InputContext* ic) {
    commitCandidate(ic, highlightedText(ic));
}

void MarathiEngine::commitCandidate(InputContext* ic, const std::string& text) {
    // If the user committed something other than the current top candidate,
    // that's an explicit correction — remember it so it ranks first next time.
    if (!buffer_.empty()) {
        auto cl = ic->inputPanel().candidateList();
        if (cl && cl->size() > 0 && cl->candidate(0).text().toString() != text) {
            learn_.record(buffer_, text);
        }
    }
    ic->commitString(text);
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
