#include "kitsugui/switch.h"
#include "kitsugui/shapes.h"

namespace KitsuGui {

TTF_Font* KitsuSwitch::g_font = nullptr;

// Dimensiones del switch
static const int SWITCH_W = 44;
static const int SWITCH_H = 22;
static const int KNOB_PAD = 3;
static const int BORDER_THICK = 2;

KitsuSwitch::KitsuSwitch(const std::string& text, bool checked)
    : text(text), checked(checked) {
    bounds = {0, 0, 220, SWITCH_H};
    requested_w = 220;
    requested_h = SWITCH_H;
}

KitsuSwitch::~KitsuSwitch() {
    destroyTextTexture();
}

void KitsuSwitch::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

KitsuSwitch& KitsuSwitch::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuSwitch& KitsuSwitch::withChecked(bool c) {
    setChecked(c);
    return *this;
}

KitsuSwitch& KitsuSwitch::withCallback(std::function<void(bool)> cb) {
    callback = cb;
    return *this;
}

KitsuSwitch& KitsuSwitch::disabled() {
    enabled_ = false;
    markDirty();
    return *this;
}

void KitsuSwitch::setChecked(bool c) {
    if (checked != c) {
        checked = c;
        markDirty();
        if (parent) parent->markDirty();
        if (callback) callback(checked);
    }
}

// ===== EVENTOS =====
bool KitsuSwitch::handleEvent(const SDL_Event& e) {
    if (!enabled_ || !visible) return false;
    
    SDL_Rect abs = getAbsoluteBounds();
    
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
            return inside;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            if (inside) {
                mouse_down = true;
                markDirty();
                if (parent) parent->markDirty();
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (mouse_down && inside) {
                mouse_down = false;
                toggle();
                return true;
            }
            mouse_down = false;
            markDirty();
            if (parent) parent->markDirty();
            break;
        }
    }
    return false;
}

// ===== TEXTURA =====
void KitsuSwitch::updateTextTexture(SDL_Renderer* renderer) {
    TTF_Font* active = getActiveFont();
    
    KitsuTheme& t = KitsuTheme::current();
    Color text_color;
    
    if (!enabled_) {
        text_color = t.text_disabled;
    } else if (mouse_inside || mouse_down) {
        text_color = t.accent;
    } else {
        text_color = t.text_primary;
    }
    
    Uint8 r = (Uint8)text_color.r;
    Uint8 g = (Uint8)text_color.g;
    Uint8 b = (Uint8)text_color.b;
    
    if (text_texture &&
        cached_text == text &&
        cached_font == active &&
        cached_r == r && cached_g == g && cached_b == b) {
        return;
    }
    
    destroyTextTexture();
    
    if (!active || text.empty() || !renderer) return;
    
    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, text.c_str(), fg);
    if (!surface) return;
    
    text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w = surface->w;
    tex_h = surface->h;
    SDL_FreeSurface(surface);
    
    cached_text = text;
    cached_font = active;
    cached_r = r; cached_g = g; cached_b = b;
}

// ===== RENDER =====
void KitsuSwitch::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    // ===== 1. Track =====
    const int SW_W = t.switch_width;
    const int SW_H = t.switch_height;
    
    int track_y = abs.y + (abs.h - SW_H) / 2;
    SDL_Rect track = { abs.x, track_y, SW_W, SW_H };
    
    Color track_border;
    Color track_fill;
    Color knob_color;
    
    if (!enabled_) {
        track_border = t.border_disabled;
        track_fill = t.bg_disabled;
        knob_color = t.text_disabled;
    } else if (checked) {
        track_border = t.accent;
        track_fill = t.accent;
        knob_color = t.text_on_accent;
    } else if (mouse_inside || mouse_down) {
        track_border = t.accent;
        track_fill = t.bg_tertiary;
        knob_color = t.accent;
    } else {
        track_border = t.border;
        track_fill = t.bg_tertiary;
        knob_color = t.border;
    }
    
    // Fondo del track
    KitsuRect((float)track.x, (float)track.y, (float)track.w, (float)track.h)
        .radius(t.radius_switch)   // ← del tema (0 = cuadrado)
        .fill(track_fill)
        .draw(renderer);
    
    // Borde del track
    KitsuRect((float)track.x, (float)track.y, (float)track.w, (float)track.h)
        .radius(t.radius_switch)
        .fill(track_border)
        .drawOutline(renderer, (float)t.border_thickness_switch);
    
    // ===== 2. Knob (rectángulo cuadrado) =====
    const int KNOB_PAD = t.switch_knob_pad;
    int knob_w = (SW_W / 2) - 2 * KNOB_PAD;
    int knob_h = SW_H - 2 * KNOB_PAD;
    
    int knob_x = checked 
        ? track.x + SW_W - knob_w - KNOB_PAD
        : track.x + KNOB_PAD;
    int knob_y = track.y + KNOB_PAD;
    
    // Knob como rectángulo (cuadrado)
    KitsuRect((float)knob_x, (float)knob_y, (float)knob_w, (float)knob_h)
        .radius(t.radius_switch)   // mismo radio que el track
        .fill(knob_color)
        .draw(renderer);
    
    // ===== 3. Texto =====
    updateTextTexture(renderer);
    if (text_texture && tex_w > 0) {
        int text_x = track.x + SW_W + 10;
        int text_y = abs.y + (abs.h - tex_h) / 2;
        SDL_Rect text_rect = { text_x, text_y, tex_w, tex_h };
        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    }
    
    clearDirty();
}

} // namespace KitsuGui
