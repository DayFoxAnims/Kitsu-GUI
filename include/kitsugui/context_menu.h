#ifndef KITSUGUI_CONTEXT_MENU_H
#define KITSUGUI_CONTEXT_MENU_H

#include "kitsugui/panel.h"
#include "kitsugui/theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <functional>

namespace KitsuGui {

// ============================================================
// Item individual del menú
// ============================================================
class KitsuMenuItem : public KitsuWidget {
public:
    KitsuMenuItem(const std::string& text, std::function<void()> cb = nullptr);
    ~KitsuMenuItem() override;
    
    void setText(const std::string& t);
    const std::string& getText() const { return text; }
    
    void setEnabled(bool e) { item_enabled = e; markDirty(); }
    bool isItemEnabled() const { return item_enabled; }
    
    // Separador (línea horizontal, no clickable)
    void setSeparator(bool s) { separator = s; }
    bool isSeparator() const { return separator; }
    
    KitsuMenuItem& withCallback(std::function<void()> cb) {
        callback = cb;
        return *this;
    }
    KitsuMenuItem& disabled() { setEnabled(false); return *this; }
    
    KitsuMenuItem& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
private:
    std::string text;
    std::function<void()> callback;
    bool item_enabled = true;
    bool separator = false;
    bool mouse_inside = false;
    bool mouse_down = false;
    TTF_Font* font = nullptr;
    
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

// ============================================================
// Menú contextual (patrón: captura global de eventos)
// ============================================================
class KitsuContextMenu : public KitsuWidget {
public:
    KitsuContextMenu(int width = 200);
    ~KitsuContextMenu() override;
    
    // ===== Items =====
    KitsuMenuItem* addItem(const std::string& text,
                           std::function<void()> cb = nullptr);
    KitsuMenuItem* addSeparator();
    void clearItems();
    
    // ===== Mostrar / ocultar =====
    void showAt(int x, int y);
    void hide();
    bool isOpen() const { return open; }
    
    // ===== Singleton del menú activo =====
    static KitsuContextMenu* getActive() { return s_active_menu; }
    
    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
private:
    int menu_width = 200;
    int item_height = 32;
    int separator_height = 9;
    int padding_v = 4;
    int padding_h = 4;
    
    bool open = false;
    bool ignore_next_up_ = false;
    
    std::vector<KitsuMenuItem*> items;
    std::vector<KitsuMenuItem*> owned_items;
    
    static KitsuContextMenu* s_active_menu;
    
    void relayout();
};

} // namespace KitsuGui

#endif
