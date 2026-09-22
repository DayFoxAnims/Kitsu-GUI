#ifndef KITSUGUI_DROPDOWN_H
#define KITSUGUI_DROPDOWN_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>

namespace KitsuGui {

class KitsuContextMenu;

// ============================================================
// KitsuDropdown
// ============================================================
// Uso mínimo:
//   auto* dd = new KitsuDropdown();
//   dd->placeholder("Selecciona...");
//   dd->add("Opción 1");
//   dd->add("Opción 2");
//   dd->onChange([](int i, const std::string& t) { ... });
//
// Fluent:
//   dd->add("A")->add("B")->add("C");
// ============================================================
class KitsuDropdown : public KitsuWidget {
public:
    explicit KitsuDropdown(int width = 200);
    ~KitsuDropdown() override;

    // ===== Opciones =====
    KitsuDropdown* add(const std::string& text,
                      std::function<void()> cb = nullptr);
    KitsuDropdown* addItems(const std::vector<std::string>& items);
    KitsuDropdown* clear();

    // ===== Selección =====
    int selectedIndex() const { return selected_index_; }
    std::string selectedText() const;
    KitsuDropdown* selectedIndex(int i);
    KitsuDropdown* selectedText(const std::string& text);

    // ===== Apariencia =====
    KitsuDropdown* font(TTF_Font* f);
    KitsuDropdown* placeholder(const std::string& text);
    KitsuDropdown* size(int w, int h);

    // ===== Callbacks =====
    KitsuDropdown* onChange(std::function<void(int, const std::string&)> cb) {
        on_change_ = std::move(cb);
        return this;
    }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;

    // ===== Acceso =====
    KitsuContextMenu* menu() const { return menu_; }

private:
    struct Item {
        std::string text;
        std::function<void()> callback;
    };
    std::vector<Item> items_;

    int selected_index_ = -1;
    std::string placeholder_ = "Seleccionar...";

    TTF_Font* font_ = nullptr;
    std::function<void(int, const std::string&)> on_change_;

    bool mouse_inside_ = false;
    bool menu_open_ = false;
    bool manual_size_ = false;

    KitsuContextMenu* menu_ = nullptr;

    // Caché
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    TTF_Font* activeFont() const;
    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);

    // Reconstruye el menú contextual desde items_
    void rebuildMenu();
};

} // namespace KitsuGui

#endif
