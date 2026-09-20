#ifndef KITSUGUI_CHECK_BOX_H
#define KITSUGUI_CHECK_BOX_H

#include "kitsugui/widget.h"
#include "kitsugui/theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

class KitsuCheckBox : public KitsuWidget {
public:
    KitsuCheckBox(const std::string& text = "", bool checked = false);
    ~KitsuCheckBox() override;
    
    KitsuCheckBox& withFont(TTF_Font* font);
    KitsuCheckBox& withChecked(bool c);
    KitsuCheckBox& withCallback(std::function<void(bool)> cb);
    KitsuCheckBox& disabled();
    KitsuCheckBox& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    bool isChecked() const { return checked; }
    void setChecked(bool c);
    void toggle() { setChecked(!checked); }
    
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
private:
    std::string text;
    bool checked = false;
    bool mouse_inside = false;
    bool mouse_down = false;
    bool enabled_ = true;
    TTF_Font* font = nullptr;
    std::function<void(bool)> callback;
    
    // Caché
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    TTF_Font* cached_font = nullptr;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    static TTF_Font* g_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void updateTextTexture(SDL_Renderer* renderer);
    void destroyTextTexture();
};

} // namespace KitsuGui

#endif
