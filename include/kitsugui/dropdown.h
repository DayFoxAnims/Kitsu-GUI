#ifndef KITSUGUI_DROPDOWN_H
#define KITSUGUI_DROPDOWN_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include "kitsugui/theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>

namespace KitsuGui {

// Forward declarations
class KitsuContextMenu;
class KitsuMenuItem;

class KitsuDropdown : public KitsuWidget {
public:
    KitsuDropdown(int width = 200);
    ~KitsuDropdown() override;
    
    // ===== Opciones =====
    int addItem(const std::string& text,
                std::function<void()> cb = nullptr);
    void addItems(const std::vector<std::string>& items);
    void clearItems();
    
    // ===== Selección =====
    int getSelectedIndex() const { return selected_index; }
    std::string getSelectedText() const;
    void setSelectedIndex(int index);
    void setSelectedText(const std::string& text);
    
    // Callback cuando cambia la selección
    KitsuDropdown& withCallback(std::function<void(int, const std::string&)> cb) {
        on_change = cb;
        return *this;
    }
    
    // ===== Configuración fluida =====
    KitsuDropdown& withFont(TTF_Font* font);
    KitsuDropdown& withPlaceholder(const std::string& text);
    
    KitsuDropdown& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
    // Acceso al menú interno (para registrarlo como overlay)
    KitsuContextMenu* getMenu() const { return menu; }
    
private:
    // ===== Opciones =====
    struct Item {
        std::string text;
        std::function<void()> callback;
    };
    std::vector<Item> items;
    
    int selected_index = -1;
    std::string placeholder = "Seleccionar...";
    
    // ===== Configuración =====
    TTF_Font* font = nullptr;
    std::function<void(int, const std::string&)> on_change;
    bool mouse_inside = false;
    bool menu_open = false;
    
    // ===== Menú interno =====
    KitsuContextMenu* menu = nullptr;
    
    // ===== Caché de textura =====
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    TTF_Font* cached_font = nullptr;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    static TTF_Font* g_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void updateTextTexture(SDL_Renderer* renderer, Color color);
    void destroyTextTexture();
};

} // namespace KitsuGui

#endif
