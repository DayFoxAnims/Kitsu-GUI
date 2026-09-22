#ifndef KITSUGUI_TEXT_INPUT_H
#define KITSUGUI_TEXT_INPUT_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

class KitsuContextMenu;

// ============================================================
// KitsuTextInput
// ============================================================
// Uso mínimo:
//   auto* ti = new KitsuTextInput();
//   ti->placeholder("Escribe aquí...");
//   ti->onEnter([](const std::string& t) { ... });
//
// Con texto inicial:
//   auto* ti = new KitsuTextInput("Hola");
//
// Fluent:
//   ti->placeholder("...")->onChange([](auto& s){...});
//
// El texto hace scroll horizontal automático cuando no cabe,
// manteniendo el caret siempre visible.
// ============================================================
class KitsuTextInput : public KitsuWidget {
public:
    explicit KitsuTextInput(const std::string& text = "");
    ~KitsuTextInput() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuTextInput* text(const std::string& t);
    KitsuTextInput* placeholder(const std::string& p);

    // ===== Estado =====
    bool isFocused() const { return focused_; }
    KitsuTextInput* focus(bool f);
    KitsuTextInput* unfocus();
    KitsuTextInput* disable();

    // ===== Apariencia =====
    KitsuTextInput* font(TTF_Font* f);
    KitsuTextInput* size(int w, int h);

    // ===== Edición =====
    KitsuTextInput* selectAll();
    KitsuTextInput* copyToClipboard();
    KitsuTextInput* cutToClipboard();
    KitsuTextInput* pasteFromClipboard();

    // ===== Callbacks =====
    KitsuTextInput* onChange(std::function<void(const std::string&)> cb) {
        on_change_ = std::move(cb);
        return this;
    }
    KitsuTextInput* onEnter(std::function<void(const std::string&)> cb) {
        on_enter_ = std::move(cb);
        return this;
    }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;

    // ===== Acceso global =====
    static KitsuTextInput* focused() { return s_focused_; }

private:
    std::string text_;
    std::string placeholder_;
    int cursor_pos_ = 0;

    bool focused_ = false;
    bool mouse_inside_ = false;
    bool mouse_down_ = false;
    bool disabled_ = false;
    bool manual_size_ = false;

    Uint32 last_blink_ = 0;
    bool caret_visible_ = true;

    TTF_Font* font_ = nullptr;

    std::function<void(const std::string&)> on_change_;
    std::function<void(const std::string&)> on_enter_;

    // Selección
    int selection_start_ = -1;
    int selection_end_ = -1;
    bool is_selecting_ = false;

    // Scroll horizontal (para texto largo)
    int scroll_x_ = 0;

    // Caché de textura
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    // Menú contextual (lazy)
    KitsuContextMenu* context_menu_ = nullptr;

    static KitsuTextInput* s_focused_;

    TTF_Font* activeFont() const;
    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);
    void autoSize();

    // UTF-8 helpers
    int prevUtf8Index(int i) const;
    int nextUtf8Index(int i) const;
    int textWidthUpTo(int i) const;
    int indexFromX(int x) const;

    // Scroll horizontal
    void updateScroll();

    // Selección
    bool hasSelection() const;
    void getSelectionRange(int& start, int& end) const;
    void clearSelection();
    void deleteSelection();
    void drawSelection(SDL_Renderer* renderer, SDL_Rect r);

    // Menú contextual (lazy)
    void ensureContextMenu();
    void showContextMenu(int x, int y);
};

} // namespace KitsuGui

#endif
