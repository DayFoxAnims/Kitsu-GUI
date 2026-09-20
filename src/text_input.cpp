#include "kitsugui/text_input.h"
#include "kitsugui/context_menu.h"
#include "internal.h"

namespace KitsuGui {

TTF_Font* KitsuTextInput::g_font = nullptr;
KitsuTextInput* KitsuTextInput::s_focused = nullptr;

// ===== DEBUG =====
static bool g_debug_ti = false;
#define TI_LOG(...) do { if (g_debug_ti) { \
    char _buf[256]; snprintf(_buf, sizeof(_buf), __VA_ARGS__); \
    SDL_Log("[TextInput %p] %s", (void*)this, _buf); } } while(0)

// ===== CONSTRUCTOR / DESTRUCTOR =====
KitsuTextInput::KitsuTextInput(const std::string& text)
    : text(text) {
    bounds = {0, 0, 250, 32};
    requested_w = 250;
    requested_h = 32;
    cursor_pos = (int)text.size();
    updateColorsFromTheme();
    
    TI_LOG("CREADO");
    
    // Crear el menú contextual propio
    createContextMenu();
}

KitsuTextInput::~KitsuTextInput() {
    TI_LOG("DESTRUIDO");
    destroyTextTexture();
    if (focused) {
        if (s_focused == this) s_focused = nullptr;
        if (g_active_animations > 0) g_active_animations--;
    }
    
    if (context_menu) {
        delete context_menu;
        context_menu = nullptr;
    }
}

void KitsuTextInput::destroyTextTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

void KitsuTextInput::updateColorsFromTheme() {
    KitsuTheme& t = KitsuTheme::current();
    bg_color = t.bg_tertiary;
    text_color = t.text_primary;
    border_color = t.border;
    placeholder_color = t.text_secondary;
    selection_color = t.selection;
}

// ===== MENÚ CONTEXTUAL =====
void KitsuTextInput::createContextMenu() {
    TI_LOG("createContextMenu() INICIO");
    
    context_menu = new KitsuContextMenu(200);
    
    if (!context_menu) {
        TI_LOG("  ERROR: no se pudo crear el menú");
        return;
    }
    
    TI_LOG("  Menú creado en %p", (void*)context_menu);
    
    context_menu->addItem("Copiar", [this]() {
        TI_LOG("  → Copiar");
        this->copyToClipboard();
    });
    context_menu->addItem("Cortar", [this]() {
        TI_LOG("  → Cortar");
        this->cutToClipboard();
    });
    context_menu->addItem("Pegar", [this]() {
        TI_LOG("  → Pegar");
        this->pasteFromClipboard();
    });
    context_menu->addItem("Seleccionar todo", [this]() {
        TI_LOG("  → Seleccionar todo");
        this->selectAll();
    });
    
    TI_LOG("  Menú configurado con 4 items");
    TI_LOG("createContextMenu() FIN");
}

void KitsuTextInput::showContextMenu(int x, int y) {
    TI_LOG("showContextMenu(%d, %d)", x, y);
    
    if (!context_menu) {
        TI_LOG("  ERROR: context_menu es nullptr");
        return;
    }
    
    TI_LOG("  Menú: %p, llamando a showAt...", (void*)context_menu);
    context_menu->showAt(x, y);
    TI_LOG("  showAt() retornó");
}

// ===== CONFIGURACIÓN FLUIDA =====
KitsuTextInput& KitsuTextInput::withFont(TTF_Font* f) {
    font = f;
    markDirty();
    return *this;
}

KitsuTextInput& KitsuTextInput::withText(const std::string& t) {
    setText(t);
    return *this;
}

KitsuTextInput& KitsuTextInput::withPlaceholder(const std::string& t) {
    placeholder = t;
    markDirty();
    return *this;
}

KitsuTextInput& KitsuTextInput::withCallback(std::function<void(const std::string&)> cb) {
    callback = cb;
    return *this;
}

KitsuTextInput& KitsuTextInput::withOnEnter(std::function<void(const std::string&)> cb) {
    on_enter = cb;
    return *this;
}

KitsuTextInput& KitsuTextInput::withTheme(Theme t) {
    theme = t;
    updateColorsFromTheme();
    markDirty();
    return *this;
}

KitsuTextInput& KitsuTextInput::setBounds(int x, int y, int w, int h) {
    setBoundsInternal(x, y, w, h);
    return *this;
}

void KitsuTextInput::setText(const std::string& t) {
    text = t;
    cursor_pos = (int)text.size();
    clearSelection();
    markDirty();
    if (callback) callback(text);
}

// ===== FOCUS =====
void KitsuTextInput::setFocus(bool f) {
    if (focused == f) return;
    
    if (f) {
        if (s_focused && s_focused != this) {
            s_focused->unfocus();
        }
        s_focused = this;
        focused = true;
        SDL_StartTextInput();
        last_blink = SDL_GetTicks();
        caret_visible = true;
        g_active_animations++;
        TI_LOG("FOCUS ON");
    } else {
        if (s_focused == this) s_focused = nullptr;
        focused = false;
        SDL_StopTextInput();
        if (g_active_animations > 0) g_active_animations--;
        caret_visible = false;
        clearSelection();
        TI_LOG("FOCUS OFF");
    }
    markDirty();
}

void KitsuTextInput::unfocus() {
    setFocus(false);
}

// ===== API PÚBLICA DE EDICIÓN =====
void KitsuTextInput::copyToClipboard() {
    if (!hasSelection()) return;
    int start, end;
    getSelectionRange(start, end);
    std::string selected = text.substr(start, end - start);
    SDL_SetClipboardText(selected.c_str());
}

void KitsuTextInput::cutToClipboard() {
    if (!hasSelection()) return;
    copyToClipboard();
    deleteSelection();
}

void KitsuTextInput::pasteFromClipboard() {
    if (!SDL_HasClipboardText()) return;
    if (hasSelection()) deleteSelection();
    char* clip = SDL_GetClipboardText();
    if (clip) {
        text.insert(cursor_pos, clip);
        cursor_pos += (int)strlen(clip);
        SDL_free(clip);
        markDirty();
        if (callback) callback(text);
    }
}

// ===== UTF-8 HELPERS =====
int KitsuTextInput::prevUtf8Index(int i) const {
    if (i <= 0) return 0;
    i--;
    while (i > 0 && (text[i] & 0xC0) == 0x80) i--;
    return i;
}

int KitsuTextInput::nextUtf8Index(int i) const {
    if (i >= (int)text.size()) return (int)text.size();
    i++;
    while (i < (int)text.size() && (text[i] & 0xC0) == 0x80) i++;
    return i;
}

int KitsuTextInput::textWidthUpTo(int i) const {
    TTF_Font* active = getActiveFont();
    if (!active || i <= 0) return 0;
    int w = 0, h = 0;
    TTF_SizeUTF8(active, text.substr(0, i).c_str(), &w, &h);
    return w;
}

int KitsuTextInput::indexFromX(int x) const {
    TTF_Font* active = getActiveFont();
    if (!active) return 0;
    int target = x - (bounds.x + 8);
    if (target <= 0) return 0;
    
    int i = 0;
    while (i < (int)text.size()) {
        int next = nextUtf8Index(i);
        int w_next = textWidthUpTo(next);
        if (w_next > target) return i;
        i = next;
    }
    return (int)text.size();
}

// ===== SELECCIÓN =====
bool KitsuTextInput::hasSelection() const {
    return selection_start >= 0 && selection_end >= 0 && selection_start != selection_end;
}

void KitsuTextInput::getSelectionRange(int& start, int& end) const {
    if (selection_start <= selection_end) {
        start = selection_start;
        end = selection_end;
    } else {
        start = selection_end;
        end = selection_start;
    }
}

void KitsuTextInput::clearSelection() {
    selection_start = -1;
    selection_end = -1;
    is_selecting = false;
}

void KitsuTextInput::selectAll() {
    if (text.empty()) return;
    selection_start = 0;
    selection_end = (int)text.size();
    cursor_pos = (int)text.size();
    markDirty();
}

void KitsuTextInput::deleteSelection() {
    if (!hasSelection()) return;
    int start, end;
    getSelectionRange(start, end);
    text.erase(start, end - start);
    cursor_pos = start;
    clearSelection();
    markDirty();
    if (callback) callback(text);
}

// ===== TEXTURA =====
void KitsuTextInput::updateTextTexture(SDL_Renderer* renderer) {
    TTF_Font* active = getActiveFont();
    if (!active || !renderer) return;
    
    std::string display = text;
    Uint8 r = text_color.r, g = text_color.g, b = text_color.b;
    
    if (display.empty() && !placeholder.empty()) {
        display = placeholder;
        r = placeholder_color.r;
        g = placeholder_color.g;
        b = placeholder_color.b;
    }
    
    if (text_texture &&
        cached_text == display &&
        cached_font == active &&
        cached_r == r && cached_g == g && cached_b == b) {
        return;
    }
    
    destroyTextTexture();
    if (display.empty()) return;
    
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

// ===== RENDER DE LA SELECCIÓN =====
void KitsuTextInput::drawSelection(SDL_Renderer* renderer, SDL_Rect abs) {
    if (!hasSelection()) return;
    
    int start, end;
    getSelectionRange(start, end);
    
    int x1 = abs.x + 8 + textWidthUpTo(start);
    int x2 = abs.x + 8 + textWidthUpTo(end);
    
    SDL_Rect sel_rect = {
        x1,
        abs.y + 4,
        x2 - x1,
        abs.h - 8
    };
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,
        selection_color.r, selection_color.g, selection_color.b, selection_color.a);
    SDL_RenderFillRect(renderer, &sel_rect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// ===== EVENTOS =====
bool KitsuTextInput::handleEvent(const SDL_Event& e) {
    // DEBUG: log SOLO eventos de mouse para no saturar
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        SDL_Rect abs = getAbsoluteBounds();
        TI_LOG("MOUSEBUTTON event=%d button=%d pos=(%d,%d) bounds=(%d,%d %dx%d)",
               e.type, e.button.button,
               e.button.x, e.button.y,
               abs.x, abs.y, abs.w, abs.h);
    }
    
    if (!enabled_ || !visible) {
        if (e.type == SDL_MOUSEBUTTONDOWN) TI_LOG("  SKIP: not enabled/visible");
        return false;
    }
    
    // ===== Click derecho: mostrar menú contextual =====
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
        SDL_Rect abs = getAbsoluteBounds();
        int mx = e.button.x;
        int my = e.button.y;
        bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                       my >= abs.y && my < abs.y + abs.h);
        
        TI_LOG("CLICK DERECHO: inside=%d", (int)inside);
        
        if (inside) {
            TI_LOG("  → setFocus(true)");
            setFocus(true);
            
            TI_LOG("  → showContextMenu(%d, %d)", mx, my);
            showContextMenu(mx, my);
            return true;
        }
    }
    
    // Solo procesar teclado si tenemos focus
    if (!focused) {
        switch (e.type) {
            case SDL_TEXTINPUT:
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                return false;
        }
    }
    
    SDL_Rect abs = getAbsoluteBounds();
    
    switch (e.type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (inside) {
                setFocus(true);
                
                int new_pos = indexFromX(mx);
                cursor_pos = new_pos;
                selection_start = new_pos;
                selection_end = new_pos;
                is_selecting = true;
                mouse_down = true;
                
                last_blink = SDL_GetTicks();
                caret_visible = true;
                markDirty();
                return true;
            }
            break;
        }
        
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x, my = e.motion.y;
            
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            if (inside != mouse_inside) {
                mouse_inside = inside;
                markDirty();
            }
            
            if (is_selecting && mouse_down) {
                int clamped_x = mx;
                if (clamped_x < abs.x + 8) clamped_x = abs.x + 8;
                if (clamped_x > abs.x + abs.w - 8) clamped_x = abs.x + abs.w - 8;
                
                int new_pos = indexFromX(clamped_x);
                selection_end = new_pos;
                cursor_pos = new_pos;
                markDirty();
                return true;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            
            if (is_selecting) {
                is_selecting = false;
                mouse_down = false;
                
                if (selection_start == selection_end) {
                    clearSelection();
                }
                
                markDirty();
                return true;
            }
            mouse_down = false;
            break;
        }
        
        case SDL_TEXTINPUT: {
            if (hasSelection()) {
                deleteSelection();
            }
            
            text.insert(cursor_pos, e.text.text);
            cursor_pos += (int)strlen(e.text.text);
            markDirty();
            if (callback) callback(text);
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
                    if (hasSelection()) {
                        deleteSelection();
                    } else if (cursor_pos > 0) {
                        int prev = prevUtf8Index(cursor_pos);
                        text.erase(prev, cursor_pos - prev);
                        cursor_pos = prev;
                        markDirty();
                        if (callback) callback(text);
                    }
                    return true;
                }
                
                case SDLK_DELETE: {
                    if (hasSelection()) {
                        deleteSelection();
                    } else if (cursor_pos < (int)text.size()) {
                        int next = nextUtf8Index(cursor_pos);
                        text.erase(cursor_pos, next - cursor_pos);
                        markDirty();
                        if (callback) callback(text);
                    }
                    return true;
                }
                
                case SDLK_LEFT: {
                    if (hasSelection()) {
                        int start, end;
                        getSelectionRange(start, end);
                        cursor_pos = start;
                        clearSelection();
                    } else if (cursor_pos > 0) {
                        cursor_pos = prevUtf8Index(cursor_pos);
                    }
                    last_blink = SDL_GetTicks();
                    caret_visible = true;
                    markDirty();
                    return true;
                }
                
                case SDLK_RIGHT: {
                    if (hasSelection()) {
                        int start, end;
                        getSelectionRange(start, end);
                        cursor_pos = end;
                        clearSelection();
                    } else if (cursor_pos < (int)text.size()) {
                        cursor_pos = nextUtf8Index(cursor_pos);
                    }
                    last_blink = SDL_GetTicks();
                    caret_visible = true;
                    markDirty();
                    return true;
                }
                
                case SDLK_HOME: {
                    cursor_pos = 0;
                    clearSelection();
                    last_blink = SDL_GetTicks();
                    caret_visible = true;
                    markDirty();
                    return true;
                }
                
                case SDLK_END: {
                    cursor_pos = (int)text.size();
                    clearSelection();
                    last_blink = SDL_GetTicks();
                    caret_visible = true;
                    markDirty();
                    return true;
                }
                
                case SDLK_RETURN:
                case SDLK_KP_ENTER: {
                    if (on_enter) on_enter(text);
                    return true;
                }
                
                default:
                    break;
            }
            break;
        }
    }
    return false;
}

// ===== TICK =====
void KitsuTextInput::tick() {
    if (!focused) return;
    
    Uint32 now = SDL_GetTicks();
    if (now - last_blink >= 500) {
        caret_visible = !caret_visible;
        last_blink = now;
        markDirty();
    }
}

// ===== RENDER =====
void KitsuTextInput::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    
    // 1. Fondo
    SDL_SetRenderDrawColor(renderer,
        (Uint8)bg_color.r, (Uint8)bg_color.g, (Uint8)bg_color.b, 255);
    SDL_RenderFillRect(renderer, &abs);
    
    // 2. Selección
    drawSelection(renderer, abs);
    
    // 3. Borde
    Uint8 br = (Uint8)border_color.r;
    Uint8 bg_ = (Uint8)border_color.g;
    Uint8 bb = (Uint8)border_color.b;
    if (focused) {
        KitsuTheme& t = KitsuTheme::current();
        br = t.border_focus.r;
        bg_ = t.border_focus.g;
        bb = t.border_focus.b;
    }
    
    SDL_SetRenderDrawColor(renderer, br, bg_, bb, 255);
    for (int i = 0; i < 2; i++) {
        SDL_Rect b = { abs.x + i, abs.y + i, abs.w - 2*i, abs.h - 2*i };
        SDL_RenderDrawRect(renderer, &b);
    }
    
    // 4. Texto
    updateTextTexture(renderer);
    if (text_texture && tex_w > 0) {
        int text_x = abs.x + 8;
        int text_y = abs.y + (abs.h - tex_h) / 2;
        
        SDL_Rect text_rect = { text_x, text_y, tex_w, tex_h };
        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    }
    
    // 5. Caret
    if (focused && caret_visible && !hasSelection()) {
        int caret_x = abs.x + 8 + textWidthUpTo(cursor_pos);
        int caret_y1 = abs.y + 6;
        int caret_y2 = abs.y + abs.h - 6;
        
        KitsuTheme& t = KitsuTheme::current();
        SDL_SetRenderDrawColor(renderer,
            t.accent.r, t.accent.g, t.accent.b, 255);
        SDL_RenderDrawLine(renderer, caret_x, caret_y1, caret_x, caret_y2);
        SDL_RenderDrawLine(renderer, caret_x + 1, caret_y1, caret_x + 1, caret_y2);
    }
    
    clearDirty();
}

} // namespace KitsuGui
