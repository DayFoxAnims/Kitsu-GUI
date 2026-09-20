#ifndef KITSUGUI_PROGRESS_BAR_H
#define KITSUGUI_PROGRESS_BAR_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

enum class ProgressMode {
    DETERMINATE,     // barra normal, 0..1
    INDETERMINATE    // loop animado, sin valor
};

enum class ProgressTextPosition {
    NONE,      // sin texto
    INSIDE,    // texto dentro de la barra, centrado
    ABOVE,     // texto encima
    BELOW,     // texto debajo
    RIGHT      // texto a la derecha
};

class KitsuProgressBar : public KitsuWidget {
public:
    KitsuProgressBar();
    ~KitsuProgressBar() override;
    
    // Configuración fluida
    KitsuProgressBar& withMode(ProgressMode m);
    KitsuProgressBar& withValue(float v);           // 0.0 a 1.0
    KitsuProgressBar& withValue(float v, float max); // v de 0 a max
    KitsuProgressBar& withTextPosition(ProgressTextPosition p);
    KitsuProgressBar& withTextColor(const Color& c);       // color sobre el track vacío
    KitsuProgressBar& withFillTextColor(const Color& c);   // color sobre el fill
    KitsuProgressBar& withTrackColors(const Color& filled, const Color& empty);
    KitsuProgressBar& withFont(TTF_Font* font);
    KitsuProgressBar& withHeight(int h);
    KitsuProgressBar& setBounds(int x, int y, int w, int h);
    
    // Estado
    float getValue() const { return value; }
    void setValue(float v);
    
    // Animación (llamar desde run() o render())
    void update();   // avanza la animación si está en INDETERMINATE
    
    // Overrides
    void render(SDL_Renderer* renderer) override;
    void tick()override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
private:
    ProgressMode mode = ProgressMode::DETERMINATE;
    ProgressTextPosition text_position = ProgressTextPosition::INSIDE;
    
    float value = 0.0f;        // 0.0 a 1.0
    float anim_phase = 0.0f;   // para el loop indeterminado
    Uint32 last_update = 0;
    
    TTF_Font* font = nullptr;
    
    Color track_filled = Color(255, 136, 0);
    Color track_empty  = Color(80, 80, 85);
    Color text_color   = Color(180, 180, 190);   // sobre track vacío
    Color fill_text_color = Color(255, 255, 255); // sobre fill
    
    // Caché de textura del texto
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    static TTF_Font* g_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void updateTextTexture(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b);
    void destroyTextTexture();
    std::string buildText() const;
};

} // namespace KitsuGui

#endif
