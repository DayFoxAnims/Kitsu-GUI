#include "kitsugui/slider.h"
#include "kitsugui/shapes.h"
#include <cstdio>
#include <cmath>

namespace KitsuGui {

TTF_Font* KitsuSlider::g_font = nullptr;

// Dimensiones
static const int TRACK_THICKNESS = 6;
static const int KNOB_SIZE = 16;
static const int NUMBER_GAP = 12;
static const int NUMBER_WIDTH = 50;

KitsuSlider::KitsuSlider(float min, float max, float value)
    : min_value(min), max_value(max), value(value) {
    bounds = {0, 0, 250, 24};
    requested_w = 250;
    requested_h = 24;
}

KitsuSlider::~KitsuSlider() {
    destroyNumberTexture();
}

void KitsuSlider::destroyNumberTexture() {
    if (number_texture) {
        SDL_DestroyTexture(number_texture);
        number_texture = nullptr;
        num_w = num_h = 0;
    }
}

// ===== CONFIGURACIÓN =====
KitsuSlider& KitsuSlider::withOrientation(SliderOrientation o) {
    orientation = o;
    if (o == SliderOrientation::HORIZONTAL) {
        bounds.w = 250; bounds.h = 24;
        requested_w = 250; requested_h = 24;
    } else {
        bounds.w = 24; bounds.h = 200;
        requested_w = 24; requested_h = 200;
    }
    markDirty();
    return *this;
}

KitsuSlider& KitsuSlider::withValue(float v) { setValue(v); return *this; }
KitsuSlider& KitsuSlider::withRange(float min, float max) {
    min_value = min; max_value = max;
    if (value < min_value) value = min_value;
    if (value > max_value) value = max_value;
    markDirty();
    return *this;
}
KitsuSlider& KitsuSlider::withStep(float s) { step = s; return *this; }
KitsuSlider& KitsuSlider::withShowNumber(bool show) { show_number = show; markDirty(); return *this; }
KitsuSlider& KitsuSlider::withNumberSuffix(const std::string& suffix) { number_suffix = suffix; markDirty(); return *this; }
KitsuSlider& KitsuSlider::withDecimals(int d) { decimals = d; markDirty(); return *this; }
KitsuSlider& KitsuSlider::withFont(TTF_Font* f) { font = f; markDirty(); return *this; }
KitsuSlider& KitsuSlider::withCallback(std::function<void(float)> cb) { callback = cb; return *this; }
KitsuSlider& KitsuSlider::disabled() { enabled_ = false; markDirty(); return *this; }

// ===== VALOR =====
void KitsuSlider::setValue(float v) {
    if (v < min_value) v = min_value;
    if (v > max_value) v = max_value;
    if (step > 0.0f) v = roundf(v / step) * step;
    
    if (value != v) {
        value = v;
        markDirty();
        if (parent) parent->markDirty();
        if (callback) callback(value);
    }
}

// ===== RECT =====
SDL_Rect KitsuSlider::getTrackRect(int abs_x, int abs_y, int abs_w, int abs_h, int num_w) const {
    SDL_Rect track;
    
    if (orientation == SliderOrientation::HORIZONTAL) {
        int usable_w = abs_w - (show_number ? (num_w + NUMBER_GAP) : 0);
        track.x = abs_x;
        track.y = abs_y + (abs_h - TRACK_THICKNESS) / 2;
        track.w = usable_w;
        track.h = TRACK_THICKNESS;
    } else {
        int usable_h = abs_h - (show_number ? (num_h + NUMBER_GAP) : 0);
        track.x = abs_x + (abs_w - TRACK_THICKNESS) / 2;
        track.y = abs_y;
        track.w = TRACK_THICKNESS;
        track.h = usable_h;
    }
    return track;
}

int KitsuSlider::getKnobPosition(int track_x, int track_y, int track_w, int track_h, int knob_size) const {
    float range = max_value - min_value;
    float norm = (range > 0.0f) ? ((value - min_value) / range) : 0.0f;
    
    if (orientation == SliderOrientation::HORIZONTAL) {
        int usable = track_w - knob_size;
        return track_x + (int)(norm * usable);
    } else {
        int usable = track_h - knob_size;
        return track_y + (int)((1.0f - norm) * usable);
    }
}

float KitsuSlider::getValueFromPosition(int mx, int my, int track_x, int track_y, int track_w, int track_h, int knob_size) const {
    float norm = 0.0f;
    
    if (orientation == SliderOrientation::HORIZONTAL) {
        int usable = track_w - knob_size;
        if (usable <= 0) return min_value;
        int pos = mx - track_x - knob_size / 2;
        norm = (float)pos / (float)usable;
    } else {
        int usable = track_h - knob_size;
        if (usable <= 0) return min_value;
        int pos = my - track_y - knob_size / 2;
        norm = 1.0f - (float)pos / (float)usable;
    }
    
    if (norm < 0.0f) norm = 0.0f;
    if (norm > 1.0f) norm = 1.0f;
    
    return min_value + norm * (max_value - min_value);
}

// ===== EVENTOS =====
bool KitsuSlider::handleEvent(const SDL_Event& e) {
    if (!enabled_ || !visible) return false;
    
    SDL_Rect abs = getAbsoluteBounds();
    SDL_Rect track = getTrackRect(abs.x, abs.y, abs.w, abs.h, NUMBER_WIDTH);
    
    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            if (inside != mouse_inside) {
                mouse_inside = inside;
                markDirty();
                if (parent) parent->markDirty();
            }
            
            if (dragging) {
                float v = getValueFromPosition(mx, my, track.x, track.y, track.w, track.h, KNOB_SIZE);
                setValue(v);
                return true;
            }
            return inside;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            
            SDL_Rect clickable = track;
            if (orientation == SliderOrientation::HORIZONTAL) {
                clickable.y -= (KNOB_SIZE - TRACK_THICKNESS) / 2;
                clickable.h = KNOB_SIZE;
            } else {
                clickable.x -= (KNOB_SIZE - TRACK_THICKNESS) / 2;
                clickable.w = KNOB_SIZE;
            }
            
            if (mx >= clickable.x && mx < clickable.x + clickable.w &&
                my >= clickable.y && my < clickable.y + clickable.h) {
                dragging = true;
                float v = getValueFromPosition(mx, my, track.x, track.y, track.w, track.h, KNOB_SIZE);
                setValue(v);
                markDirty();
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            if (dragging) {
                dragging = false;
                markDirty();
                if (parent) parent->markDirty();
                return true;
            }
            break;
        }
    }
    return false;
}

// ===== TEXTURA DEL NÚMERO =====
void KitsuSlider::updateNumberTexture(SDL_Renderer* renderer) {
    if (!show_number) return;
    
    TTF_Font* active = getActiveFont();
    if (!active) return;
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f%s", decimals, value, number_suffix.c_str());
    
    KitsuTheme& t = KitsuTheme::current();
    Color text_color;
    
    if (!enabled_) {
        text_color = t.text_disabled;
    } else if (mouse_inside || dragging) {
        text_color = t.accent;
    } else {
        text_color = t.text_primary;
    }
    
    Uint8 r = (Uint8)text_color.r;
    Uint8 g = (Uint8)text_color.g;
    Uint8 b = (Uint8)text_color.b;
    
    if (number_texture &&
        cached_number == buf &&
        cached_r == r && cached_g == g && cached_b == b) {
        return;
    }
    
    destroyNumberTexture();
    
    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, buf, fg);
    if (!surface) return;
    
    number_texture = SDL_CreateTextureFromSurface(renderer, surface);
    num_w = surface->w;
    num_h = surface->h;
    SDL_FreeSurface(surface);
    
    cached_number = buf;
    cached_r = r; cached_g = g; cached_b = b;
}

// ===== RENDER =====
void KitsuSlider::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    updateNumberTexture(renderer);
    
    SDL_Rect track = getTrackRect(abs.x, abs.y, abs.w, abs.h, num_w);
    
    Color track_empty_color;
    Color track_filled_color;
    Color knob_fill;
    Color knob_border;
    
    if (!enabled_) {
        track_empty_color = t.bg_disabled;
        track_filled_color = t.border_disabled;
        knob_fill = t.bg_secondary;
        knob_border = t.border_disabled;
    } else {
        track_empty_color = t.bg_tertiary;
        track_filled_color = t.accent;
        knob_fill = t.bg_secondary;
        knob_border = t.border;
        
        if (mouse_inside || dragging) {
            knob_border = t.accent;
        }
    }
    
    // ===== 1. Track vacío =====
    KitsuRect((float)track.x, (float)track.y, (float)track.w, (float)track.h)
        .radius(t.radius_slider_track)   // ← del tema
        .fill(track_empty_color)
        .draw(renderer);
    
    // ===== 2. Fill =====
    float range = max_value - min_value;
    float norm = (range > 0.0f) ? ((value - min_value) / range) : 0.0f;
    
    SDL_Rect fill = track;
    if (orientation == SliderOrientation::HORIZONTAL) {
        fill.w = (int)(track.w * norm);
    } else {
        fill.h = (int)(track.h * norm);
        fill.y = track.y + track.h - fill.h;
    }
    
    if (fill.w > 0 && fill.h > 0) {
        KitsuRect((float)fill.x, (float)fill.y, (float)fill.w, (float)fill.h)
            .radius(t.radius_slider_track)
            .fill(track_filled_color)
            .draw(renderer);
    }
    
    // ===== 3. Knob =====
    const int KNOB_SIZE = t.slider_knob_size;
    int knob_pos = getKnobPosition(track.x, track.y, track.w, track.h, KNOB_SIZE);
    
    SDL_Rect knob;
    if (orientation == SliderOrientation::HORIZONTAL) {
        knob.x = knob_pos;
        knob.y = track.y + (track.h - KNOB_SIZE) / 2;
    } else {
        knob.x = track.x + (track.w - KNOB_SIZE) / 2;
        knob.y = knob_pos;
    }
    knob.w = KNOB_SIZE;
    knob.h = KNOB_SIZE;
    
    KitsuRect((float)knob.x, (float)knob.y, (float)knob.w, (float)knob.h)
        .radius(t.radius_slider_knob)   // ← del tema
        .fill(knob_fill)
        .draw(renderer);
    
    KitsuRect((float)knob.x, (float)knob.y, (float)knob.w, (float)knob.h)
        .radius(t.radius_slider_knob)
        .fill(knob_border)
        .drawOutline(renderer, 2.0f);
    
    // ===== 4. Número =====
    if (show_number && number_texture) {
        int text_x, text_y;
        if (orientation == SliderOrientation::HORIZONTAL) {
            text_x = track.x + track.w + NUMBER_GAP;
            text_y = abs.y + (abs.h - num_h) / 2;
        } else {
            text_x = abs.x + (abs.w - num_w) / 2;
            text_y = track.y + track.h + NUMBER_GAP;
        }
        SDL_Rect text_rect = { text_x, text_y, num_w, num_h };
        SDL_RenderCopy(renderer, number_texture, nullptr, &text_rect);
    }
    
    clearDirty();
}

} // namespace KitsuGui
