#include "kitsugui/check_box.h"
#include "kitsugui/shapes.h"

namespace KitsuGui {

TTF_Font* KitsuCheckBox::g_font = nullptr;

KitsuCheckBox::KitsuCheckBox(const std::string& text, bool checked)
    : text(text), checked(checked) {
    bounds = {0, 0, 200, 24};
    requested_w = 200;
    requested_h = 24;
}

KitsuCheckBox::~KitsuCheckBox() {
    destroyTextTexture();
}

void KitsuCheckBox::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

KitsuCheckBox& KitsuCheckBox::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuCheckBox& KitsuCheckBox::withChecked(bool c) {
    setChecked(c);
    return *this;
}

KitsuCheckBox& KitsuCheckBox::withCallback(std::function<void(bool)> cb) {
    callback = cb;
    return *this;
}

KitsuCheckBox& KitsuCheckBox::disabled() {
    enabled_ = false;
    markDirty();
    return *this;
}

void KitsuCheckBox::setChecked(bool c) {
    if (checked != c) {
        checked = c;
        markDirty();
        if (parent) parent->markDirty();
        if (callback) callback(checked);
    }
}

// ===== EVENTOS =====
bool KitsuCheckBox::handleEvent(const SDL_Event& e) {
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
void KitsuCheckBox::updateTextTexture(SDL_Renderer* renderer) {
    TTF_Font* active = getActiveFont();
    
    // Color del texto según estado y tema
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
void KitsuCheckBox::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    // ===== 1. Cuadrado del check =====
    const int box_size = t.checkbox_size;
    int box_y = abs.y + (abs.h - box_size) / 2;
    int box_x = abs.x;
    
    // Colores según estado
    Color box_border;
    Color box_fill;
    Color check_color;
    
    if (!enabled_) {
        box_border = t.border_disabled;
        box_fill = t.bg_disabled;
        check_color = t.text_disabled;
    } else if (checked) {
        box_border = t.accent;
        box_fill = t.accent;
        check_color = t.text_on_accent;
    } else if (mouse_inside || mouse_down) {
        box_border = t.accent;
        box_fill = t.bg_tertiary;
        check_color = t.text_on_accent;
    } else {
        box_border = t.border;
        box_fill = t.bg_secondary;
        check_color = t.text_primary;
    }
    
    // Fondo del cuadrado
    KitsuRect((float)box_x, (float)box_y, (float)box_size, (float)box_size)
        .radius(t.radius_checkbox)   // ← del tema
        .fill(box_fill)
        .draw(renderer);
    
    // Borde del cuadrado
    KitsuRect((float)box_x, (float)box_y, (float)box_size, (float)box_size)
        .radius(t.radius_checkbox)   // ← del tema
        .fill(box_border)
        .drawOutline(renderer, (float)t.border_thickness_checkbox);
    
    // ===== 2. Check mark =====
    if (checked) {
        int cx = box_x + box_size / 2;
        int cy = box_y + box_size / 2;
        
        SDL_SetRenderDrawColor(renderer,
            (Uint8)check_color.r, (Uint8)check_color.g, (Uint8)check_color.b, 255);
        
        for (int thick = -1; thick <= 1; thick++) {
            SDL_RenderDrawLine(renderer, cx - 5, cy + thick, cx - 1, cy + 4 + thick);
        }
        for (int thick = -1; thick <= 1; thick++) {
            SDL_RenderDrawLine(renderer, cx - 1, cy + 4 + thick, cx + 5, cy - 4 + thick);
        }
    }
    
    // ===== 3. Texto =====
    updateTextTexture(renderer);
    if (text_texture && tex_w > 0) {
        int text_x = box_x + box_size + 8;
        int text_y = abs.y + (abs.h - tex_h) / 2;
        SDL_Rect text_rect = { text_x, text_y, tex_w, tex_h };
        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    }
    
    clearDirty();
}

} // namespace KitsuGui
