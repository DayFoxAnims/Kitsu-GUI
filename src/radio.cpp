#include "kitsugui/radio.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"
#include <algorithm>

namespace KitsuGui {

// ============================================================
// KitsuRadioButton
// ============================================================

KitsuRadioButton::KitsuRadioButton(const std::string& text, bool checked)
    : text_(text), checked_(checked) {
    autoSize();
}

KitsuRadioButton::~KitsuRadioButton() {
    destroyTexture();
}

KitsuRadioButton* KitsuRadioButton::text(const std::string& t) {
    if (text_ != t) {
        text_ = t;
        autoSize();
        invalidate();
    }
    return this;
}

KitsuRadioButton* KitsuRadioButton::checked(bool c) {
    if (checked_ != c) {
        checked_ = c;
        invalidate();
        if (on_change_) on_change_(checked_);
    }
    return this;
}

KitsuRadioButton* KitsuRadioButton::disable() {
    disabled_ = true;
    enabled = false;
    invalidate();
    return this;
}

KitsuRadioButton* KitsuRadioButton::font(TTF_Font* f) {
    font_ = f;
    autoSize();
    invalidate();
    return this;
}

KitsuRadioButton* KitsuRadioButton::size(int w, int h) {
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
TTF_Font* KitsuRadioButton::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuRadioButton::autoSize() {
    if (manual_size_) return;

    KitsuTheme& t = KitsuTheme::active();
    int radio_size = t.radio_size;

    if (text_.empty()) {
        desired_w = radio_size;
        desired_h = radio_size;
        return;
    }

    TTF_Font* f = activeFont();
    if (!f) {
        desired_w = radio_size;
        desired_h = radio_size;
        return;
    }

    int tw = 0, th = 0;
    Utils::measureText(f, text_, tw, th);

    desired_w = radio_size + 8 + tw;
    desired_h = std::max(radio_size, th);

    bounds.w = desired_w;
    bounds.h = desired_h;
}

void KitsuRadioButton::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuRadioButton::updateTextTexture(SDL_Renderer* renderer, Color color) {
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

void KitsuRadioButton::activateFromGroup() {
    if (!checked_) {
        checked_ = true;
        invalidate();
        if (on_change_) on_change_(true);
    }
}

// ============================================================
// Eventos
// ============================================================
bool KitsuRadioButton::handleEvent(const SDL_Event& e) {
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
                if (!checked_) {
                    if (group_) {
                        group_->notifySelected(this);
                    } else {
                        checked(true);
                    }
                }
                invalidate();
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
void KitsuRadioButton::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    const int circle_size = t.radio_size;
    int circle_x = r.x;
    int circle_y = r.y + (r.h - circle_size) / 2;

    float cx = (float)circle_x + circle_size / 2.0f;
    float cy = (float)circle_y + circle_size / 2.0f;
    float outer_r = circle_size / 2.0f - 1.0f;

    Color circle_border;
    Color circle_fill;
    Color dot_color;
    Color text_color;

    if (disabled_) {
        circle_border = t.border_disabled;
        circle_fill   = t.bg_disabled;
        dot_color     = t.text_disabled;
        text_color    = t.text_disabled;
    } else if (checked_) {
        circle_border = t.accent;
        circle_fill   = t.bg_secondary;
        dot_color     = t.accent;
        text_color    = (mouse_inside_ || mouse_down_) ? t.accent : t.text_primary;
    } else if (mouse_inside_ || mouse_down_) {
        circle_border = t.accent;
        circle_fill   = t.bg_secondary;
        dot_color     = t.accent;
        text_color    = t.accent;
    } else {
        circle_border = t.border;
        circle_fill   = t.bg_secondary;
        dot_color     = t.accent;
        text_color    = t.text_primary;
    }

    // ===== 1. Círculo exterior (borde) =====
    KitsuCircle(cx, cy, outer_r)
        .fill(circle_border)
        .draw(renderer);

    // ===== 2. Círculo interior (fondo) =====
    KitsuCircle(cx, cy, outer_r - 2.0f)
        .fill(circle_fill)
        .draw(renderer);

    // ===== 3. Punto central =====
    if (checked_) {
        KitsuCircle(cx, cy, outer_r - 5.0f)
            .fill(dot_color)
            .draw(renderer);
    }

    // ===== 4. Texto =====
    if (!text_.empty()) {
        updateTextTexture(renderer, text_color);
        if (text_texture_ && tex_w_ > 0) {
            int tx = circle_x + circle_size + 8;
            int ty = r.y + (r.h - tex_h_) / 2;
            SDL_Rect dst = { tx, ty, tex_w_, tex_h_ };
            SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);
        }
    }

    clearNeedsRender();
}

// ============================================================
// KitsuRadioGroup
// ============================================================

KitsuRadioGroup::KitsuRadioGroup()
    : KitsuBox(false) {   // vertical
    spacing(6);
    padding(0);
    align(KitsuAlign::START);
    justify(KitsuJustify::START);
}

KitsuRadioGroup::~KitsuRadioGroup() = default;

KitsuRadioGroup* KitsuRadioGroup::add(KitsuRadioButton* radio, bool owns) {
    if (!radio) return this;

    radio->group_ = this;
    radios_.push_back(radio);
    if (owns) owned_.push_back(radio);

    KitsuBox::add(radio, false);   // no owning en la base

    // Si ya viene checked, lo marcamos como seleccionado
    if (radio->isChecked()) {
        if (selected_ && selected_ != radio) {
            selected_->checked(false);
        }
        selected_ = radio;
    }

    invalidate();
    return this;
}

void KitsuRadioGroup::remove(KitsuRadioButton* radio) {
    if (!radio) return;

    auto it = std::find(radios_.begin(), radios_.end(), radio);
    if (it != radios_.end()) radios_.erase(it);

    auto it2 = std::find(owned_.begin(), owned_.end(), radio);
    if (it2 != owned_.end()) {
        owned_.erase(it2);
        KitsuBox::remove(radio);   // lo quita del árbol y lo destruye
    }

    if (selected_ == radio) selected_ = nullptr;
    invalidate();
}

void KitsuRadioGroup::notifySelected(KitsuRadioButton* radio) {
    if (!radio || radio == selected_) return;

    if (selected_ && selected_ != radio) {
        selected_->checked(false);
    }

    selected_ = radio;
    radio->activateFromGroup();

    if (on_change_) on_change_(selected_);
    invalidate();
}

} // namespace KitsuGui
