#include "kitsugui/context_menu.h"
#include "kitsugui/shapes.h"
#include "internal.h"

namespace KitsuGui {

// ============================================================
// KitsuMenuItem
// ============================================================
TTF_Font* KitsuMenuItem::g_font = nullptr;

KitsuMenuItem::KitsuMenuItem(const std::string& text, std::function<void()> cb)
    : text(text), callback(cb) {
    bounds = {0, 0, 200, 32};
    requested_w = 200;
    requested_h = 32;
}

KitsuMenuItem::~KitsuMenuItem() {
    destroyTextTexture();
}

void KitsuMenuItem::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

void KitsuMenuItem::setText(const std::string& t) {
    if (text != t) {
        text = t;
        markDirty();
    }
}

void KitsuMenuItem::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = getActiveFont();
    if (!active || text.empty() || !renderer) return;
    
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;
    
    if (text_texture &&
        cached_text == text &&
        cached_font == active &&
        cached_r == r && cached_g == g && cached_b == b) {
        return;
    }
    
    destroyTextTexture();
    
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

bool KitsuMenuItem::handleEvent(const SDL_Event& e) {
    if (!visible || !item_enabled || separator) return false;
    
    SDL_Rect abs = getAbsoluteBounds();
    
    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            if (inside != mouse_inside) {
                mouse_inside = inside;
                markDirty();
            }
            return false;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            if (inside) {
                mouse_down = true;
                markDirty();
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            bool was_down = mouse_down;
            mouse_down = false;
            markDirty();
            
            if (was_down && inside) {
                auto cb = callback;
                // Cerrar el menú ANTES del callback
                if (KitsuContextMenu::getActive()) {
                    KitsuContextMenu::getActive()->hide();
                }
                if (cb) cb();
                return true;
            }
            break;
        }
    }
    return false;
}

void KitsuMenuItem::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    // ===== Separador =====
    if (separator) {
        SDL_SetRenderDrawColor(renderer,
            t.border.r, t.border.g, t.border.b, 255);
        int y = abs.y + abs.h / 2;
        SDL_RenderDrawLine(renderer, abs.x + 8, y, abs.x + abs.w - 8, y);
        clearDirty();
        return;
    }
    
    // ===== Fondo según estado =====
    if (item_enabled) {
        if (mouse_down) {
            KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
                .radius(0)
                .fill(t.accent_pressed)
                .draw(renderer);
        } else if (mouse_inside) {
            KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
                .radius(0)
                .fill(t.accent_hover)
                .draw(renderer);
        }
    }
    
    // ===== Texto =====
    Color text_color;
    if (!item_enabled) {
        text_color = t.text_disabled;
    } else if (mouse_inside || mouse_down) {
        text_color = t.text_on_accent;
    } else {
        text_color = t.text_primary;
    }
    
    updateTextTexture(renderer, text_color);
    
    if (text_texture) {
        int text_x = abs.x + 12;
        int text_y = abs.y + (abs.h - tex_h) / 2;
        SDL_Rect text_rect = { text_x, text_y, tex_w, tex_h };
        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    }
    
    clearDirty();
}

// ============================================================
// KitsuContextMenu
// ============================================================
KitsuContextMenu* KitsuContextMenu::s_active_menu = nullptr;

KitsuContextMenu::KitsuContextMenu(int width) {
    menu_width = width;
    bounds = {0, 0, menu_width, 0};
    requested_w = menu_width;
    requested_h = 0;
    
    visible = false;
    open = false;
    parent = nullptr;   // A propósito: coordenadas absolutas
}

KitsuContextMenu::~KitsuContextMenu() {
    for (auto* item : owned_items) delete item;
    owned_items.clear();
    items.clear();
    
    if (s_active_menu == this) {
        s_active_menu = nullptr;
    }
}

void KitsuContextMenu::clearItems() {
    for (auto* item : owned_items) delete item;
    owned_items.clear();
    items.clear();
    relayout();
}

KitsuMenuItem* KitsuContextMenu::addItem(const std::string& text,
                                         std::function<void()> cb) {
    auto* item = new KitsuMenuItem(text, cb);
    item->setSeparator(false);
    item->parent = this;                    // ← NUEVO
    items.push_back(item);
    owned_items.push_back(item);
    relayout();
    return item;
}

KitsuMenuItem* KitsuContextMenu::addSeparator() {
    auto* sep = new KitsuMenuItem("", nullptr);
    sep->setSeparator(true);
    sep->setEnabled(false);
    sep->parent = this;                     // ← NUEVO
    items.push_back(sep);
    owned_items.push_back(sep);
    relayout();
    return sep;
}

void KitsuContextMenu::relayout() {
    int current_y = padding_v;
    for (auto* item : items) {
        int h = item->isSeparator() ? separator_height : item_height;
        item->setBounds(padding_h, current_y,
                        menu_width - padding_h * 2, h);
        current_y += h;
    }
    int new_height = current_y + padding_v;
    setBoundsInternal(bounds.x, bounds.y, menu_width, new_height);   // ← aquí
}

void KitsuContextMenu::showAt(int x, int y) {
    if (s_active_menu && s_active_menu != this) {
        s_active_menu->hide();
    }
    
    setBoundsInternal(x, y, menu_width, bounds.h);   // ← aquí
    visible = true;
    open = true;
    ignore_next_up_ = true;
    
    relayout();
    setBoundsInternal(x, y, menu_width, bounds.h);   // ← y aquí
    
    s_active_menu = this;
    markDirty();
    
    if (g_renderer) g_renderer->markAllDirty();
}
void KitsuContextMenu::hide() {
    if (!open && !visible) return;
    visible = false;
    open = false;
    ignore_next_up_ = false;
    
    // Limpiar estados de hover/down de los items
    for (auto* item : items) {
        item->markDirty();
    }
    
    if (s_active_menu == this) {
        s_active_menu = nullptr;
    }
    markDirty();
    if (g_renderer) g_renderer->markAllDirty();
}

void KitsuContextMenu::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    KitsuTheme& t = KitsuTheme::current();
    
    // ===== Sombra sutil (offset) =====
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 40);
    SDL_Rect shadow = { abs.x + 2, abs.y + 2, abs.w, abs.h };
    SDL_RenderFillRect(renderer, &shadow);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // ===== Fondo =====
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(t.radius_panel)
        .fill(t.bg_secondary)
        .draw(renderer);
    
    // ===== Borde =====
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(t.radius_panel)
        .fill(t.border)
        .drawOutline(renderer, 1.0f);
    
    // ===== Items =====
    for (auto* item : items) {
        if (item) item->render(renderer);
    }
    
    clearDirty();
}

bool KitsuContextMenu::handleEvent(const SDL_Event& e) {
    if (!open) return false;
    
    // ===== Ignorar el MOUSEBUTTONUP del click que abrió el menú =====
    if (ignore_next_up_) {
        if (e.type == SDL_MOUSEBUTTONUP) {
            ignore_next_up_ = false;
        }
        return true;   // consumir mientras ignoramos
    }
    
    SDL_Rect abs = getAbsoluteBounds();
    
    switch (e.type) {
        // ===== ESC → cerrar =====
        case SDL_KEYDOWN: {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                hide();
                return true;
            }
            return true;   // consumir todo el teclado mientras esté abierto
        }
        
        // ===== Click: dentro → items, fuera → cerrar =====
        case SDL_MOUSEBUTTONDOWN: {
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (!inside) {
                hide();
                return true;   // consumir: que no llegue al root
            }
            // Dentro: propagar a items (de arriba a abajo)
            for (auto it = items.rbegin(); it != items.rend(); ++it) {
                if (*it && (*it)->handleEvent(e)) return true;
            }
            return true;
        }
        
        case SDL_MOUSEBUTTONUP: {
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (!inside) {
                return true;   // consumir sin hacer nada
            }
            for (auto it = items.rbegin(); it != items.rend(); ++it) {
                if (*it && (*it)->handleEvent(e)) return true;
            }
            return true;
        }
        
        case SDL_MOUSEMOTION: {
            for (auto* item : items) {
                if (item) item->handleEvent(e);
            }
            return true;
        }
        
        case SDL_MOUSEWHEEL: {
            return true;   // consumir
        }
        
        default:
            return true;   // consumir TODO mientras el menú esté abierto
    }
}

} // namespace KitsuGui
