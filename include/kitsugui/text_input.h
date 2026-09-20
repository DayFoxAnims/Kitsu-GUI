#ifndef KITSUGUI_TEXT_INPUT_H
#define KITSUGUI_TEXT_INPUT_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>
#include <cstring>

namespace KitsuGui {

// Forward
class KitsuContextMenu;

class KitsuTextInput : public KitsuWidget {
public:
    KitsuTextInput(const std::string& text = "");
    ~KitsuTextInput() override;
    
    // ===== Configuración fluida =====
    KitsuTextInput& withFont(TTF_Font* font);
    KitsuTextInput& withText(const std::string& text);
    KitsuTextInput& withPlaceholder(const std::string& text);
    KitsuTextInput& withCallback(std::function<void(const std::string&)> cb);
    KitsuTextInput& withOnEnter(std::function<void(const std::string&)> cb);
    KitsuTextInput& withTheme(Theme t);
    KitsuTextInput& setBounds(int x, int y, int w, int h);
    
    // ===== Estado =====
    std::string getText() const { return text; }
    void setText(const std::string& t);
    bool isFocused() const { return focused; }
    void setFocus(bool f);
    void unfocus();
    
    // ===== API pública de edición =====
    void selectAll();
    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboard();
    
    // ===== Acceso al menú contextual =====
    KitsuContextMenu* getContextMenu() const { return context_menu; }
    
    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    static KitsuTextInput* getFocused() { return s_focused; }
    
private:
    std::string text;
    std::string placeholder;
    int cursor_pos = 0;
    bool focused = false;
    bool mouse_inside = false;
    bool mouse_down = false;
    bool enabled_ = true;
    Uint32 last_blink = 0;
    bool caret_visible = true;
    TTF_Font* font = nullptr;
    std::function<void(const std::string&)> callback;
    std::function<void(const std::string&)> on_enter;
    Theme theme = Theme::LIGHT;
    
    int selection_start = -1;
    int selection_end = -1;
    bool is_selecting = false;
    
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    TTF_Font* cached_font = nullptr;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    Color bg_color;
    Color text_color;
    Color border_color;
    Color placeholder_color;
    Color selection_color = {100, 150, 255, 100};
    
    // ===== Menú contextual propio =====
    KitsuContextMenu* context_menu = nullptr;
    
    static TTF_Font* g_font;
    static KitsuTextInput* s_focused;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void updateTextTexture(SDL_Renderer* renderer);
    void destroyTextTexture();
    void updateColorsFromTheme();
    
    int prevUtf8Index(int i) const;
    int nextUtf8Index(int i) const;
    int textWidthUpTo(int i) const;
    int indexFromX(int x) const;
    
    bool hasSelection() const;
    void getSelectionRange(int& start, int& end) const;
    void clearSelection();
    void deleteSelection();
    void drawSelection(SDL_Renderer* renderer, SDL_Rect abs);
    
    // ===== Menú contextual =====
    void createContextMenu();
    void showContextMenu(int x, int y);
};

} // namespace KitsuGui

#endif
