#include "kitsugui/progress_bar.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "internal.h"
#include <cstdio>
#include <algorithm>

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuProgressBar::KitsuProgressBar() {
    desired_w = 250;
    desired_h = 24;
    bounds = {0, 0, 250, 24};
    last_update_ = SDL_GetTicks();
}

KitsuProgressBar::~KitsuProgressBar() {
    destroyTexture();
    if (mode_ == ProgressMode::INDETERMINATE) {
        g_active_animations--;
        if (g_active_animations < 0) g_active_animations = 0;
    }
}

// ============================================================
// Valor
// ============================================================
KitsuProgressBar* KitsuProgressBar::value(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    if (value_ != v) {
        value_ = v;
        invalidate();
    }
    return this;
}

KitsuProgressBar* KitsuProgressBar::value(float v, float max) {
    if (max > 0.0f) value(v / max);
    return this;
}

// ============================================================
// Modo
// ============================================================
KitsuProgressBar* KitsuProgressBar::mode(ProgressMode m) {
    if (mode_ == m) return this;

    // Actualizar contador global
    if (mode_ == ProgressMode::INDETERMINATE) {
        g_active_animations--;
        if (g_active_animations < 0) g_active_animations = 0;
    }
    if (m == ProgressMode::INDETERMINATE) {
        g_active_animations++;
        anim_phase_ = 0.0f;
        last_update_ = SDL_GetTicks();
    }

    mode_ = m;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::indeterminate() {
    return mode(ProgressMode::INDETERMINATE);
}

// ============================================================
// Apariencia
// ============================================================
KitsuProgressBar* KitsuProgressBar::textPosition(ProgressTextPosition p) {
    text_position_ = p;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::showText(bool show) {
    text_position_ = show ? ProgressTextPosition::INSIDE
                          : ProgressTextPosition::NONE;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::suffix(const std::string& s) {
    suffix_ = s;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::colors(const Color& filled,
                                          const Color& empty) {
    track_filled_ = filled;
    track_empty_  = empty;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::textColor(const Color& c) {
    text_color_ = c;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::fillTextColor(const Color& c) {
    fill_text_color_ = c;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::font(TTF_Font* f) {
    font_ = f;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::height(int h) {
    desired_h = h;
    bounds.h = h;
    manual_size_ = true;
    invalidate();
    return this;
}

KitsuProgressBar* KitsuProgressBar::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    invalidate();
    return this;
}

// ============================================================
// Helpers internos
// ============================================================
TTF_Font* KitsuProgressBar::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

std::string KitsuProgressBar::buildText() const {
    if (text_position_ == ProgressTextPosition::NONE) return "";
    if (mode_ == ProgressMode::INDETERMINATE) return "";

    char buf[64];
    snprintf(buf, sizeof(buf), "%.0f%%", value_ * 100.0f);
    return buf;
}

void KitsuProgressBar::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuProgressBar::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = activeFont();
    if (!active || !renderer) return;

    std::string txt = buildText();
    if (txt.empty()) {
        destroyTexture();
        return;
    }

    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;

    if (text_texture_ &&
        cached_text_ == txt &&
        cached_r_ == r && cached_g_ == g && cached_b_ == b) {
        return;
    }

    destroyTexture();

    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, txt.c_str(), fg);
    if (!surface) return;

    text_texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w_ = surface->w;
    tex_h_ = surface->h;
    SDL_FreeSurface(surface);

    cached_text_ = txt;
    cached_r_ = r;
    cached_g_ = g;
    cached_b_ = b;
}

// ============================================================
// Tick
// ============================================================
void KitsuProgressBar::tick() {
    if (mode_ != ProgressMode::INDETERMINATE) return;

    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - last_update_;
    last_update_ = now;

    anim_phase_ += (float)elapsed * 0.0008f;
    while (anim_phase_ > 1.0f) anim_phase_ -= 1.0f;

    invalidate();
}

// ============================================================
// Render
// ============================================================
void KitsuProgressBar::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // Resolver colores
    Color filled_c = (track_filled_.r < 0) ? t.accent       : track_filled_;
    Color empty_c  = (track_empty_.r  < 0) ? t.bg_tertiary  : track_empty_;
    Color tclr     = (text_color_.r   < 0) ? t.text_secondary : text_color_;
    Color ftclr    = (fill_text_color_.r < 0) ? t.text_on_accent : fill_text_color_;

    // ===== 1. Track vacío =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_progress_bar)
        .fill(empty_c)
        .draw(renderer);

    // ===== 2. Calcular rect del fill =====
    SDL_Rect fill = r;

    if (mode_ == ProgressMode::DETERMINATE) {
        fill.w = (int)(r.w * value_);
    } else {
        // Ventana deslizante del 40%
        const float window_size = 0.4f;
        int win_w = (int)(r.w * window_size);
        int pos_x = (int)(r.w * (anim_phase_ * (1.0f + window_size)) - win_w);

        fill.x = r.x + pos_x;
        fill.w = win_w;

        if (fill.x < r.x) {
            fill.w -= (r.x - fill.x);
            fill.x = r.x;
        }
        if (fill.x + fill.w > r.x + r.w) {
            fill.w = r.x + r.w - fill.x;
        }
    }

    // ===== 3. Fill =====
    if (fill.w > 0) {
        KitsuRect((float)fill.x, (float)fill.y,
                  (float)fill.w, (float)fill.h)
            .radius(t.radius_progress_bar)
            .fill(filled_c)
            .draw(renderer);
    }

    // ===== 4. Texto (solo DETERMINATE) =====
    if (mode_ != ProgressMode::DETERMINATE) {
        clearNeedsRender();
        return;
    }

    if (text_position_ == ProgressTextPosition::INSIDE) {
        // Texto "fantasma" gris
        updateTextTexture(renderer, tclr);
        if (text_texture_) {
            SDL_Rect text_rect = {
                r.x + (r.w - tex_w_) / 2,
                r.y + (r.h - tex_h_) / 2,
                tex_w_, tex_h_
            };
            SDL_RenderCopy(renderer, text_texture_, nullptr, &text_rect);

            // Texto encima del fill con clipping
            if (fill.w > 0) {
                SDL_RenderSetClipRect(renderer, &fill);
                destroyTexture();
                updateTextTexture(renderer, ftclr);
                if (text_texture_) {
                    SDL_RenderCopy(renderer, text_texture_, nullptr, &text_rect);
                }
                SDL_RenderSetClipRect(renderer, nullptr);
            }
        }
    }
    else if (text_position_ != ProgressTextPosition::NONE) {
        updateTextTexture(renderer, tclr);
        if (text_texture_) {
            SDL_Rect text_rect;
            switch (text_position_) {
                case ProgressTextPosition::ABOVE:
                    text_rect = { r.x + (r.w - tex_w_) / 2,
                                  r.y - tex_h_ - 2, tex_w_, tex_h_ };
                    break;
                case ProgressTextPosition::BELOW:
                    text_rect = { r.x + (r.w - tex_w_) / 2,
                                  r.y + r.h + 2, tex_w_, tex_h_ };
                    break;
                case ProgressTextPosition::RIGHT:
                    text_rect = { r.x + r.w + 8,
                                  r.y + (r.h - tex_h_) / 2,
                                  tex_w_, tex_h_ };
                    break;
                default:
                    text_rect = { 0, 0, 0, 0 };
            }
            SDL_RenderCopy(renderer, text_texture_, nullptr, &text_rect);
        }
    }

    clearNeedsRender();
}

} // namespace KitsuGui
