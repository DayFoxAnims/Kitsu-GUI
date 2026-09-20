#ifndef KITSUGUI_SLIDER_H
#define KITSUGUI_SLIDER_H

#include "kitsugui/widget.h"
#include "kitsugui/theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

enum class SliderOrientation {
    HORIZONTAL,
    VERTICAL
};

class KitsuSlider : public KitsuWidget {
public:
    KitsuSlider(float min = 0.0f, float max = 100.0f, float value = 0.0f);
    ~KitsuSlider() override;
    
    KitsuSlider& withOrientation(SliderOrientation o);
    KitsuSlider& withValue(float v);
    KitsuSlider& withRange(float min, float max);
    KitsuSlider& withStep(float s);
    KitsuSlider& withShowNumber(bool show);
    KitsuSlider& withNumberSuffix(const std::string& suffix);
    KitsuSlider& withDecimals(int d);
    KitsuSlider& withFont(TTF_Font* font);
    KitsuSlider& withCallback(std::function<void(float)> cb);
    KitsuSlider& disabled();
    KitsuSlider& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    float getValue() const { return value; }
    void setValue(float v);
    
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
private:
    float min_value = 0.0f;
    float max_value = 100.0f;
    float value = 0.0f;
    float step = 0.0f;
    bool show_number = true;
    bool mouse_inside = false;
    bool dragging = false;
    bool enabled_ = true;
    SliderOrientation orientation = SliderOrientation::HORIZONTAL;
    
    std::string number_suffix = "";
    int decimals = 0;
    
    TTF_Font* font = nullptr;
    std::function<void(float)> callback;
    
    // Caché del número
    SDL_Texture* number_texture = nullptr;
    int num_w = 0, num_h = 0;
    std::string cached_number;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    static TTF_Font* g_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void updateNumberTexture(SDL_Renderer* renderer);
    void destroyNumberTexture();
    
    int getKnobPosition(int track_x, int track_y, int track_w, int track_h, int knob_size) const;
    float getValueFromPosition(int mx, int my, int track_x, int track_y, int track_w, int track_h, int knob_size) const;
    SDL_Rect getTrackRect(int abs_x, int abs_y, int abs_w, int abs_h, int num_w) const;
};

} // namespace KitsuGui

#endif
