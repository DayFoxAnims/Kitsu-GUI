#include "kitsugui/context_menu.h"
#include "kitsugui/fonts.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "internal.h"
#include <algorithm>

namespace KitsuGui {

// ============================================================
// KitsuMenuItem
// ============================================================
KitsuMenuItem::KitsuMenuItem(const std::string& text, std::function<void()> cb)
    : text_(text), callback_(std::move(cb)) {
    desired_w = 200;
    desired_h = 32;
    bounds = {0, 0, 200, 32};
}

KitsuMenuItem::~KitsuMenuItem() {
    destroyTexture();
}

KitsuMenuItem* KitsuMenuItem::text(const std::string& t) {
    if (text_ != t) {
        text_ = t;
        invalidate();
    }
    return this;
}

KitsuMenuItem* KitsuMenuItem::item_enabled(bool e) {
    item_enabled_ = e;
    invalidate();
    return this;
}

KitsuMenuItem* KitsuMenuItem::disable() {
    return item_enabled(false);
}

KitsuMenuItem* KitsuMenuItem::separator(bool s) {
    separator_ = s;
    invalidate();
    return this;
}

void KitsuMenuItem::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuMenuItem::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = KitsuFonts::normal();
    if (!active || text_.empty() || !renderer) return;

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

bool KitsuMenuItem::handleEvent(const SDL_Event& e) {
    if (!visible || !item_enabled_ || separator_) return false;

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
            invalidate();

            if (was_down && inside) {
                auto cb = callback_;
                if (KitsuContextMenu::active()) {
                    KitsuContextMenu::active()->hide();
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

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // Separador
    if (separator_) {
        SDL_SetRenderDrawColor(renderer,
            (Uint8)t.border.r, (Uint8)t.border.g, (Uint8)t.border.b, 255);
        int y = r.y + r.h / 2;
        SDL_RenderDrawLine(renderer, r.x + 8, y, r.x + r.w - 8, y);
        clearNeedsRender();
        return;
    }

    // Fondo según estado
    if (item_enabled_) {
        if (mouse_down_) {
            KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
                .fill(t.accent_pressed)
                .draw(renderer);
        } else if (mouse_inside_) {
            KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
                .fill(t.accent_hover)
                .draw(renderer);
        }
    }

    // Texto
    Color fg;
    if (!item_enabled_)    fg = t.text_disabled;
    else if (mouse_inside_ || mouse_down_) fg = t.text_on_accent;
    else                   fg = t.text_primary;

    updateTextTexture(renderer, fg);

    if (text_texture_) {
        int tx = r.x + 12;
        int ty = r.y + (r.h - tex_h_) / 2;
        SDL_Rect dst = { tx, ty, tex_w_, tex_h_ };
        SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);
    }

    clearNeedsRender();
}

// ============================================================
// KitsuContextMenu
// ============================================================
KitsuContextMenu* KitsuContextMenu::s_active_ = nullptr;

KitsuContextMenu::KitsuContextMenu(int width)
    : menu_width_(width) {
    bounds = {0, 0, menu_width_, 0};
    desired_w = menu_width_;
    desired_h = 0;

    visible = false;
    open_ = false;
    parent = nullptr;   // coordenadas globales
}

KitsuContextMenu::~KitsuContextMenu() {
    for (auto* i : owned_) delete i;
    owned_.clear();
    items_.clear();

    if (s_active_ == this) s_active_ = nullptr;
}

KitsuContextMenu* KitsuContextMenu::clear() {
    for (auto* i : owned_) delete i;
    owned_.clear();
    items_.clear();
    relayout();
    return this;
}

KitsuMenuItem* KitsuContextMenu::addItem(const std::string& text,
                                        std::function<void()> cb) {
    auto* item = new KitsuMenuItem(text, std::move(cb));
    item->parent = this;
    items_.push_back(item);
    owned_.push_back(item);
    relayout();
    return item;
}

KitsuMenuItem* KitsuContextMenu::addSeparator() {
    auto* sep = new KitsuMenuItem("", nullptr);
    sep->separator(true);
    sep->item_enabled(false);
    sep->parent = this;
    items_.push_back(sep);
    owned_.push_back(sep);
    relayout();
    return sep;
}

void KitsuContextMenu::relayout() {
    int current_y = padding_v_;
    for (auto* item : items_) {
        int h = item->isSeparator() ? separator_height_ : item_height_;
        item->place(padding_h_, current_y,
                    menu_width_ - padding_h_ * 2, h);
        current_y += h;
    }
    int new_h = current_y + padding_v_;
    place(bounds.x, bounds.y, menu_width_, new_h);
}

KitsuContextMenu* KitsuContextMenu::showAt(int x, int y) {
    if (s_active_ && s_active_ != this) {
        s_active_->hide();
    }

    relayout();
    place(x, y, menu_width_, bounds.h);
    visible = true;
    open_ = true;
    ignore_next_up_ = true;

    s_active_ = this;
    invalidate();

    if (g_renderer) g_renderer->invalidate();
    return this;
}

KitsuContextMenu* KitsuContextMenu::hide() {
    if (!open_ && !visible) return this;

    visible = false;
    open_ = false;
    ignore_next_up_ = false;

    for (auto* item : items_) item->invalidate();

    if (s_active_ == this) s_active_ = nullptr;
    invalidate();
    if (g_renderer) g_renderer->invalidate();
    return this;
}

void KitsuContextMenu::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // Sombra
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 40);
    SDL_Rect shadow = { r.x + 2, r.y + 2, r.w, r.h };
    SDL_RenderFillRect(renderer, &shadow);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Fondo
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_panel)
        .fill(t.bg_secondary)
        .draw(renderer);

    // Borde
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_panel)
        .fill(t.border)
        .drawOutline(renderer, 1.0f);

    // Items
    for (auto* item : items_) {
        if (item) item->render(renderer);
    }

    clearNeedsRender();
}

bool KitsuContextMenu::handleEvent(const SDL_Event& e) {
    if (!open_) return false;

    // Ignorar el MOUSEBUTTONUP que abrió el menú
    if (ignore_next_up_) {
        if (e.type == SDL_MOUSEBUTTONUP) {
            ignore_next_up_ = false;
        }
        return true;
    }

    SDL_Rect r = globalRect();

    switch (e.type) {
        case SDL_KEYDOWN: {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                hide();
                return true;
            }
            return true;
        }

        case SDL_MOUSEBUTTONDOWN: {
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);

            if (!inside) {
                hide();
                return true;
            }
            for (auto it = items_.rbegin(); it != items_.rend(); ++it) {
                if (*it && (*it)->handleEvent(e)) return true;
            }
            return true;
        }

        case SDL_MOUSEBUTTONUP: {
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);

            if (!inside) return true;

            for (auto it = items_.rbegin(); it != items_.rend(); ++it) {
                if (*it && (*it)->handleEvent(e)) return true;
            }
            return true;
        }

        case SDL_MOUSEMOTION: {
            for (auto* item : items_) {
                if (item) item->handleEvent(e);
            }
            return true;
        }

        default:
            return true;
    }
}

} // namespace KitsuGui
