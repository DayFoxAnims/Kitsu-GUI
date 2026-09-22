#include "kitsugui/utils.h"
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cctype>

namespace KitsuGui {
namespace Utils {

// ===== Strings =====
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string current;
    for (char c : s) {
        if (c == delim) {
            std::string t = trim(current);
            if (!t.empty()) out.push_back(t);
            current.clear();
        } else {
            current += c;
        }
    }
    std::string t = trim(current);
    if (!t.empty()) out.push_back(t);
    return out;
}

bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() &&
           s.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

char toLowerAscii(char c) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 'A' && uc <= 'Z') return (char)(uc + 32);
    return c;
}

std::string toLower(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = toLowerAscii(c);
    return out;
}

// ===== Conversión segura =====
int safeStoi(const std::string& s, int fallback) {
    if (s.empty()) return fallback;
    try {
        size_t pos = 0;
        int v = std::stoi(s, &pos);
        if (pos == 0) return fallback;
        return v;
    } catch (...) {
        return fallback;
    }
}

float safeStof(const std::string& s, float fallback) {
    if (s.empty()) return fallback;
    try {
        size_t pos = 0;
        float v = std::stof(s, &pos);
        if (pos == 0) return fallback;
        return v;
    } catch (...) {
        return fallback;
    }
}

bool safeStob(const std::string& s, bool fallback) {
    if (s.empty()) return fallback;
    std::string low = toLower(trim(s));
    if (low == "true"  || low == "1" || low == "yes" || low == "on")  return true;
    if (low == "false" || low == "0" || low == "no"  || low == "off") return false;
    return fallback;
}

// ===== Medición de texto =====
void measureText(TTF_Font* font, const std::string& text, int& w, int& h) {
    w = 0;
    h = 0;
    if (!font || text.empty()) return;
    TTF_SizeUTF8(font, text.c_str(), &w, &h);
}

int textWidth(TTF_Font* font, const std::string& text) {
    int w = 0, h = 0;
    measureText(font, text, w, h);
    return w;
}

int textHeight(TTF_Font* font) {
    if (!font) return 0;
    return TTF_FontHeight(font);
}

} // namespace Utils
} // namespace KitsuGui
