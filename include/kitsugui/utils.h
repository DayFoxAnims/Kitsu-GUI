#ifndef KITSUGUI_UTILS_H
#define KITSUGUI_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

struct SDL_Renderer;

// ===== Forward declaration correcta de TTF_Font =====
// Coincide con la definición de SDL_ttf.h para evitar conflictos.
struct TTF_Font;
typedef struct TTF_Font TTF_Font;

namespace KitsuGui {
namespace Utils {

// ===== Strings =====
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char delim);
bool startsWith(const std::string& s, const std::string& prefix);
bool endsWith(const std::string& s, const std::string& suffix);
char toLowerAscii(char c);
std::string toLower(const std::string& s);

// ===== Conversión segura =====
int safeStoi(const std::string& s, int fallback = 0);
float safeStof(const std::string& s, float fallback = 0.0f);
bool safeStob(const std::string& s, bool fallback = false);

// ===== Medición de texto =====
// Devuelve el ancho y alto en píxeles del texto con la fuente dada.
// Si font es nullptr, devuelve 0,0.
void measureText(TTF_Font* font, const std::string& text, int& w, int& h);
int textWidth(TTF_Font* font, const std::string& text);
int textHeight(TTF_Font* font);

} // namespace Utils
} // namespace KitsuGui

#endif
