#include "kitsugui/text_input.h"
#include "kitsugui/context_menu.h"
#include "kitsugui/fonts.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"
#include "internal.h"
#include <cstring>
#include <algorithm>

namespace KitsuGui {

KitsuTextInput* KitsuTextInput::s_focused_ = nullptr;

// Padding interno del texto (a cada lado del input)
static const int TEXT_PAD = 8;

// ============================================================
// Constructor / destructor
// ============================================================
KitsuTextInput::KitsuTextInput(const std::string& text)
    : text_(text) {
    cursor_pos_ = (int)text.size();
    autoSize();
}

KitsuTextInput::~KitsuTextInput() {
    destroyTexture();
    if (focused_) {
        if (s_focused_ == this) s_focused_ = nullptr;
        if (g_active_animations > 0) g_active_animations--;
    }
    if (context_menu_) {
        delete context_menu_;
        context_menu_ = nullptr;
    }
}

// ============================================================
// Contenido
// ============================================================
KitsuTextInput* KitsuTextInput::text(const std::string& t) {
    if (text_ != t) {
        text_ = t;
        cursor_pos_ = (int)text_.size();
        clearSelection();
        updateScroll();
        invalidate();
        if (on_change_) on_change_(text_);
    }
    return this;
}

KitsuTextInput* KitsuTextInput::placeholder(const std::string& p) {
    placeholder_ = p;
    invalidate();
    return this;
}

// ============================================================
// Estado
// ============================================================
KitsuTextInput* KitsuTextInput::focus(bool f) {
    if (focused_ == f) return this;

    if (f) {
        if (s_focused_ && s_focused_ != this) {
            s_focused_->unfocus();
        }
        s_focused_ = this;
        focused_ = true;
        SDL_StartTextInput();
        last_blink_ = SDL_GetTicks();
        caret_visible_ = true;
        g_active_animations++;
        updateScroll();
    } else {
        if (s_focused_ == this) s_focused_ = nullptr;
        focused_ = false;
        SDL_StopTextInput();
        if (g_active_animations > 0) g_active_animations--;
        caret_visible_ = false;
        clearSelection();
        scroll_x_ = 0;
    }
    invalidate();
    return this;
}

KitsuTextInput* KitsuTextInput::unfocus() {
    return focus(false);
}

KitsuTextInput* KitsuTextInput::disable() {
    disabled_ = true;
    enabled = false;
    invalidate();
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuTextInput* KitsuTextInput::font(TTF_Font* f) {
    font_ = f;
    autoSize();
    updateScroll();
    invalidate();
    return this;
}

KitsuTextInput* KitsuTextInput::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    updateScroll();
    invalidate();
    return this;
}

// ============================================================
// Helpers internos
// ============================================================
TTF_Font* KitsuTextInput::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuTextInput::autoSize() {
    if (manual_size_) return;

    KitsuTheme& t = KitsuTheme::active();
    int line_h = t.text_input_height;

    TTF_Font* f = activeFont();
    if (f) {
        int th = TTF_FontHeight(f);
        if (th + 12 > line_h) line_h = th + 12;
    }

    desired_w = 250;
    desired_h = line_h;
    bounds.w = desired_w;
    bounds.h = desired_h;
}

void KitsuTextInput::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuTextInput::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = activeFont();
    if (!active || !renderer) return;

    std::string display = text_;
    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;

    // Placeholder si está vacío
    if (display.empty() && !placeholder_.empty()) {
        display = placeholder_;
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
    if (display.empty()) return;

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
// UTF-8 helpers
// ============================================================
int KitsuTextInput::prevUtf8Index(int i) const {
    if (i <= 0) return 0;
    i--;
    while (i > 0 && (text_[i] & 0xC0) == 0x80) i--;
    return i;
}

int KitsuTextInput::nextUtf8Index(int i) const {
    if (i >= (int)text_.size()) return (int)text_.size();
    i++;
    while (i < (int)text_.size() && (text_[i] & 0xC0) == 0x80) i++;
    return i;
}

int KitsuTextInput::textWidthUpTo(int i) const {
    TTF_Font* active = activeFont();
    if (!active || i <= 0) return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(active, text_.substr(0, i).c_str(), &w, &h);
    return w;
}

int KitsuTextInput::indexFromX(int x) const {
    TTF_Font* active = activeFont();
    if (!active) return 0;

    SDL_Rect r = globalRect();
    // El click está en x. La posición del texto empieza en r.x + TEXT_PAD.
    // El scroll desplaza el texto hacia la izquierda.
    int target = x - (r.x + TEXT_PAD) + scroll_x_;
    if (target <= 0) return 0;

    int i = 0;
    while (i < (int)text_.size()) {
        int next = nextUtf8Index(i);
        int w_next = textWidthUpTo(next);
        if (w_next > target) return i;
        i = next;
    }
    return (int)text_.size();
}

// ============================================================
// Scroll horizontal
// ============================================================
void KitsuTextInput::updateScroll() {
    if (!focused_) {
        scroll_x_ = 0;
        return;
    }

    // Área visible para el texto (dentro del padding)
    int view_w = bounds.w - TEXT_PAD * 2;
    if (view_w <= 0) {
        scroll_x_ = 0;
        return;
    }

    // Posición del caret dentro del texto (en píxeles)
    int caret_x = textWidthUpTo(cursor_pos_);

    // Ajustar scroll para que el caret sea visible
    if (caret_x - scroll_x_ < 0) {
        // Caret a la izquierda del área visible
        scroll_x_ = caret_x;
    } else if (caret_x - scroll_x_ > view_w) {
        // Caret a la derecha del área visible
        scroll_x_ = caret_x - view_w;
    }

    // Clamp inferior: no scrollear antes del inicio
    if (scroll_x_ < 0) scroll_x_ = 0;

    // Clamp superior: no scrollear más allá del final del texto
    int total_w = textWidthUpTo((int)text_.size());
    if (total_w <= view_w) {
        scroll_x_ = 0;
    } else {
        int max_scroll = total_w - view_w;
        if (scroll_x_ > max_scroll) scroll_x_ = max_scroll;
    }
}

// ============================================================
// Selección
// ============================================================
bool KitsuTextInput::hasSelection() const {
    return selection_start_ >= 0 && selection_end_ >= 0 &&
           selection_start_ != selection_end_;
}

void KitsuTextInput::getSelectionRange(int& start, int& end) const {
    if (selection_start_ <= selection_end_) {
        start = selection_start_;
        end = selection_end_;
    } else {
        start = selection_end_;
        end = selection_start_;
    }
}

void KitsuTextInput::clearSelection() {
    selection_start_ = -1;
    selection_end_ = -1;
    is_selecting_ = false;
}

KitsuTextInput* KitsuTextInput::selectAll() {
    if (text_.empty()) return this;
    selection_start_ = 0;
    selection_end_ = (int)text_.size();
    cursor_pos_ = (int)text_.size();
    updateScroll();
    invalidate();
    return this;
}

void KitsuTextInput::deleteSelection() {
    if (!hasSelection()) return;
    int start, end;
    getSelectionRange(start, end);
    text_.erase(start, end - start);
    cursor_pos_ = start;
    clearSelection();
    updateScroll();
    invalidate();
    if (on_change_) on_change_(text_);
}

KitsuTextInput* KitsuTextInput::copyToClipboard() {
    if (!hasSelection()) return this;
    int start, end;
    getSelectionRange(start, end);
    std::string sel = text_.substr(start, end - start);
    SDL_SetClipboardText(sel.c_str());
    return this;
}

KitsuTextInput* KitsuTextInput::cutToClipboard() {
    if (!hasSelection()) return this;
    copyToClipboard();
    deleteSelection();
    return this;
}

KitsuTextInput* KitsuTextInput::pasteFromClipboard() {
    if (!SDL_HasClipboardText()) return this;
    if (hasSelection()) deleteSelection();

    char* clip = SDL_GetClipboardText();
    if (clip) {
        text_.insert(cursor_pos_, clip);
        cursor_pos_ += (int)strlen(clip);
        SDL_free(clip);
        updateScroll();
        invalidate();
        if (on_change_) on_change_(text_);
    }
    return this;
}

// ============================================================
// Menú contextual (lazy)
// ============================================================
void KitsuTextInput::ensureContextMenu() {
    if (context_menu_) return;

    context_menu_ = new KitsuContextMenu(200);
    context_menu_->addItem("Copiar", [this]() { copyToClipboard(); });
    context_menu_->addItem("Cortar", [this]() { cutToClipboard(); });
    context_menu_->addItem("Pegar",  [this]() { pasteFromClipboard(); });
    context_menu_->addItem("Seleccionar todo", [this]() { selectAll(); });
}

void KitsuTextInput::showContextMenu(int x, int y) {
    ensureContextMenu();
    if (context_menu_) context_menu_->showAt(x, y);
}

// ============================================================
// Render de la selección (con clipping y scroll)
// ============================================================
void KitsuTextInput::drawSelection(SDL_Renderer* renderer, SDL_Rect r) {
    if (!hasSelection()) return;

    int start, end;
    getSelectionRange(start, end);

    int x1 = r.x + TEXT_PAD + textWidthUpTo(start) - scroll_x_;
    int x2 = r.x + TEXT_PAD + textWidthUpTo(end) - scroll_x_;

    // Área visible
    SDL_Rect clip = { r.x + TEXT_PAD, r.y, r.w - TEXT_PAD * 2, r.h };
    if (clip.w <= 0) return;

    SDL_RenderSetClipRect(renderer, &clip);

    SDL_Rect sel = { x1, r.y + 4, x2 - x1, r.h - 8 };

    KitsuTheme& t = KitsuTheme::active();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
        (Uint8)t.selection.r, (Uint8)t.selection.g,
        (Uint8)t.selection.b, (Uint8)t.selection.a);
    SDL_RenderFillRect(renderer, &sel);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    SDL_RenderSetClipRect(renderer, nullptr);
}

// ============================================================
// Eventos
// ============================================================
bool KitsuTextInput::handleEvent(const SDL_Event& e) {
    if (disabled_ || !visible || !enabled) return false;

    SDL_Rect r = globalRect();

    // ===== Click derecho: menú contextual =====
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
        int mx = e.button.x, my = e.button.y;
        bool inside = (mx >= r.x && mx < r.x + r.w &&
                       my >= r.y && my < r.y + r.h);
        if (inside) {
            focus(true);
            showContextMenu(mx, my);
            return true;
        }
    }

    // Si no tiene foco, solo procesamos clicks izquierdos
    if (!focused_) {
        switch (e.type) {
            case SDL_TEXTINPUT:
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                return false;
        }
    }

    switch (e.type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside) {
                focus(true);
                int new_pos = indexFromX(mx);
                cursor_pos_ = new_pos;
                selection_start_ = new_pos;
                selection_end_ = new_pos;
                is_selecting_ = true;
                mouse_down_ = true;
                updateScroll();
                last_blink_ = SDL_GetTicks();
                caret_visible_ = true;
                invalidate();
                return true;
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside != mouse_inside_) {
                mouse_inside_ = inside;
                invalidate();
            }

            if (is_selecting_ && mouse_down_) {
                int cx = mx;
                if (cx < r.x + TEXT_PAD) cx = r.x + TEXT_PAD;
                if (cx > r.x + r.w - TEXT_PAD) cx = r.x + r.w - TEXT_PAD;
                int new_pos = indexFromX(cx);
                selection_end_ = new_pos;
                cursor_pos_ = new_pos;
                updateScroll();
                invalidate();
                return true;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            if (is_selecting_) {
                is_selecting_ = false;
                mouse_down_ = false;
                if (selection_start_ == selection_end_) clearSelection();
                invalidate();
                return true;
            }
            mouse_down_ = false;
            break;
        }

        case SDL_TEXTINPUT: {
            if (hasSelection()) deleteSelection();
            text_.insert(cursor_pos_, e.text.text);
            cursor_pos_ += (int)strlen(e.text.text);
            updateScroll();
            invalidate();
            if (on_change_) on_change_(text_);
            return true;
        }

        case SDL_KEYDOWN: {
            SDL_Keycode key = e.key.keysym.sym;
            SDL_Keymod mod = (SDL_Keymod)e.key.keysym.mod;

            if (mod & KMOD_CTRL) {
                if (key == SDLK_v) { pasteFromClipboard(); return true; }
                if (key == SDLK_a) { selectAll(); return true; }
                if (key == SDLK_c) { copyToClipboard(); return true; }
                if (key == SDLK_x) { cutToClipboard(); return true; }
                break;
            }
            if (mod & KMOD_ALT) break;

            switch (key) {
                case SDLK_BACKSPACE: {
                    if (hasSelection()) deleteSelection();
                    else if (cursor_pos_ > 0) {
                        int prev = prevUtf8Index(cursor_pos_);
                        text_.erase(prev, cursor_pos_ - prev);
                        cursor_pos_ = prev;
                        updateScroll();
                        invalidate();
                        if (on_change_) on_change_(text_);
                    }
                    return true;
                }
                case SDLK_DELETE: {
                    if (hasSelection()) deleteSelection();
                    else if (cursor_pos_ < (int)text_.size()) {
                        int next = nextUtf8Index(cursor_pos_);
                        text_.erase(cursor_pos_, next - cursor_pos_);
                        updateScroll();
                        invalidate();
                        if (on_change_) on_change_(text_);
                    }
                    return true;
                }
                case SDLK_LEFT: {
                    if (hasSelection()) {
                        int s, en; getSelectionRange(s, en);
                        cursor_pos_ = s;
                        clearSelection();
                    } else if (cursor_pos_ > 0) {
                        cursor_pos_ = prevUtf8Index(cursor_pos_);
                    }
                    updateScroll();
                    last_blink_ = SDL_GetTicks();
                    caret_visible_ = true;
                    invalidate();
                    return true;
                }
                case SDLK_RIGHT: {
                    if (hasSelection()) {
                        int s, en; getSelectionRange(s, en);
                        cursor_pos_ = en;
                        clearSelection();
                    } else if (cursor_pos_ < (int)text_.size()) {
                        cursor_pos_ = nextUtf8Index(cursor_pos_);
                    }
                    updateScroll();
                    last_blink_ = SDL_GetTicks();
                    caret_visible_ = true;
                    invalidate();
                    return true;
                }
                case SDLK_HOME: {
                    cursor_pos_ = 0;
                    clearSelection();
                    updateScroll();
                    invalidate();
                    return true;
                }
                case SDLK_END: {
                    cursor_pos_ = (int)text_.size();
                    clearSelection();
                    updateScroll();
                    invalidate();
                    return true;
                }
                case SDLK_RETURN:
                case SDLK_KP_ENTER: {
                    if (on_enter_) on_enter_(text_);
                    return true;
                }
                default: break;
            }
            break;
        }
    }
    return false;
}

// ============================================================
// Tick
// ============================================================
void KitsuTextInput::tick() {
    if (!focused_) return;
    Uint32 now = SDL_GetTicks();
    if (now - last_blink_ >= 500) {
        caret_visible_ = !caret_visible_;
        last_blink_ = now;
        invalidate();
    }
}

// ============================================================
// Render
// ============================================================
void KitsuTextInput::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    Color bg     = disabled_ ? t.bg_disabled     : t.bg_tertiary;
    Color border = disabled_ ? t.border_disabled :
                   focused_  ? t.border_focus    :
                   mouse_inside_ ? t.accent       : t.border;
    Color fg     = disabled_ ? t.text_disabled : t.text_primary;

    // ===== 1. Fondo =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_text_input)
        .fill(bg)
        .draw(renderer);

    // ===== 2. Selección (ya clippeada internamente) =====
    drawSelection(renderer, r);

    // ===== 3. Borde =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(t.radius_text_input)
        .fill(border)
        .drawOutline(renderer, (float)t.border_thickness_text_input);

    // ===== 4. Texto (con clipping y scroll) =====
    int view_w = r.w - TEXT_PAD * 2;
    if (view_w > 0) {
        SDL_Rect clip = { r.x + TEXT_PAD, r.y, view_w, r.h };
        SDL_RenderSetClipRect(renderer, &clip);

        updateTextTexture(renderer, fg);
        if (text_texture_ && tex_w_ > 0) {
            int tx = r.x + TEXT_PAD - scroll_x_;
            int ty = r.y + (r.h - tex_h_) / 2;
            SDL_Rect dst = { tx, ty, tex_w_, tex_h_ };
            SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);
        }

        SDL_RenderSetClipRect(renderer, nullptr);
    }

    // ===== 5. Caret (también clippeado) =====
    if (focused_ && caret_visible_ && !hasSelection() && !disabled_) {
        if (view_w > 0) {
            SDL_Rect clip = { r.x + TEXT_PAD, r.y, view_w, r.h };
            SDL_RenderSetClipRect(renderer, &clip);

            int cx = r.x + TEXT_PAD + textWidthUpTo(cursor_pos_) - scroll_x_;
            int cy1 = r.y + 6;
            int cy2 = r.y + r.h - 6;

            SDL_SetRenderDrawColor(renderer,
                (Uint8)t.accent.r, (Uint8)t.accent.g, (Uint8)t.accent.b, 255);
            SDL_RenderDrawLine(renderer, cx, cy1, cx, cy2);
            SDL_RenderDrawLine(renderer, cx + 1, cy1, cx + 1, cy2);

            SDL_RenderSetClipRect(renderer, nullptr);
        }
    }

    clearNeedsRender();
}

} // namespace KitsuGui
