#include "kitsugui/dropdown.h"
#include "kitsugui/context_menu.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "internal.h"

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuDropdown::KitsuDropdown(int width) {
    desired_w = width;
    desired_h = 36;
    bounds = {0, 0, width, 36};

    menu_ = new KitsuContextMenu(width);
}

KitsuDropdown::~KitsuDropdown() {
    destroyTexture();
    if (menu_) {
        delete menu_;
        menu_ = nullptr;
    }
}

// ============================================================
// Opciones
// ============================================================
KitsuDropdown* KitsuDropdown::add(const std::string& text,
                                 std::function<void()> cb) {
    Item item;
    item.text = text;
    item.callback = cb;
    items_.push_back(item);

    rebuildMenu();
    return this;
}

KitsuDropdown* KitsuDropdown::addItems(const std::vector<std::string>& new_items) {
    for (const auto& t : new_items) {
        add(t);
    }
    return this;
}

KitsuDropdown* KitsuDropdown::clear() {
    items_.clear();
    selected_index_ = -1;
    rebuildMenu();
    invalidate();
    return this;
}

// ============================================================
// Reconstruir menú — arregla el bug #12
// ============================================================
void KitsuDropdown::rebuildMenu() {
    if (!menu_) return;

    // Destruir el viejo y crear uno nuevo
    delete menu_;
    menu_ = new KitsuContextMenu(desired_w);

    for (size_t i = 0; i < items_.size(); i++) {
        int idx = (int)i;
        std::string text = items_[i].text;

        menu_->addItem(text, [this, idx, text]() {
            selectedIndex(idx);
            if (items_[idx].callback) items_[idx].callback();
            menu_open_ = false;
        });
    }
}

// ============================================================
// Selección
// ============================================================
std::string KitsuDropdown::selectedText() const {
    if (selected_index_ < 0 || selected_index_ >= (int)items_.size()) {
        return "";
    }
    return items_[selected_index_].text;
}

KitsuDropdown* KitsuDropdown::selectedIndex(int index) {
    if (index < -1 || index >= (int)items_.size()) return this;
    if (index == selected_index_) return this;

    selected_index_ = index;
    invalidate();

    if (on_change_) on_change_(selected_index_, selectedText());
    return this;
}

KitsuDropdown* KitsuDropdown::selectedText(const std::string& text) {
    for (int i = 0; i < (int)items_.size(); i++) {
        if (items_[i].text == text) {
            selectedIndex(i);
            return this;
        }
    }
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuDropdown* KitsuDropdown::font(TTF_Font* f) {
    font_ = f;
    invalidate();
    return this;
}

KitsuDropdown* KitsuDropdown::placeholder(const std::string& text) {
    placeholder_ = text;
    invalidate();
    return this;
}

KitsuDropdown* KitsuDropdown::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    if (menu_) {
        // Recrear el menú con el nuevo ancho
        rebuildMenu();
    }
    invalidate();
    return this;
}

// ============================================================
// Helpers internos
// ============================================================
TTF_Font* KitsuDropdown::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuDropdown::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuDropdown::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = activeFont();
    if (!active || !renderer) return;

    std::string display = selectedText();
    bool is_placeholder = false;

    if (display.empty()) {
        display = placeholder_;
        is_placeholder = true;
    }

    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;

    if (is_placeholder) {
        KitsuTheme& t = KitsuTheme::active();
        r = (Uint8)t.text_secondary.r;
        g = (Uint8)t.text_secondary.g;
        b = (Uint8)t.text_secondary.b;
    }

    if (text_texture_ &&
        cached_text_ == display &&
        cached_font_ == active &&
        cached_r_ == r && cached_g_ == g && cached_b_ == b) {
        return;
    }

    destroyTexture();

    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = TTF_RenderUTF8_Blended(active, display.c_str(), fg);
    if (!surface) return;

    text_texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w_ = surface->w;
    tex_h_ = surface->h;
    SDL_FreeSurface(surface);

    cached_text_ = display;
    cached_font_ = active;
    cached_r_ = r;
    cached_g_ = g;
    cached_b_ = b;
}

// ============================================================
// Eventos
// ============================================================
bool KitsuDropdown::handleEvent(const SDL_Event& e) {
    if (!enabled || !visible) return false;

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
            return false;
        }

        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);

            if (inside && menu_) {
                menu_open_ = true;
                menu_->showAt(r.x, r.y + r.h);
                invalidate();
                return true;
            }
            break;
        }
    }

    return false;
}

// ============================================================
// Tick
// ============================================================
void KitsuDropdown::tick() {
    // Sincronizar el estado si el menú se cerró externamente
    if (menu_open_ && menu_ && !menu_->isOpen()) {
        menu_open_ = false;
        invalidate();
    }
}

// ============================================================
// Render
// ============================================================
void KitsuDropdown::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    Color bg     = enabled ? t.bg_tertiary : t.bg_disabled;
    Color border = !enabled ? t.border_disabled :
                   menu_open_ ? t.border_focus :
                   mouse_inside_ ? t.accent : t.border;
    Color fg     = enabled ? t.text_primary : t.text_disabled;

    // ===== 1. Fondo =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_button)
        .fill(bg)
        .draw(renderer);

    // ===== 2. Borde =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_button)
        .fill(border)
        .drawOutline(renderer, (float)t.border_thickness_button);

    // ===== 3. Texto =====
    updateTextTexture(renderer, fg);
    if (text_texture_ && tex_w_ > 0) {
        int tx = r.x + 12;
        int ty = r.y + (r.h - tex_h_) / 2;

        int max_text_w = r.w - 40;
        if (tex_w_ > max_text_w) {
            SDL_Rect src = { 0, 0, max_text_w, tex_h_ };
            SDL_Rect dst = { tx, ty, max_text_w, tex_h_ };
            SDL_RenderCopy(renderer, text_texture_, &src, &dst);
        } else {
            SDL_Rect dst = { tx, ty, tex_w_, tex_h_ };
            SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);
        }
    }

    // ===== 4. Flecha ▼ =====
    int arrow_x = r.x + r.w - 20;
    int arrow_y = r.y + r.h / 2 - 4;
    int arrow_size = 8;

    SDL_SetRenderDrawColor(renderer,
        (Uint8)border.r, (Uint8)border.g, (Uint8)border.b, 255);

    for (int i = 0; i < arrow_size; i++) {
        SDL_RenderDrawLine(renderer,
            arrow_x + i, arrow_y + i,
            arrow_x + arrow_size * 2 - i, arrow_y + i);
    }

    clearNeedsRender();
}

} // namespace KitsuGui
