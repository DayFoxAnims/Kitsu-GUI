#ifndef KITSUGUI_SHAPES_H
#define KITSUGUI_SHAPES_H

#include <SDL2/SDL.h>
#include "kitsugui/color.h"

namespace KitsuGui {

// ===== RECTÁNGULO =====
struct KitsuRect {
    float x = 0, y = 0;
    float w = 0, h = 0;
    float corner_radius = 0;
    Color color = Color(255, 136, 0);
    
    KitsuRect() = default;
    KitsuRect(float x, float y, float w, float h)
        : x(x), y(y), w(w), h(h) {}
    
    // Configuración fluida inline
    KitsuRect& at(float px, float py) { x = px; y = py; return *this; }
    KitsuRect& size(float sw, float sh) { w = sw; h = sh; return *this; }
    KitsuRect& radius(float r) { corner_radius = r; return *this; }
    KitsuRect& fill(const Color& c) { color = c; return *this; }
    
    // Dibujo
    void draw(SDL_Renderer* renderer) const;
    void drawOutline(SDL_Renderer* renderer, float thickness = 1.0f) const;
};

// ===== CÍRCULO =====
struct KitsuCircle {
    float cx = 0, cy = 0;
    float radius = 10;
    Color color = Color(255, 136, 0);
    
    KitsuCircle() = default;
    KitsuCircle(float cx, float cy, float radius)
        : cx(cx), cy(cy), radius(radius) {}
    
    KitsuCircle& at(float px, float py) { cx = px; cy = py; return *this; }
    KitsuCircle& r(float rad) { radius = rad; return *this; }
    KitsuCircle& fill(const Color& c) { color = c; return *this; }
    
    void draw(SDL_Renderer* renderer) const;
    void drawOutline(SDL_Renderer* renderer, float thickness = 1.0f) const;
};

// ===== ESTILOS POR DEFECTO =====
namespace KitsuStyle {
    constexpr float ButtonRadius = 6.0f;
    constexpr float PanelRadius  = 10.0f;
    constexpr float CardRadius   = 8.0f;
    constexpr float DefaultRadius = 0.0f;
}

} // namespace KitsuGui

#endif
