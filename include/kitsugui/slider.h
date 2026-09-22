#ifndef KITSUGUI_SLIDER_H
#define KITSUGUI_SLIDER_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

enum class SliderOrientation {
    HORIZONTAL,
    VERTICAL
};

// ============================================================
// KitsuSlider
// ============================================================
// Uso mínimo:
//   auto* s = new KitsuSlider(0, 100, 50);
//   s->onChange([](float v) { ... });
//
// Con valores default (0..100, valor 0):
//   auto* s = new KitsuSlider();
//
// Formato del número:
//   s->suffix("%").decimals(1).showNumber(true);
// ============================================================
class KitsuSlider : public KitsuWidget {
public:
    explicit KitsuSlider(float min = 0.0f,
                        float max = 100.0f,
                        float value = 0.0f);
    ~KitsuSlider() override;

    // ===== Valor =====
    float value() const { return value_; }
    KitsuSlider* value(float v);

    float min() const { return min_; }
    float max() const { return max_; }
    KitsuSlider* range(float min, float max);
    KitsuSlider* step(float s);

    // ===== Apariencia =====
    KitsuSlider* orientation(SliderOrientation o);
    KitsuSlider* showNumber(bool show);
    KitsuSlider* suffix(const std::string& s);
    KitsuSlider* decimals(int d);
    KitsuSlider* font(TTF_Font* f);
    KitsuSlider* size(int w, int h);
    KitsuSlider* disable();

    // ===== Callbacks =====
    KitsuSlider* onChange(std::function<void(float)> cb) {
        on_change_ = std::move(cb);
        return this;
    }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    float min_ = 0.0f;
    float max_ = 100.0f;
    float value_ = 0.0f;
    float step_ = 0.0f;

    SliderOrientation orientation_ = SliderOrientation::HORIZONTAL;
    bool show_number_ = true;
    bool mouse_inside_ = false;
    bool dragging_ = false;
    bool disabled_ = false;
    bool manual_size_ = false;

    std::string suffix_;
    int decimals_ = 0;

    TTF_Font* font_ = nullptr;
    std::function<void(float)> on_change_;

    // Caché del número
    SDL_Texture* number_texture_ = nullptr;
    int num_w_ = 0, num_h_ = 0;
    std::string cached_number_;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    TTF_Font* activeFont() const;
    void destroyNumberTexture();
    void updateNumberTexture(SDL_Renderer* renderer, Color color);
    void autoSize();

    SDL_Rect trackRect(int abs_x, int abs_y, int abs_w, int abs_h,
                      int num_w) const;
    int knobPosition(int track_x, int track_y,
                     int track_w, int track_h, int knob_size) const;
    float valueFromPos(int mx, int my,
                      int track_x, int track_y,
                      int track_w, int track_h, int knob_size) const;
};

} // namespace KitsuGui

#endif
	
