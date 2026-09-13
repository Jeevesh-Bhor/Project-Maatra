#include "learn_store.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fcitx {

namespace {
std::string dataDir() {
    // Follows XDG: $XDG_DATA_HOME/fcitx5/marathitranslit, else ~/.local/share/...
    if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && *xdg) {
        return std::string(xdg) + "/fcitx5/marathitranslit";
    }
    const char* home = std::getenv("HOME");
    return std::string(home ? home : ".") + "/.local/share/fcitx5/marathitranslit";
}
} // namespace

LearnStore::LearnStore() {
    std::string dir = dataDir();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    path_ = dir + "/learned.tsv";
    load();
}

void LearnStore::load() {
    std::ifstream in(path_);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        auto tab = line.find('\t');
        if (tab == std::string::npos) continue;
        std::string latin = line.substr(0, tab);
        std::string dev = line.substr(tab + 1);
        if (latin.empty() || dev.empty()) continue;
        ++data_[latin][dev]; // each logged line is one selection
    }
}

void LearnStore::appendLine(const std::string& latin, const std::string& devanagari) {
    std::ofstream out(path_, std::ios::app);
    if (!out) return;
    out << latin << '\t' << devanagari << '\n';
}

void LearnStore::record(const std::string& latin, const std::string& devanagari) {
    if (latin.empty() || devanagari.empty()) return;
    ++data_[latin][devanagari];
    appendLine(latin, devanagari);
}

std::string LearnStore::best(const std::string& latin) const {
    auto it = data_.find(latin);
    if (it == data_.end() || it->second.empty()) return {};
    const std::string* bestDev = nullptr;
    uint32_t bestCount = 0;
    for (const auto& [dev, count] : it->second) {
        if (count > bestCount) {
            bestCount = count;
            bestDev = &dev;
        }
    }
    return bestDev ? *bestDev : std::string{};
}

void LearnStore::rerank(const std::string& latin, std::vector<std::string>& candidates) const {
    auto it = data_.find(latin);
    if (it == data_.end() || candidates.empty()) return;

    // Learned readings, most-chosen first.
    std::vector<std::pair<std::string, uint32_t>> learned(it->second.begin(), it->second.end());
    std::sort(learned.begin(), learned.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Move each learned reading to the front (in order), keeping the rest as-is.
    // A learned reading not already in the list is prepended too (the user chose
    // it before, so it's clearly relevant).
    for (auto rit = learned.rbegin(); rit != learned.rend(); ++rit) {
        const std::string& dev = rit->first;
        auto found = std::find(candidates.begin(), candidates.end(), dev);
        if (found != candidates.end()) {
            candidates.erase(found);
        }
        candidates.insert(candidates.begin(), dev);
    }
}

} // namespace fcitx
