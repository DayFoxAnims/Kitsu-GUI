#ifndef KITSUGUI_PROGRESS_BAR_H
#define KITSUGUI_PROGRESS_BAR_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

enum class ProgressMode {
    DETERMINATE,     // 0.0 a 1.0
    INDETERMINATE    // loop animado
};

enum class ProgressTextPosition {
    NONE,
    INSIDE,
    ABOVE,
    BELOW,
    RIGHT
};

// ============================================================
// KitsuProgressBar
// ============================================================
// Uso mínimo:
//   auto* pb = new KitsuProgressBar();
//   pb->value(0.42f);
//
// Modo indeterminado:
//   auto* pb = new KitsuProgressBar();
//   pb->mode(ProgressMode::INDETERMINATE);
//
// Encadenable:
//   pb->value(0.5f).showText(true).suffix("%");
// ============================================================
class KitsuProgressBar : public KitsuWidget {
public:
    KitsuProgressBar();
    ~KitsuProgressBar() override;

    // ===== Valor =====
    float value() const { return value_; }
    KitsuProgressBar* value(float v);
    KitsuProgressBar* value(float v, float max);

    // ===== Modo =====
    ProgressMode mode() const { return mode_; }
    KitsuProgressBar* mode(ProgressMode m);
    KitsuProgressBar* indeterminate();

    // ===== Apariencia =====
    KitsuProgressBar* textPosition(ProgressTextPosition p);
    KitsuProgressBar* showText(bool show);
    KitsuProgressBar* suffix(const std::string& s);
    KitsuProgressBar* colors(const Color& filled, const Color& empty);
    KitsuProgressBar* textColor(const Color& c);
    KitsuProgressBar* fillTextColor(const Color& c);
    KitsuProgressBar* font(TTF_Font* f);
    KitsuProgressBar* height(int h);
    KitsuProgressBar* size(int w, int h);

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    void tick() override;

private:
    ProgressMode mode_ = ProgressMode::DETERMINATE;
    ProgressTextPosition text_position_ = ProgressTextPosition::INSIDE;

    float value_ = 0.0f;         // 0.0 a 1.0
    float anim_phase_ = 0.0f;    // para INDETERMINATE
    Uint32 last_update_ = 0;

    TTF_Font* font_ = nullptr;
    std::string suffix_;

    // -1 = "usar el del tema"
    Color track_filled_ = Color(-1, -1, -1);
    Color track_empty_  = Color(-1, -1, -1);
    Color text_color_   = Color(-1, -1, -1);
    Color fill_text_color_ = Color(-1, -1, -1);

    bool manual_size_ = false;

    // Caché del texto
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    TTF_Font* activeFont() const;
    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);
    std::string buildText() const;
};

} // namespace KitsuGui

#endif
