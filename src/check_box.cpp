#include "kitsugui/check_box.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuCheckBox::KitsuCheckBox(const std::string& text, bool checked)
    : text_(text), checked_(checked) {
    autoSize();
}

KitsuCheckBox::~KitsuCheckBox() {
    destroyTexture();
}

// ============================================================
// Contenido
// ============================================================
KitsuCheckBox* KitsuCheckBox::text(const std::string& t) {
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
KitsuCheckBox* KitsuCheckBox::checked(bool c) {
    if (checked_ != c) {
        checked_ = c;
        invalidate();
        if (on_change_) on_change_(checked_);
    }
    return this;
}

KitsuCheckBox* KitsuCheckBox::toggle() {
    return checked(!checked_);
}

KitsuCheckBox* KitsuCheckBox::disable() {
    disabled_ = true;
    enabled = false;
    invalidate();
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuCheckBox* KitsuCheckBox::font(TTF_Font* f) {
    font_ = f;
    autoSize();
    invalidate();
    return this;
}

KitsuCheckBox* KitsuCheckBox::size(int w, int h) {
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
TTF_Font* KitsuCheckBox::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuCheckBox::autoSize() {
	if (manual_size_) return;
    TTF_Font* f = activeFont();
    if (!f) {
        desired_w = 24;
        desired_h = KitsuTheme::active().checkbox_size;
        return;
    }

    int box_size = KitsuTheme::active().checkbox_size;

    if (text_.empty()) {
        desired_w = box_size;
        desired_h = box_size;
        return;
    }

    int tw = 0, th = 0;
    Utils::measureText(f, text_, tw, th);

    // Layout: [box] + 8px gap + [texto]
    desired_w = box_size + 8 + tw;
    desired_h = std::max(box_size, th);

    bounds.w = desired_w;
    bounds.h = desired_h;
}

void KitsuCheckBox::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuCheckBox::updateTextTexture(SDL_Renderer* renderer, Color color) {
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
bool KitsuCheckBox::handleEvent(const SDL_Event& e) {
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
void KitsuCheckBox::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // ===== 1. Cuadrado del check =====
    const int box_size = t.checkbox_size;
    int box_x = r.x;
    int box_y = r.y + (r.h - box_size) / 2;

    Color box_border;
    Color box_fill;
    Color check_color;
    Color text_color;

    if (disabled_) {
        box_border = t.border_disabled;
        box_fill   = t.bg_disabled;
        check_color = t.text_disabled;
        text_color  = t.text_disabled;
    } else if (checked_) {
        box_border = t.accent;
        box_fill   = t.accent;
        check_color = t.text_on_accent;
        text_color  = (mouse_inside_ || mouse_down_) ? t.accent : t.text_primary;
    } else if (mouse_inside_ || mouse_down_) {
        box_border = t.accent;
        box_fill   = t.bg_tertiary;
        check_color = t.accent;
        text_color  = t.accent;
    } else {
        box_border = t.border;
        box_fill   = t.bg_secondary;
        check_color = t.text_primary;
        text_color  = t.text_primary;
    }

    // Fondo del cuadrado
    KitsuRect((float)box_x, (float)box_y,
              (float)box_size, (float)box_size)
        .radius(t.radius_checkbox)
        .fill(box_fill)
        .draw(renderer);

    // Borde
    KitsuRect((float)box_x, (float)box_y,
              (float)box_size, (float)box_size)
        .radius(t.radius_checkbox)
        .fill(box_border)
        .drawOutline(renderer, (float)t.border_thickness_checkbox);

    // ===== 2. Check mark =====
    if (checked_) {
        int cx = box_x + box_size / 2;
        int cy = box_y + box_size / 2;

        SDL_SetRenderDrawColor(renderer,
            (Uint8)check_color.r, (Uint8)check_color.g,
            (Uint8)check_color.b, 255);

        // Tick (dos líneas)
        int tick_size = box_size / 3;
        for (int thick = -1; thick <= 1; thick++) {
            SDL_RenderDrawLine(renderer,
                cx - tick_size, cy + thick,
                cx - 1, cy + tick_size + thick);
        }
        for (int thick = -1; thick <= 1; thick++) {
            SDL_RenderDrawLine(renderer,
                cx - 1, cy + tick_size + thick,
                cx + tick_size, cy - tick_size + thick);
        }
    }

    // ===== 3. Texto =====
    if (!text_.empty()) {
        updateTextTexture(renderer, text_color);
        if (text_texture_ && tex_w_ > 0) {
            int tx = box_x + box_size + 8;
            int ty = r.y + (r.h - tex_h_) / 2;
            SDL_Rect dst = { tx, ty, tex_w_, tex_h_ };
            SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);
        }
    }

    clearNeedsRender();
}

} // namespace KitsuGui
