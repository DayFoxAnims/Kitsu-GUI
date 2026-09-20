#include "kitsugui/shapes.h"
#include <SDL2/SDL2_gfxPrimitives.h>

namespace KitsuGui {

// ==================================================================
// KitsuRect
// ==================================================================

void KitsuRect::draw(SDL_Renderer* renderer) const {
    if (!renderer || w <= 0 || h <= 0) return;
    
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;
    Uint8 a = (Uint8)color.a;
    
    // Coordenadas enteras (SDL2_gfx usa Sint16)
    Sint16 x1 = (Sint16)x;
    Sint16 y1 = (Sint16)y;
    Sint16 x2 = (Sint16)(x + w - 1);
    Sint16 y2 = (Sint16)(y + h - 1);
    
    if (corner_radius < 1.0f) {
        // Rectángulo cuadrado simple
        boxRGBA(renderer, x1, y1, x2, y2, r, g, b, a);
    } else {
        // Rectángulo redondeado con anti-aliasing
        roundedBoxRGBA(renderer, x1, y1, x2, y2, (Sint16)corner_radius, r, g, b, a);
    }
}

void KitsuRect::drawOutline(SDL_Renderer* renderer, float thickness) const {
    if (!renderer || w <= 0 || h <= 0) return;
    
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;
    Uint8 a = (Uint8)color.a;
    
    Sint16 x1 = (Sint16)x;
    Sint16 y1 = (Sint16)y;
    Sint16 x2 = (Sint16)(x + w - 1);
    Sint16 y2 = (Sint16)(y + h - 1);
    
    if (corner_radius < 1.0f) {
        // Contorno cuadrado (varias líneas para grosor)
        for (int i = 0; i < (int)thickness; i++) {
            rectangleRGBA(renderer, x1 + i, y1 + i, x2 - i, y2 - i, r, g, b, a);
        }
    } else {
        // Contorno redondeado con AA
        roundedRectangleRGBA(renderer, x1, y1, x2, y2, (Sint16)corner_radius, r, g, b, a);
    }
}

// ==================================================================
// KitsuCircle
// ==================================================================

void KitsuCircle::draw(SDL_Renderer* renderer) const {
    if (!renderer || radius <= 0) return;
    
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;
    Uint8 a = (Uint8)color.a;
    
    Sint16 cx_i = (Sint16)cx;
    Sint16 cy_i = (Sint16)cy;
    Sint16 rad_i = (Sint16)radius;
    
    // filledCircleRGBA acepta RGBA directamente (sin problema de endianness)
    filledCircleRGBA(renderer, cx_i, cy_i, rad_i, r, g, b, a);
}

void KitsuCircle::drawOutline(SDL_Renderer* renderer, float thickness) const {
    if (!renderer || radius <= 0) return;
    
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;
    Uint8 a = (Uint8)color.a;
    
    Sint16 cx_i = (Sint16)cx;
    Sint16 cy_i = (Sint16)cy;
    Sint16 rad_i = (Sint16)radius;
    
    if (thickness <= 1.0f) {
        // Contorno simple con AA
        aacircleRGBA(renderer, cx_i, cy_i, rad_i, r, g, b, a);
    } else {
        // Anillo (varios círculos concéntricos)
        for (int i = 0; i < (int)thickness; i++) {
            aacircleRGBA(renderer, cx_i, cy_i, rad_i - i, r, g, b, a);
        }
    }
}

} // namespace KitsuGui
