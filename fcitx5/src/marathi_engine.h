#pragma once

#include <fcitx/addonfactory.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

#include <string>

#include "learn_store.h"
#include "transliterator.h"

namespace fcitx {

class MarathiEngine final : public InputMethodEngineV2 {
public:
    explicit MarathiEngine(Instance* instance);

    void keyEvent(const InputMethodEntry& entry, KeyEvent& event) override;
    void reset(const InputMethodEntry& entry, InputContextEvent& event) override;

    // Commit a specific candidate string, then clear composing state.
    void commitCandidate(InputContext* ic, const std::string& text);

private:
    // Rebuild the preedit + candidate popup from the current Latin buffer.
    void updateUI(InputContext* ic);
    // Push `text` to the application as the underlined inline preedit.
    void setPreedit(InputContext* ic, const std::string& text);
    // Commit whatever candidate is currently highlighted (or the top one).
    void commitSelected(InputContext* ic);
    void clearState(InputContext* ic);
    [[nodiscard]] std::string highlightedText(InputContext* ic) const;

    Instance* instance_;
    marathi::Transliterator transliterator_;
    LearnStore learn_;               // per-user learned candidate choices
    std::string buffer_;             // accumulated Latin input for the current word
    std::string lastCommittedWord_;  // previous Devanagari word, for bigram context
    bool passthrough_ = false;       // global Latin passthrough mode (Ctrl+Space)
};

class MarathiEngineFactory : public AddonFactory {
public:
    AddonInstance* create(AddonManager* manager) override;
};

} // namespace fcitx
