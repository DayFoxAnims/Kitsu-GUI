#ifndef KITSUGUI_LABEL_H
#define KITSUGUI_LABEL_H

#include "kitsugui/widget.h"
#include "kitsugui/theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

enum class TextAlign {
    LEFT, CENTER, RIGHT
};

enum class TextVAlign {
    TOP, MIDDLE, BOTTOM
};

class KitsuLabel : public KitsuWidget {
public:
    KitsuLabel(const std::string& text, int width = 0, int height = 0);
    ~KitsuLabel() override;
    
    KitsuLabel& withFont(TTF_Font* font);
    KitsuLabel& withColor(Uint8 r, Uint8 g, Uint8 b);
    KitsuLabel& withAlign(TextAlign align);
    KitsuLabel& withVAlign(TextVAlign valign);
    KitsuLabel& withWrap(int max_width);
    
    KitsuLabel& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    KitsuLabel& at(int x, int y) {
        bounds.x = x; bounds.y = y; markDirty(); return *this;
    }
    KitsuLabel& size(int w, int h) {
        bounds.w = w; bounds.h = h; markDirty(); return *this;
    }
    
    void setText(const std::string& new_text);
    const std::string& getText() const { return text; }
    
    void render(SDL_Renderer* renderer) override;
    
    static void setDefaultFont(TTF_Font* font);
    static TTF_Font* getDefaultFont();
    
private:
    std::string text;
    TTF_Font* font = nullptr;
    Uint8 color_r = 0, color_g = 0, color_b = 0;
    TextAlign align = TextAlign::LEFT;
    TextVAlign valign = TextVAlign::TOP;
    int wrap_width = 0;
    
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    int cached_r = -1, cached_g = -1, cached_b = -1;
    int cached_wrap = -1;
    TTF_Font* cached_font = nullptr;
    
    static TTF_Font* g_default_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_default_font; }
    void updateTextTexture(SDL_Renderer* renderer);
    void destroyTexture();
};

} // namespace KitsuGui

#endif
