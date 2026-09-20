#ifndef KITSUGUI_FPS_H
#define KITSUGUI_FPS_H

#include "kitsugui/widget.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

class KitsuFPSView : public KitsuWidget {
public:
    KitsuFPSView();
    ~KitsuFPSView() override;
    
    // Llamar cuando se renderiza un frame real
    void frameRendered();
    
    void render(SDL_Renderer* renderer) override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    static KitsuFPSView* getInstance() { return s_instance; }
    
private:
    int frame_count = 0;
    Uint32 last_second = 0;
    float history[8] = {0};
    int history_idx = 0;
    
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    std::string last_rendered;
    
    static TTF_Font* g_font;
    static KitsuFPSView* s_instance;
};

} // namespace KitsuGui

#endif
