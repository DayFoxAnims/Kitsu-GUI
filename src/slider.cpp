#include "kitsugui/slider.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuSlider::KitsuSlider(float min, float max, float value)
    : min_(min), max_(max), value_(value) {
    autoSize();
}

KitsuSlider::~KitsuSlider() {
    destroyNumberTexture();
}

// ============================================================
// Valor
// ============================================================
KitsuSlider* KitsuSlider::value(float v) {
    if (v < min_) v = min_;
    if (v > max_) v = max_;
    if (step_ > 0.0f) v = roundf(v / step_) * step_;

    if (value_ != v) {
        value_ = v;
        invalidate();
        if (on_change_) on_change_(value_);
    }
    return this;
}

KitsuSlider* KitsuSlider::range(float min, float max) {
    min_ = min;
    max_ = max;
    if (value_ < min_) value_ = min_;
    if (value_ > max_) value_ = max_;
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::step(float s) {
    step_ = s;
    invalidate();
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuSlider* KitsuSlider::orientation(SliderOrientation o) {
    orientation_ = o;
    autoSize();
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::showNumber(bool show) {
    show_number_ = show;
    autoSize();
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::suffix(const std::string& s) {
    suffix_ = s;
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::decimals(int d) {
    decimals_ = d;
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::font(TTF_Font* f) {
    font_ = f;
    autoSize();
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    invalidate();
    return this;
}

KitsuSlider* KitsuSlider::disable() {
    disabled_ = true;
    enabled = false;
    invalidate();
    return this;
}

// ============================================================
// Helpers internos
// ============================================================
TTF_Font* KitsuSlider::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuSlider::autoSize() {
    KitsuTheme& t = KitsuTheme::active();
    int knob = t.slider_knob_size;

    if (orientation_ == SliderOrientation::HORIZONTAL) {
        desired_w = 250;
        desired_h = std::max(knob, KitsuFonts::normal()
                             ? TTF_FontHeight(KitsuFonts::normal()) : 16);
    } else {
        desired_w = std::max(knob, 24);
        desired_h = 200;
    }
}

void KitsuSlider::destroyNumberTexture() {
    if (number_texture_) {
        SDL_DestroyTexture(number_texture_);
        number_texture_ = nullptr;
        num_w_ = num_h_ = 0;
    }
}

void KitsuSlider::updateNumberTexture(SDL_Renderer* renderer, Color color) {
    if (!show_number_) return;

    TTF_Font* active = activeFont();
    if (!active || !renderer) return;

    char buf[64];
    snprintf(buf, sizeof(buf), "%.*f%s",
             decimals_, value_, suffix_.c_str());

    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;

    if (number_texture_ &&
        cached_number_ == buf &&
        cached_r_ == r && cached_g_ == g && cached_b_ == b) {
        return;
    }

    destroyNumberTexture();

    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, buf, fg);
    if (!surface) return;

    number_texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    num_w_ = surface->w;
    num_h_ = surface->h;
    SDL_FreeSurface(surface);

    cached_number_ = buf;
    cached_r_ = r;
    cached_g_ = g;
    cached_b_ = b;
}

// ============================================================
// Geometría
// ============================================================
SDL_Rect KitsuSlider::trackRect(int abs_x, int abs_y,
                                int abs_w, int abs_h,
                                int num_w) const {
    KitsuTheme& t = KitsuTheme::active();
    int track_th = t.slider_track_thickness;
    int gap = 12;

    SDL_Rect tr;
    if (orientation_ == SliderOrientation::HORIZONTAL) {
        int usable_w = abs_w - (show_number_ ? (num_w + gap) : 0);
        tr.x = abs_x;
        tr.y = abs_y + (abs_h - track_th) / 2;
        tr.w = usable_w;
        tr.h = track_th;
    } else {
        int usable_h = abs_h - (show_number_ ? (num_h_ + gap) : 0);
        tr.x = abs_x + (abs_w - track_th) / 2;
        tr.y = abs_y;
        tr.w = track_th;
        tr.h = usable_h;
    }
    return tr;
}

int KitsuSlider::knobPosition(int track_x, int track_y,
                              int track_w, int track_h,
                              int knob_size) const {
    float range = max_ - min_;
    float norm = (range > 0.0f) ? ((value_ - min_) / range) : 0.0f;

    if (orientation_ == SliderOrientation::HORIZONTAL) {
        int usable = track_w - knob_size;
        return track_x + (int)(norm * usable);
    } else {
        int usable = track_h - knob_size;
        return track_y + (int)((1.0f - norm) * usable);
    }
}

float KitsuSlider::valueFromPos(int mx, int my,
                                int track_x, int track_y,
                                int track_w, int track_h,
                                int knob_size) const {
    float norm = 0.0f;

    if (orientation_ == SliderOrientation::HORIZONTAL) {
        int usable = track_w - knob_size;
        if (usable <= 0) return min_;
        int pos = mx - track_x - knob_size / 2;
        norm = (float)pos / (float)usable;
    } else {
        int usable = track_h - knob_size;
        if (usable <= 0) return min_;
        int pos = my - track_y - knob_size / 2;
        norm = 1.0f - (float)pos / (float)usable;
    }

    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;

    return min_ + norm * (max_ - min_);
}

// ============================================================
// Eventos
// ============================================================
bool KitsuSlider::handleEvent(const SDL_Event& e) {
    if (disabled_ || !visible || !enabled) return false;

    KitsuTheme& t = KitsuTheme::active();
    SDL_Rect r = globalRect();
    SDL_Rect tr = trackRect(r.x, r.y, r.w, r.h, num_w_);

    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside != mouse_inside_) {
                mouse_inside_ = inside;
                invalidate();
            }

            if (dragging_) {
                value(valueFromPos(mx, my, tr.x, tr.y, tr.w, tr.h,
                                   t.slider_knob_size));
                return true;
            }
            return inside;
        }

        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;

            SDL_Rect clickable = tr;
            if (orientation_ == SliderOrientation::HORIZONTAL) {
                clickable.y -= (t.slider_knob_size - t.slider_track_thickness) / 2;
                clickable.h = t.slider_knob_size;
            } else {
                clickable.x -= (t.slider_knob_size - t.slider_track_thickness) / 2;
                clickable.w = t.slider_knob_size;
            }

            if (mx >= clickable.x && mx < clickable.x + clickable.w &&
                my >= clickable.y && my < clickable.y + clickable.h) {
                dragging_ = true;
                value(valueFromPos(mx, my, tr.x, tr.y, tr.w, tr.h,
                                   t.slider_knob_size));
                invalidate();
                return true;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            if (dragging_) {
                dragging_ = false;
                invalidate();
                return true;
            }
            break;
        }
    }
    return false;
}

// ============================================================
// Render
// ============================================================
void KitsuSlider::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // Resolver colores
    Color num_color;
    if (disabled_) {
        num_color = t.text_disabled;
    } else if (mouse_inside_ || dragging_) {
        num_color = t.accent;
    } else {
        num_color = t.text_primary;
    }

    updateNumberTexture(renderer, num_color);

    SDL_Rect tr = trackRect(r.x, r.y, r.w, r.h, num_w_);

    Color track_empty;
    Color track_filled;
    Color knob_fill;
    Color knob_border;

    if (disabled_) {
        track_empty = t.bg_disabled;
        track_filled = t.border_disabled;
        knob_fill = t.bg_secondary;
        knob_border = t.border_disabled;
    } else {
        track_empty = t.bg_tertiary;
        track_filled = t.accent;
        knob_fill = t.bg_secondary;
        knob_border = (mouse_inside_ || dragging_) ? t.accent : t.border;
    }

    // ===== 1. Track vacío =====
    KitsuRect((float)tr.x, (float)tr.y, (float)tr.w, (float)tr.h)
        .radius(t.radius_slider_track)
        .fill(track_empty)
        .draw(renderer);

    // ===== 2. Fill =====
    float range = max_ - min_;
    float norm = (range > 0.0f) ? ((value_ - min_) / range) : 0.0f;

    SDL_Rect fill = tr;
    if (orientation_ == SliderOrientation::HORIZONTAL) {
        fill.w = (int)(tr.w * norm);
    } else {
        fill.h = (int)(tr.h * norm);
        fill.y = tr.y + tr.h - fill.h;
    }

    if (fill.w > 0 && fill.h > 0) {
        KitsuRect((float)fill.x, (float)fill.y,
                  (float)fill.w, (float)fill.h)
            .radius(t.radius_slider_track)
            .fill(track_filled)
            .draw(renderer);
    }

    // ===== 3. Knob =====
    const int knob_size = t.slider_knob_size;
    int knob_pos = knobPosition(tr.x, tr.y, tr.w, tr.h, knob_size);

    SDL_Rect knob;
    if (orientation_ == SliderOrientation::HORIZONTAL) {
        knob.x = knob_pos;
        knob.y = tr.y + (tr.h - knob_size) / 2;
    } else {
        knob.x = tr.x + (tr.w - knob_size) / 2;
        knob.y = knob_pos;
    }
    knob.w = knob_size;
    knob.h = knob_size;

    KitsuRect((float)knob.x, (float)knob.y,
              (float)knob.w, (float)knob.h)
        .radius(t.radius_slider_knob)
        .fill(knob_fill)
        .draw(renderer);

    KitsuRect((float)knob.x, (float)knob.y,
              (float)knob.w, (float)knob.h)
        .radius(t.radius_slider_knob)
        .fill(knob_border)
        .drawOutline(renderer, 2.0f);

    // ===== 4. Número =====
    if (show_number_ && number_texture_) {
        int tx, ty;
        if (orientation_ == SliderOrientation::HORIZONTAL) {
            tx = tr.x + tr.w + 12;
            ty = r.y + (r.h - num_h_) / 2;
        } else {
            tx = r.x + (r.w - num_w_) / 2;
            ty = tr.y + tr.h + 12;
        }
        SDL_Rect dst = { tx, ty, num_w_, num_h_ };
        SDL_RenderCopy(renderer, number_texture_, nullptr, &dst);
    }

    clearNeedsRender();
}

} // namespace KitsuGui
