#include "kitsugui/progress_bar.h"
#include "kitsugui/widget.h"
#include <cstdio>
#include <cmath>

namespace KitsuGui {

TTF_Font* KitsuProgressBar::g_font = nullptr;

KitsuProgressBar::KitsuProgressBar() {
    bounds = {0, 0, 250, 24};
    requested_w = 250;
    requested_h = 24;
    last_update = SDL_GetTicks();
}

KitsuProgressBar::~KitsuProgressBar() {
    destroyTextTexture();
    if (mode == ProgressMode::INDETERMINATE) {
        g_active_animations--;
        if (g_active_animations < 0) g_active_animations = 0;
    }
}

void KitsuProgressBar::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

// ===== CONFIGURACIÓN FLUIDA =====
KitsuProgressBar& KitsuProgressBar::withMode(ProgressMode m) {
    if (mode == m) return *this;
    
    // Actualizar contador global
    if (mode == ProgressMode::INDETERMINATE) {
        g_active_animations--;
        if (g_active_animations < 0) g_active_animations = 0;
    }
    if (m == ProgressMode::INDETERMINATE) {
        g_active_animations++;
        anim_phase = 0.0f;
        last_update = SDL_GetTicks();
    }
    
    mode = m;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withValue(float v) {
    setValue(v);
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withValue(float v, float max) {
    if (max > 0) setValue(v / max);
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withTextPosition(ProgressTextPosition p) {
    text_position = p;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withTextColor(const Color& c) {
    text_color = c;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withFillTextColor(const Color& c) {
    fill_text_color = c;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withTrackColors(const Color& filled, const Color& empty) {
    track_filled = filled;
    track_empty = empty;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::withHeight(int h) {
    bounds.h = h;
    requested_h = h;
    markDirty();
    return *this;
}

KitsuProgressBar& KitsuProgressBar::setBounds(int x, int y, int w, int h) {
    setBoundsInternal(x, y, w, h);
    return *this;
}

// ===== VALOR =====
void KitsuProgressBar::setValue(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    
    if (value != v) {
        value = v;
        markDirty();
    }
}

// ===== TICK (llamado cada frame por run.cpp) =====
void KitsuProgressBar::tick() {
    if (mode != ProgressMode::INDETERMINATE) return;
    
    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - last_update;
    last_update = now;
    
    // Avanza el phase
    anim_phase += (float)elapsed * 0.0008f;   // ~0.8 por segundo
    while (anim_phase > 1.0f) anim_phase -= 1.0f;
    
    // Marca dirty para forzar redibujado
    markDirty();
}

// ===== CONSTRUIR EL TEXTO =====
std::string KitsuProgressBar::buildText() const {
    if (text_position == ProgressTextPosition::NONE) return "";
    if (mode == ProgressMode::INDETERMINATE) return "";
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%.0f%%", value * 100.0f);
    return buf;
}

// ===== TEXTURA DEL TEXTO =====
void KitsuProgressBar::updateTextTexture(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b) {
    TTF_Font* active = getActiveFont();
    if (!active) return;
    
    std::string text = buildText();
    if (text.empty()) {
        destroyTextTexture();
        return;
    }
    
    if (text_texture &&
        cached_text == text &&
        cached_r == r && cached_g == g && cached_b == b) {
        return;
    }
    
    destroyTextTexture();
    
    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, text.c_str(), fg);
    if (!surface) return;
    
    text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w = surface->w;
    tex_h = surface->h;
    SDL_FreeSurface(surface);
    
    cached_text = text;
    cached_r = r; cached_g = g; cached_b = b;
}

// ===== RENDER =====
void KitsuProgressBar::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    
    // Track vacío (fondo)
    SDL_SetRenderDrawColor(renderer,
        (Uint8)track_empty.r, (Uint8)track_empty.g, (Uint8)track_empty.b, 255);
    SDL_RenderFillRect(renderer, &abs);
    
    // Calcular rect del fill
    SDL_Rect fill = abs;
    
    if (mode == ProgressMode::DETERMINATE) {
        fill.w = (int)(abs.w * value);
    } else {
        // Ventana deslizante del 40% del ancho
        const float window_size = 0.4f;
        float phase = anim_phase;
        
        int win_w = (int)(abs.w * window_size);
        int pos_x = (int)(abs.w * (phase * (1.0f + window_size)) - win_w);
        
        fill.x = abs.x + pos_x;
        fill.w = win_w;
        
        // Clippear al track
        if (fill.x < abs.x) {
            fill.w -= (abs.x - fill.x);
            fill.x = abs.x;
        }
        if (fill.x + fill.w > abs.x + abs.w) {
            fill.w = abs.x + abs.w - fill.x;
        }
    }
    
    // Dibujar fill
    if (fill.w > 0) {
        SDL_SetRenderDrawColor(renderer,
            (Uint8)track_filled.r, (Uint8)track_filled.g, (Uint8)track_filled.b, 255);
        SDL_RenderFillRect(renderer, &fill);
    }
    
    // Texto (solo en modo DETERMINATE)
    if (mode == ProgressMode::DETERMINATE &&
        text_position == ProgressTextPosition::INSIDE) {
        std::string text = buildText();
        if (!text.empty()) {
            // Primero gris (sobre track vacío)
            updateTextTexture(renderer,
                (Uint8)text_color.r, (Uint8)text_color.g, (Uint8)text_color.b);
            
            if (text_texture) {
                SDL_Rect text_rect = {
                    abs.x + (abs.w - tex_w) / 2,
                    abs.y + (abs.h - tex_h) / 2,
                    tex_w, tex_h
                };
                SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
                
                // Luego blanco (sobre fill), con clipping
                if (fill.w > 0) {
                    SDL_RenderSetClipRect(renderer, &fill);
                    
                    destroyTextTexture();
                    updateTextTexture(renderer,
                        (Uint8)fill_text_color.r, (Uint8)fill_text_color.g, (Uint8)fill_text_color.b);
                    
                    if (text_texture) {
                        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
                    }
                    
                    SDL_RenderSetClipRect(renderer, nullptr);
                }
            }
        }
    }
    else if (mode == ProgressMode::DETERMINATE &&
             text_position != ProgressTextPosition::NONE &&
             text_position != ProgressTextPosition::INSIDE) {
        updateTextTexture(renderer,
            (Uint8)text_color.r, (Uint8)text_color.g, (Uint8)text_color.b);
        
        if (text_texture) {
            SDL_Rect text_rect;
            switch (text_position) {
                case ProgressTextPosition::ABOVE:
                    text_rect = { abs.x + (abs.w - tex_w) / 2, abs.y - tex_h - 2, tex_w, tex_h };
                    break;
                case ProgressTextPosition::BELOW:
                    text_rect = { abs.x + (abs.w - tex_w) / 2, abs.y + abs.h + 2, tex_w, tex_h };
                    break;
                case ProgressTextPosition::RIGHT:
                    text_rect = { abs.x + abs.w + 8, abs.y + (abs.h - tex_h) / 2, tex_w, tex_h };
                    break;
                default:
                    text_rect = { 0, 0, 0, 0 };
            }
            SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
        }
    }
    
    clearDirty();
}

} // namespace KitsuGui
