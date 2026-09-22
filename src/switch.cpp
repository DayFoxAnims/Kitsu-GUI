#include "kitsugui/switch.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"
#include <algorithm>

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuSwitch::KitsuSwitch(const std::string& text, bool checked)
    : text_(text), checked_(checked) {
    autoSize();
}

KitsuSwitch::~KitsuSwitch() {
    destroyTexture();
}

// ============================================================
// Contenido
// ============================================================
KitsuSwitch* KitsuSwitch::text(const std::string& t) {
    if (text_ != t) {
        text_ = t;
        autoSize();
        invalidate();
    }
    return this;
}

// ============================================================
// Estado
// ============================================================
KitsuSwitch* KitsuSwitch::checked(bool c) {
    if (checked_ != c) {
        checked_ = c;
        invalidate();
        if (on_change_) on_change_(checked_);
    }
    return this;
}

KitsuSwitch* KitsuSwitch::toggle() {
    return checked(!checked_);
}

KitsuSwitch* KitsuSwitch::disable() {
    disabled_ = true;
    enabled = false;
    invalidate();
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuSwitch* KitsuSwitch::font(TTF_Font* f) {
    font_ = f;
    autoSize();
    invalidate();
    return this;
}

KitsuSwitch* KitsuSwitch::size(int w, int h) {
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
TTF_Font* KitsuSwitch::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuSwitch::autoSize() {
	if (manual_size_) return;
    KitsuTheme& t = KitsuTheme::active();
    int sw_w = t.switch_width;
    int sw_h = t.switch_height;

    if (text_.empty()) {
        desired_w = sw_w;
        desired_h = sw_h;
        return;
    }

    TTF_Font* f = activeFont();
    if (!f) {
        desired_w = sw_w;
        desired_h = sw_h;
        return;
    }

    int tw = 0, th = 0;
    Utils::measureText(f, text_, tw, th);

    // Layout: [switch] + 10px gap + [texto]
    desired_w = sw_w + 10 + tw;
    desired_h = std::max(sw_h, th);

    bounds.w = desired_w;
    bounds.h = desired_h;
}

void KitsuSwitch::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuSwitch::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = activeFont();
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;

    if (text_texture_ &&
        cached_text_ == text_ &&
        cached_font_ == active &&
        cached_r_ == r && cached_g_ == g && cached_b_ == b) {
        return;
    }

    destroyTexture();
    if (!active || text_.empty() || !renderer) return;

    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, text_.c_str(), fg);
    if (!surface) return;

    text_texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w_ = surface->w;
    tex_h_ = surface->h;
    SDL_FreeSurface(surface);

    cached_text_ = text_;
    cached_font_ = active;
    cached_r_ = r;
    cached_g_ = g;
    cached_b_ = b;
}

// ============================================================
// Eventos
// ============================================================
bool KitsuSwitch::handleEvent(const SDL_Event& e) {
    if (disabled_ || !visible || !enabled) return false;

    SDL_Rect r = globalRect();

    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside != mouse_inside_) {
                mouse_inside_ = inside;
                invalidate();
            }
            return inside;
        }

        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside) {
                mouse_down_ = true;
                invalidate();
                return true;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);

            bool was_down = mouse_down_;
            mouse_down_ = false;

            if (was_down && inside) {
                toggle();
                return true;
            }
            invalidate();
            break;
        }
    }
    return false;
}

// ============================================================
// Render
// ============================================================
void KitsuSwitch::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    const int sw_w = t.switch_width;
    const int sw_h = t.switch_height;

    int track_x = r.x;
    int track_y = r.y + (r.h - sw_h) / 2;

    Color track_border;
    Color track_fill;
    Color knob_color;
    Color text_color;

    if (disabled_) {
        track_border = t.border_disabled;
        track_fill   = t.bg_disabled;
        knob_color   = t.text_disabled;
        text_color   = t.text_disabled;
    } else if (checked_) {
        track_border = t.accent;
        track_fill   = t.accent;
        knob_color   = t.text_on_accent;
        text_color   = (mouse_inside_ || mouse_down_) ? t.accent : t.text_primary;
    } else if (mouse_inside_ || mouse_down_) {
        track_border = t.accent;
        track_fill   = t.bg_tertiary;
        knob_color   = t.accent;
        text_color   = t.accent;
    } else {
        track_border = t.border;
        track_fill   = t.bg_tertiary;
        knob_color   = t.border;
        text_color   = t.text_primary;
    }

    // ===== 1. Track =====
    KitsuRect((float)track_x, (float)track_y,
              (float)sw_w, (float)sw_h)
        .radius(t.radius_switch)
        .fill(track_fill)
        .draw(renderer);

    KitsuRect((float)track_x, (float)track_y,
              (float)sw_w, (float)sw_h)
        .radius(t.radius_switch)
        .fill(track_border)
        .drawOutline(renderer, (float)t.border_thickness_switch);

    // ===== 2. Knob =====
    const int pad = t.switch_knob_pad;
    int knob_w = (sw_w / 2) - 2 * pad;
    int knob_h = sw_h - 2 * pad;

    int knob_x = checked_
        ? track_x + sw_w - knob_w - pad
        : track_x + pad;
    int knob_y = track_y + pad;

    KitsuRect((float)knob_x, (float)knob_y,
              (float)knob_w, (float)knob_h)
        .radius(t.radius_switch)
        .fill(knob_color)
        .draw(renderer);

    // ===== 3. Texto =====
    if (!text_.empty()) {
        updateTextTexture(renderer, text_color);
        if (text_texture_ && tex_w_ > 0) {
            int tx = track_x + sw_w + 10;
            int ty = r.y + (r.h - tex_h_) / 2;
            SDL_Rect dst = { tx, ty, tex_w_, tex_h_ };
            SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);
        }
    }

    clearNeedsRender();
}

} // namespace KitsuGui
