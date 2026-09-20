#ifndef KITSUGUI_ICON_H
#define KITSUGUI_ICON_H

#include "kitsugui/widget.h"
#include "kitsugui/theme.h"
#include "kitsugui/icon_theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

enum class IconSource {
    FONTAWESOME,
    SYSTEM_THEME
};

class KitsuIconView : public KitsuWidget {
public:
    // Constructor FontAwesome (unicode)
    KitsuIconView(uint16_t unicode, int size = 20);
    
    // Constructor para iconos del sistema (Papirus, Breeze, etc.)
    KitsuIconView(const std::string& name, int size = 20,
                  IconSource src = IconSource::SYSTEM_THEME);
    ~KitsuIconView() override;
    
    // ===== Configuración =====
    void setUnicode(uint16_t u);
    void setIconName(const std::string& name);
    void setSize(int s);
    KitsuIconView& withColor(Uint8 r, Uint8 g, Uint8 b);
    KitsuIconView& withSource(IconSource src);
    
    // ===== Render =====
    void render(SDL_Renderer* renderer) override;
    
    // ===== Fuentes FontAwesome =====
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
private:
    IconSource source = IconSource::FONTAWESOME;
    
    // FontAwesome
    uint16_t unicode = 0;
    SDL_Texture* fa_texture = nullptr;
    uint16_t cached_unicode = 0;
    int cached_size = 0;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    // System theme
    std::string icon_name;
    int size = 20;
    SDL_Texture* system_texture = nullptr;
    SDL_Renderer* system_renderer = nullptr;   // renderer dueño de la textura
    bool owns_system_texture = false;          // true si debemos destruirla
    
    static TTF_Font* g_font;
    
    void updateFATexture(SDL_Renderer* renderer);
    void destroyFATexture();
    void destroySystemTexture();
};

} // namespace KitsuGui

#endif
