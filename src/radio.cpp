#include "kitsugui/radio.h"
#include "kitsugui/shapes.h"

namespace KitsuGui {

TTF_Font* KitsuRadioButton::g_font = nullptr;

// ============================================================
// KitsuRadioButton
// ============================================================

KitsuRadioButton::KitsuRadioButton(const std::string& text, bool checked)
    : text(text), checked(checked) {
    bounds = {0, 0, 220, 24};
    requested_w = 220;
    requested_h = 24;
}

KitsuRadioButton::~KitsuRadioButton() {
    destroyTextTexture();
}

void KitsuRadioButton::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

KitsuRadioButton& KitsuRadioButton::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuRadioButton& KitsuRadioButton::withChecked(bool c) {
    setChecked(c);
    return *this;
}

KitsuRadioButton& KitsuRadioButton::withCallback(std::function<void(bool)> cb) {
    callback = cb;
    return *this;
}

void KitsuRadioButton::setChecked(bool c) {
    if (checked != c) {
        checked = c;
        markDirty();
        if (parent) parent->markDirty();
        if (callback) callback(checked);
    }
}

void KitsuRadioButton::activateFromGroup() {
    if (!checked) {
        checked = true;
        markDirty();
        if (parent) parent->markDirty();
        if (callback) callback(true);
    }
}

// ===== EVENTOS =====
bool KitsuRadioButton::handleEvent(const SDL_Event& e) {
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
                if (!checked) {
                    if (group) {
                        group->notifySelected(this);
                    } else {
                        setChecked(true);
                    }
                }
                markDirty();
                if (parent) parent->markDirty();
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
void KitsuRadioButton::updateTextTexture(SDL_Renderer* renderer) {
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
void KitsuRadioButton::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    // ===== 1. Círculo exterior =====
    const int circle_size = t.radio_size;
    int circle_y = abs.y + (abs.h - circle_size) / 2;
    int circle_x = abs.x;
    
    float cx = (float)circle_x + circle_size / 2.0f;
    float cy = (float)circle_y + circle_size / 2.0f;
    float outer_radius = circle_size / 2.0f - 1.0f;
    
    // Colores según estado
    Color circle_border;
    Color circle_fill;
    Color dot_color;
    
    if (!enabled_) {
        circle_border = t.border_disabled;
        circle_fill = t.bg_disabled;
        dot_color = t.text_disabled;
    } else if (checked) {
        circle_border = t.accent;
        circle_fill = t.bg_secondary;
        dot_color = t.accent;
    } else if (mouse_inside || mouse_down) {
        circle_border = t.accent;
        circle_fill = t.bg_secondary;
        dot_color = t.accent;
    } else {
        circle_border = t.border;
        circle_fill = t.bg_secondary;
        dot_color = t.accent;
    }
    
    // ===== Borde: círculo exterior relleno =====
    KitsuCircle(cx, cy, outer_radius)
        .fill(circle_border)
        .draw(renderer);
    
    // ===== Relleno: círculo interior más pequeño =====
    KitsuCircle(cx, cy, outer_radius - 2.0f)   // -2 = grosor del borde
        .fill(circle_fill)
        .draw(renderer);
    
    // ===== Punto central (si checked) =====
    if (checked) {
        KitsuCircle(cx, cy, outer_radius - 5.0f)
            .fill(dot_color)
            .draw(renderer);
    }
    
    // ===== Texto =====
    updateTextTexture(renderer);
    if (text_texture && tex_w > 0) {
        int text_x = circle_x + circle_size + 8;
        int text_y = abs.y + (abs.h - tex_h) / 2;
        SDL_Rect text_rect = { text_x, text_y, tex_w, tex_h };
        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    }
    
    clearDirty();
}

// ============================================================
// KitsuRadioGroup
// ============================================================

KitsuRadioGroup::KitsuRadioGroup()
    : KitsuBox(false) {
    setSpacing(6);
    setPadding(0);
    setAlignment(KitsuAlign::START);
    setJustify(KitsuJustify::START);
    setAutoLayout(true);
}

KitsuRadioGroup::~KitsuRadioGroup() {
    for (auto* r : owned_radios) delete r;
    owned_radios.clear();
    radios.clear();
}

void KitsuRadioGroup::addRadio(KitsuRadioButton* radio, bool owns) {
    if (!radio) return;
    
    radio->group = this;
    radios.push_back(radio);
    if (owns) owned_radios.push_back(radio);
    
    addChild(radio, false);
    
    if (radio->isChecked()) {
        if (selected && selected != radio) {
            selected->setChecked(false);
        }
        selected = radio;
    }
    
    markDirty();
}

void KitsuRadioGroup::removeRadio(KitsuRadioButton* radio) {
    if (!radio) return;
    
    auto it = std::find(radios.begin(), radios.end(), radio);
    if (it != radios.end()) radios.erase(it);
    
    auto it2 = std::find(owned_radios.begin(), owned_radios.end(), radio);
    if (it2 != owned_radios.end()) {
        owned_radios.erase(it2);
        delete radio;
    }
    
    if (selected == radio) selected = nullptr;
    markDirty();
}

void KitsuRadioGroup::notifySelected(KitsuRadioButton* radio) {
    if (!radio || radio == selected) return;
    
    if (selected && selected != radio) {
        selected->setChecked(false);
    }
    
    selected = radio;
    radio->activateFromGroup();
    
    if (on_change) on_change(selected);
    
    markDirty();
}

void KitsuRadioGroup::tick() {
    for (auto* c : getChildren()) {
        if (c) c->tick();
    }
}

bool KitsuRadioGroup::handleEvent(const SDL_Event& e) {
    return KitsuBox::handleEvent(e);
}

} // namespace KitsuGui
