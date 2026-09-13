#include <iostream>
#include <sstream>
#include <string>

#include "transliterator.h"

// Reads whitespace-separated Latin words per line, transliterates each using the
// previous word's Devanagari as bigram context, and prints the joined result.
int main() {
    marathi::Transliterator t;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string word;
        std::string prev;
        std::string out;
        bool first = true;
        while (iss >> word) {
            std::string dev = t.transliterate(word, prev);
            if (!first) out += ' ';
            out += dev;
            prev = dev;
            first = false;
        }
        std::cout << out << std::endl;
    }
    return 0;
}
