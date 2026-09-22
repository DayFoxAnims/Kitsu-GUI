#include "kitsugui/button.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"
#include "internal.h"

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuButton::KitsuButton(const std::string& text)
    : text_(text) {
    autoSize();
}

KitsuButton::~KitsuButton() {
    destroyTextTexture();
}

// ============================================================
// Contenido
// ============================================================
KitsuButton* KitsuButton::text(const std::string& t) {
    if (text_ != t) {
        text_ = t;
        autoSize();
        invalidate();
    }
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuButton* KitsuButton::font(TTF_Font* f) {
    font_ = f;
    invalidate();
    return this;
}

KitsuButton* KitsuButton::corner(float r) {
    corner_ = r;
    invalidate();
    return this;
}

KitsuButton* KitsuButton::padding(int p) {
    padding_ = p;
    autoSize();
    invalidate();
    return this;
}

KitsuButton* KitsuButton::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    invalidate();
    return this;
}

// ============================================================
// Estado
// ============================================================
KitsuButton* KitsuButton::disable() {
    state_ = ButtonState::DISABLED;
    invalidate();
    return this;
}

// ============================================================
// Helpers internos
// ============================================================
TTF_Font* KitsuButton::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuButton::autoSize() {
    // Si el usuario ya dio tamaño explícito, respetarlo
    if (desired_w > 0 && desired_h > 0 && desired_w != 0 && desired_h != 0) {
        // Ya tiene tamaño, no tocar
        // (pero solo si NO venimos de un default 0x0)
    }

    TTF_Font* f = activeFont();
    if (!f || text_.empty()) {
        if (desired_w <= 0) desired_w = 100;
        if (desired_h <= 0) desired_h = KitsuTheme::active().button_height;
        bounds.w = desired_w;
        bounds.h = desired_h;
        return;
    }

    int tw = 0, th = 0;
    Utils::measureText(f, text_, tw, th);

    // padding_ interno del botón (por defecto 16 a cada lado)
    int pad_x = (padding_ > 0) ? padding_ : 16;
    int pad_y = 8;

    desired_w = tw + pad_x * 2;
    desired_h = th + pad_y * 2;

    // Mínimo sensato
    if (desired_h < KitsuTheme::active().button_height) {
        desired_h = KitsuTheme::active().button_height;
    }

    bounds.w = desired_w;
    bounds.h = desired_h;
}

void KitsuButton::destroyTextTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuButton::updateTextTexture(SDL_Renderer* renderer,
                                    Uint8 r, Uint8 g, Uint8 b) {
    TTF_Font* active = activeFont();

    if (text_texture_ &&
        cached_text_ == text_ &&
        cached_font_ == active &&
        cached_r_ == r && cached_g_ == g && cached_b_ == b) {
        return;
    }

    destroyTextTexture();
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
bool KitsuButton::handleEvent(const SDL_Event& e) {
    if (state_ == ButtonState::DISABLED || !visible || !enabled) return false;
    if (bounds.w <= 0 || bounds.h <= 0) return false;

    SDL_Rect r = globalRect();

    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside != mouse_inside_) {
                mouse_inside_ = inside;
                if (inside && state_ == ButtonState::NORMAL) {
                    state_ = ButtonState::HOVER;
                } else if (!inside && (state_ == ButtonState::HOVER ||
                                       state_ == ButtonState::PRESSED)) {
                    state_ = ButtonState::NORMAL;
                }
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
                mouse_inside_ = true;
                state_ = ButtonState::PRESSED;
                invalidate();
                return true;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            if (state_ != ButtonState::PRESSED) break;

            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);

            if (inside) {
                state_ = ButtonState::HOVER;
                invalidate();
                auto cb = on_click_;
                if (cb) cb();
                return true;
            }
            state_ = ButtonState::NORMAL;
            mouse_inside_ = false;
            invalidate();
            break;
        }
    }
    return false;
}

// ============================================================
// Render
// ============================================================
void KitsuButton::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    if (bounds.w <= 0 || bounds.h <= 0) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // ===== Colores según estado =====
    Color bg, border, fg;

    switch (state_) {
        case ButtonState::PRESSED:
            bg     = t.accent_pressed;
            border = t.accent_pressed;
            fg     = t.text_on_accent;
            break;
        case ButtonState::HOVER:
            bg     = t.accent_hover;
            border = t.accent;
            fg     = t.text_on_accent;
            break;
        case ButtonState::DISABLED:
            bg     = t.bg_disabled;
            border = t.border_disabled;
            fg     = t.text_disabled;
            break;
        default: // NORMAL
            bg     = t.bg_secondary;
            border = t.accent;
            fg     = t.text_primary;
            break;
    }

    // ===== Radio del tema o custom =====
    float radius = (corner_ >= 0.0f) ? corner_ : t.radius_button;
    float border_th = (float)t.border_thickness_button;

    // ===== 1. Borde (rect más grande) =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(radius)
        .fill(border)
        .draw(renderer);

    // ===== 2. Relleno (rect interior) =====
    float inner_r = radius - border_th;
    if (inner_r < 0) inner_r = 0;

    KitsuRect((float)r.x + border_th,
              (float)r.y + border_th,
              (float)r.w - 2 * border_th,
              (float)r.h - 2 * border_th)
        .radius(inner_r)
        .fill(bg)
        .draw(renderer);

    // ===== 3. Texto centrado =====
    updateTextTexture(renderer, fg.r, fg.g, fg.b);
    if (text_texture_) {
        SDL_Rect text_rect = {
            r.x + (r.w - tex_w_) / 2,
            r.y + (r.h - tex_h_) / 2,
            tex_w_, tex_h_
        };
        SDL_RenderCopy(renderer, text_texture_, nullptr, &text_rect);
    }

    clearNeedsRender();
}

} // namespace KitsuGui
