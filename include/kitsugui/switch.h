#ifndef KITSUGUI_SWITCH_H
#define KITSUGUI_SWITCH_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

// ============================================================
// KitsuSwitch — toggle estilo iOS/Android
// ============================================================
// Uso mínimo:
//   auto* sw = new KitsuSwitch("Modo oscuro");
//   sw->onChange([](bool c) { ... });
//
// Con estado inicial:
//   auto* sw = new KitsuSwitch("Modo oscuro", true);
// ============================================================
class KitsuSwitch : public KitsuWidget {
public:
    explicit KitsuSwitch(const std::string& text = "",
                        bool checked = false);
    ~KitsuSwitch() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuSwitch* text(const std::string& t);

    // ===== Estado =====
    bool isChecked() const { return checked_; }
    KitsuSwitch* checked(bool c);
    KitsuSwitch* toggle();
    KitsuSwitch* disable();

    // ===== Apariencia =====
    KitsuSwitch* font(TTF_Font* f);
    KitsuSwitch* size(int w, int h);

    // ===== Callbacks =====
    KitsuSwitch* onChange(std::function<void(bool)> cb) {
        on_change_ = std::move(cb);
        return this;
    }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    std::string text_;
    bool checked_ = false;
    bool mouse_inside_ = false;
    bool mouse_down_ = false;
    bool disabled_ = false;
    bool manual_size_ = false;

    TTF_Font* font_ = nullptr;
    std::function<void(bool)> on_change_;

    // Caché de textura
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    TTF_Font* activeFont() const;
    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);
    void autoSize();
};

} // namespace KitsuGui

#endif
