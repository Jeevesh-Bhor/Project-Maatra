#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace fcitx {

// Per-user learning store (Phase 4). Records which Devanagari word the user
// actually chose for a given Latin key, and floats that choice to the top of
// the candidate list next time the same Latin is typed. Append-only log on
// disk so it is crash-safe and needs no rewrite; counts are rebuilt on load.
class LearnStore {
public:
    // Loads (or initializes) the store under the user's data dir.
    LearnStore();

    // Record that `latin` was committed as `devanagari` (a user choice).
    void record(const std::string& latin, const std::string& devanagari);

    // Reorder `candidates` in place so previously-chosen readings for `latin`
    // come first (most-chosen first), preserving the rest of the order.
    void rerank(const std::string& latin, std::vector<std::string>& candidates) const;

    // Best learned choice for `latin`, or empty if none.
    [[nodiscard]] std::string best(const std::string& latin) const;

private:
    // latin -> (devanagari -> times chosen)
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> data_;
    std::string path_;

    void load();
    void appendLine(const std::string& latin, const std::string& devanagari);
};

} // namespace fcitx
