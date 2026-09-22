#ifndef KITSUGUI_CONTEXT_MENU_H
#define KITSUGUI_CONTEXT_MENU_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>

namespace KitsuGui {

// ============================================================
// KitsuMenuItem — item individual
// ============================================================
class KitsuMenuItem : public KitsuWidget {
public:
    explicit KitsuMenuItem(const std::string& text,
                          std::function<void()> cb = nullptr);
    ~KitsuMenuItem() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuMenuItem* text(const std::string& t);

    // ===== Estado =====
    bool isItemEnabled() const { return item_enabled_; }
    KitsuMenuItem* item_enabled(bool e);
    KitsuMenuItem* disable();

    // ===== Separador =====
    bool isSeparator() const { return separator_; }
    KitsuMenuItem* separator(bool s);

    // ===== Callbacks =====
    KitsuMenuItem* onClick(std::function<void()> cb) {
        callback_ = std::move(cb);
        return this;
    }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    std::string text_;
    std::function<void()> callback_;
    bool item_enabled_ = true;
    bool separator_ = false;
    bool mouse_inside_ = false;
    bool mouse_down_ = false;

    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);
};

// ============================================================
// KitsuContextMenu — menú emergente
// ============================================================
class KitsuContextMenu : public KitsuWidget {
public:
    explicit KitsuContextMenu(int width = 200);
    ~KitsuContextMenu() override;

    // ===== Items =====
    KitsuMenuItem* addItem(const std::string& text,
                          std::function<void()> cb = nullptr);
    KitsuMenuItem* addSeparator();
    KitsuContextMenu* clear();

    // ===== Mostrar / ocultar =====
    KitsuContextMenu* showAt(int x, int y);
    KitsuContextMenu* hide();
    bool isOpen() const { return open_; }

    // ===== Singleton del menú activo =====
    static KitsuContextMenu* active() { return s_active_; }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    int menu_width_ = 200;
    int item_height_ = 32;
    int separator_height_ = 9;
    int padding_v_ = 4;
    int padding_h_ = 4;

    bool open_ = false;
    bool ignore_next_up_ = false;

    std::vector<KitsuMenuItem*> items_;
    std::vector<KitsuMenuItem*> owned_;

    static KitsuContextMenu* s_active_;

    void relayout();
};

} // namespace KitsuGui

#endif
