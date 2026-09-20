#include "kitsugui/color.h"
#include <cstring>
#include <algorithm>
#include <cmath>

namespace KitsuGui {

// Constructor desde HEX
Color::Color(const char* hex) {
    // Saltar '#' si existe
    if (hex[0] == '#') hex++;
    
    // Asegurar que tenemos 6 caracteres
    if (strlen(hex) != 6) {
        r = g = b = 0;
        return;
    }
    
    // Convertir cada par de hex a entero
    auto hexToInt = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    };
    
    r = hexToInt(hex[0]) * 16 + hexToInt(hex[1]);
    g = hexToInt(hex[2]) * 16 + hexToInt(hex[3]);
    b = hexToInt(hex[4]) * 16 + hexToInt(hex[5]);
}

// HSV a RGB
Color Color::fromHSV(float h, float s, float v) {
    // Normalizar
    h = fmod(h, 360.0f);
    if (h < 0) h += 360.0f;
    s = std::max(0.0f, std::min(1.0f, s));
    v = std::max(0.0f, std::min(1.0f, v));
    
    float c = v * s;
    float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return Color(
        (int)((r + m) * 255),
        (int)((g + m) * 255),
        (int)((b + m) * 255)
    );
}

} // namespace KitsuGui
