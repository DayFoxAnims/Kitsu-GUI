#include "kitsugui/dropdown.h"
#include "kitsugui/context_menu.h"
#include "kitsugui/shapes.h"
#include "internal.h"

namespace KitsuGui {

TTF_Font* KitsuDropdown::g_font = nullptr;

// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================
KitsuDropdown::KitsuDropdown(int width) {
    bounds = {0, 0, width, 36};
    requested_w = width;
    requested_h = 36;
    
    // Crear el menú interno (se llenará cuando se añadan items)
    menu = new KitsuContextMenu(width);
}

KitsuDropdown::~KitsuDropdown() {
    destroyTextTexture();
    if (menu) {
        delete menu;
        menu = nullptr;
    }
}

void KitsuDropdown::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

// ============================================================
// CONFIGURACIÓN FLUIDA
// ============================================================
KitsuDropdown& KitsuDropdown::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuDropdown& KitsuDropdown::withPlaceholder(const std::string& text) {
    placeholder = text;
    markDirty();
    return *this;
}

// ============================================================
// GESTIÓN DE ITEMS
// ============================================================
int KitsuDropdown::addItem(const std::string& text,
                          std::function<void()> cb) {
    Item item;
    item.text = text;
    item.callback = cb;
    items.push_back(item);
    
    int index = (int)items.size() - 1;
    
    // Añadir al menú contextual
    if (menu) {
        menu->addItem(text, [this, index, cb]() {
            this->setSelectedIndex(index);
            if (cb) cb();
            this->menu_open = false;
        });
    }
    
    return index;
}

void KitsuDropdown::addItems(const std::vector<std::string>& new_items) {
    for (const auto& text : new_items) {
        addItem(text);
    }
}

void KitsuDropdown::clearItems() {
    items.clear();
    // No podemos limpiar el menú interno (KitsuContextMenu no tiene clearItems),
    // así que simplemente reseteamos la selección.
    // TODO: implementar clearItems en KitsuContextMenu
    selected_index = -1;
    markDirty();
}

// ============================================================
// SELECCIÓN
// ============================================================
std::string KitsuDropdown::getSelectedText() const {
    if (selected_index < 0 || selected_index >= (int)items.size()) {
        return "";
    }
    return items[selected_index].text;
}

void KitsuDropdown::setSelectedIndex(int index) {
    if (index < -1 || index >= (int)items.size()) return;
    if (index == selected_index) return;
    
    selected_index = index;
    markDirty();
    
    if (on_change) {
        on_change(selected_index, getSelectedText());
    }
}

void KitsuDropdown::setSelectedText(const std::string& text) {
    for (int i = 0; i < (int)items.size(); i++) {
        if (items[i].text == text) {
            setSelectedIndex(i);
            return;
        }
    }
}

// ============================================================
// TEXTURA DEL TEXTO
// ============================================================
void KitsuDropdown::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = getActiveFont();
    if (!active || !renderer) return;
    
    std::string display = getSelectedText();
    bool is_placeholder = false;
    
    if (display.empty()) {
        display = placeholder;
        is_placeholder = true;
    }
    
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;
    
    if (is_placeholder) {
        KitsuTheme& t = KitsuTheme::current();
        r = (Uint8)t.text_secondary.r;
        g = (Uint8)t.text_secondary.g;
        b = (Uint8)t.text_secondary.b;
    }
    
    if (text_texture &&
        cached_text == display &&
        cached_font == active &&
        cached_r == r && cached_g == g && cached_b == b) {
        return;
    }
    
    destroyTextTexture();
    
    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, display.c_str(), fg);
    if (!surface) return;
    
    text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w = surface->w;
    tex_h = surface->h;
    SDL_FreeSurface(surface);
    
    cached_text = display;
    cached_font = active;
    cached_r = r; cached_g = g; cached_b = b;
}

// ============================================================
// EVENTOS
// ============================================================
bool KitsuDropdown::handleEvent(const SDL_Event& e) {
    if (!enabled || !visible) return false;
    
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
            return false;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (inside && menu) {
                menu_open = true;
                int menu_x = abs.x;
                int menu_y = abs.y + abs.h;
                menu->showAt(menu_x, menu_y);
                markDirty();
                return true;
            }
            break;
        }
    }
    
    return false;
}
void KitsuDropdown::tick() {
    // Si el menú se cerró externamente, actualizar el estado
    if (menu_open && menu && !menu->isOpen()) {
        menu_open = false;
        markDirty();
    }
}

// ============================================================
// RENDER
// ============================================================
void KitsuDropdown::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    // ===== Colores según estado =====
    Color bg_color = t.bg_tertiary;
    Color border_color = t.border;
    Color text_color = t.text_primary;
    
    if (!enabled) {
        bg_color = t.bg_disabled;
        border_color = t.border_disabled;
        text_color = t.text_disabled;
    } else if (menu_open) {
        border_color = t.border_focus;
    } else if (mouse_inside) {
        border_color = t.accent;
    }
    
    // ===== 1. Fondo =====
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(t.radius_button)
        .fill(bg_color)
        .draw(renderer);
    
    // ===== 2. Borde =====
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(t.radius_button)
        .fill(border_color)
        .drawOutline(renderer, (float)t.border_thickness_button);
    
    // ===== 3. Texto =====
    updateTextTexture(renderer, text_color);
    if (text_texture && tex_w > 0) {
        int text_x = abs.x + 12;
        int text_y = abs.y + (abs.h - tex_h) / 2;
        
        int max_text_w = abs.w - 40;
        if (tex_w > max_text_w) {
            SDL_Rect src = { 0, 0, max_text_w, tex_h };
            SDL_Rect dst = { text_x, text_y, max_text_w, tex_h };
            SDL_RenderCopy(renderer, text_texture, &src, &dst);
        } else {
            SDL_Rect dst = { text_x, text_y, tex_w, tex_h };
            SDL_RenderCopy(renderer, text_texture, nullptr, &dst);
        }
    }
    
    // ===== 4. Flecha ▼ =====
    int arrow_size = 8;
    int arrow_x = abs.x + abs.w - 20;
    int arrow_y = abs.y + abs.h / 2 - 4;
    
    SDL_SetRenderDrawColor(renderer,
        (Uint8)border_color.r, (Uint8)border_color.g, (Uint8)border_color.b, 255);
    
    for (int i = 0; i < arrow_size; i++) {
        SDL_RenderDrawLine(renderer,
            arrow_x + i, arrow_y + i,
            arrow_x + arrow_size * 2 - i, arrow_y + i);
    }
    
    clearDirty();
}

} // namespace KitsuGui
