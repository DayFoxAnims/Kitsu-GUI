#ifndef KITSUGUI_BUTTON_H
#define KITSUGUI_BUTTON_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include "kitsugui/shapes.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

enum class ButtonState {
    NORMAL,
    HOVER,
    PRESSED,
    DISABLED
};

class KitsuButton : public KitsuWidget {
public:
    KitsuButton(const std::string& text, int width = 120, int height = 40);
    ~KitsuButton() override;
    
    // Configuración fluida
    KitsuButton& disabled();
    KitsuButton& withCallback(std::function<void()> cb);
    KitsuButton& withTheme(Theme t);
    KitsuButton& withFont(TTF_Font* font);
    KitsuButton& withCorner(float radius);
    
    KitsuButton& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    KitsuButton& setPosition(int x, int y) {
        bounds.x = x;
        bounds.y = y;
        markDirty();
        return *this;
    }
    
    // Getters
    ButtonState getState() const { return state; }
    
    // Setters
    void setState(ButtonState s);
    
    // Overrides
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();
    
private:
    std::string text;
    ButtonState state = ButtonState::NORMAL;
    Theme theme = Theme::LIGHT;
    std::function<void()> callback;
    bool mouse_inside = false;
    TTF_Font* font = nullptr;
    float corner_radius = KitsuStyle::ButtonRadius;
    
    SDL_Texture* text_texture = nullptr;
    int cached_text_r = -1;
    int cached_text_g = -1;
    int cached_text_b = -1;
    std::string cached_text;
    TTF_Font* cached_font = nullptr;
    
    static TTF_Font* g_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void getColorsForState(Uint8& r, Uint8& g, Uint8& b,
                          Uint8& br, Uint8& bg, Uint8& bb,
                          Uint8& tr, Uint8& tg, Uint8& tb) const;
    void updateTextTexture(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b);
};

} // namespace KitsuGui

#endif
