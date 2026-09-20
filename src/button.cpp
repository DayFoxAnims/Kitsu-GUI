#include "kitsugui/button.h"
#include "kitsugui/shapes.h"
#include "internal.h"

namespace KitsuGui {

TTF_Font* KitsuButton::g_font = nullptr;

void KitsuButton::setFont(TTF_Font* font) { g_font = font; }
TTF_Font* KitsuButton::getFont() { return g_font; }

KitsuButton::KitsuButton(const std::string& text, int width, int height)
    : text(text) {
    bounds.w = width;
    bounds.h = height;
    requested_w = width;
    requested_h = height;
}

KitsuButton::~KitsuButton() {
    if (text_texture) SDL_DestroyTexture(text_texture);
}

KitsuButton& KitsuButton::disabled() {
    state = ButtonState::DISABLED;
    markDirty();
    return *this;
}

KitsuButton& KitsuButton::withCallback(std::function<void()> cb) {
    callback = cb;
    return *this;
}

KitsuButton& KitsuButton::withTheme(Theme t) {
    theme = t;
    markDirty();
    return *this;
}

KitsuButton& KitsuButton::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuButton& KitsuButton::withCorner(float radius) {
    corner_radius = radius;
    markDirty();
    return *this;
}

void KitsuButton::setState(ButtonState s) {
    if (state != s) {
        state = s;
        if (parent) parent->markDirty();
        markDirty();
        if (g_renderer) {
            g_renderer->markAllDirty();   // ← FORZAR TODO
        }
    }
}

bool KitsuButton::handleEvent(const SDL_Event& e) {
    if (state == ButtonState::DISABLED) return false;
    if (bounds.w <= 0 || bounds.h <= 0) return false;
    
    SDL_Rect abs = getAbsoluteBounds();
    
    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (inside != mouse_inside) {
                mouse_inside = inside;
                if (inside && state == ButtonState::NORMAL) {
                    setState(ButtonState::HOVER);
                } else if (!inside && (state == ButtonState::HOVER ||
                                       state == ButtonState::PRESSED)) {
                    setState(ButtonState::NORMAL);
                }
                // FIX: forzar redibujado del renderer
                markDirty();
            }
            return inside;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            if (inside) {
                mouse_inside = true;
                setState(ButtonState::PRESSED);
                markDirty();
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            if (state == ButtonState::PRESSED) {
                int mx = e.button.x, my = e.button.y;
                bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                               my >= abs.y && my < abs.y + abs.h);
                if (inside) {
                    setState(ButtonState::HOVER);
                    markDirty();
                    auto cb = callback;
                    if (cb) cb();
                    return true;
                } else {
                    setState(ButtonState::NORMAL);
                    markDirty();
                    mouse_inside = false;
                }
            }
            break;
        }
    }
    return false;
}

void KitsuButton::getColorsForState(Uint8& r, Uint8& g, Uint8& b,
                                    Uint8& br, Uint8& bg, Uint8& bb,
                                    Uint8& tr, Uint8& tg, Uint8& tb) const {
    KitsuTheme& theme = KitsuTheme::current();
    
    if (state == ButtonState::PRESSED) {
        // Pressed: fondo = accent, borde = accent, texto = on_accent
        r = theme.accent_pressed.r; g = theme.accent_pressed.g; b = theme.accent_pressed.b;
        br = theme.accent_pressed.r; bg = theme.accent_pressed.g; bb = theme.accent_pressed.b;
        tr = theme.text_on_accent.r; tg = theme.text_on_accent.g; tb = theme.text_on_accent.b;
        return;
    }
    
    // Borde = accent siempre (excepto disabled)
    br = theme.accent.r; bg = theme.accent.g; bb = theme.accent.b;
    
    if (state == ButtonState::HOVER) {
        r = theme.accent_hover.r; g = theme.accent_hover.g; b = theme.accent_hover.b;
        tr = theme.text_on_accent.r; tg = theme.text_on_accent.g; tb = theme.text_on_accent.b;
    }
    else if (state == ButtonState::DISABLED) {
        r = theme.bg_disabled.r; g = theme.bg_disabled.g; b = theme.bg_disabled.b;
        br = theme.border_disabled.r; bg = theme.border_disabled.g; bb = theme.border_disabled.b;
        tr = theme.text_disabled.r; tg = theme.text_disabled.g; tb = theme.text_disabled.b;
    }
    else {
        // NORMAL
        r = theme.bg_secondary.r; g = theme.bg_secondary.g; b = theme.bg_secondary.b;
        tr = theme.text_primary.r; tg = theme.text_primary.g; tb = theme.text_primary.b;
    }
}

void KitsuButton::updateTextTexture(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b) {
    TTF_Font* active = getActiveFont();
    
    if (text_texture &&
        cached_text == text &&
        cached_text_r == r && cached_text_g == g && cached_text_b == b &&
        cached_font == active) {
        return;
    }
    
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
    }
    
    if (!active || text.empty() || !renderer) return;
    
    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, text.c_str(), fg);
    if (!surface) return;
    
    text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    cached_text = text;
    cached_text_r = r; cached_text_g = g; cached_text_b = b;
    cached_font = active;
}

void KitsuButton::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    if (bounds.w <= 0 || bounds.h <= 0) return;
    
    SDL_Rect abs = getRenderBounds();
    
    Uint8 r, g, b, br, bg, bb, tr, tg, tb;
    getColorsForState(r, g, b, br, bg, bb, tr, tg, tb);
    
    const float border_thickness = 3.0f;
    
    // 1. Borde: rectángulo redondeado del color del borde
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(corner_radius)
        .fill(Color(br, bg, bb, 255))
        .draw(renderer);
    
    // 2. Fondo: rectángulo redondeado encima
    float inner_radius = corner_radius - border_thickness;
    if (inner_radius < 0) inner_radius = 0;
    
    KitsuRect((float)abs.x + border_thickness, (float)abs.y + border_thickness,
              (float)abs.w - 2 * border_thickness, (float)abs.h - 2 * border_thickness)
        .radius(inner_radius)
        .fill(Color(r, g, b, 255))
        .draw(renderer);
    
    // 3. Texto centrado
    updateTextTexture(renderer, tr, tg, tb);
    if (text_texture) {
        int tex_w = 0, tex_h = 0;
        SDL_QueryTexture(text_texture, nullptr, nullptr, &tex_w, &tex_h);
        
        SDL_Rect text_rect = {
            abs.x + (abs.w - tex_w) / 2,
            abs.y + (abs.h - tex_h) / 2,
            tex_w, tex_h
        };
        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    }
    
    clearDirty();
}

} // namespace KitsuGui
