#ifndef KITSUGUI_CHECK_BOX_H
#define KITSUGUI_CHECK_BOX_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>

namespace KitsuGui {

// ============================================================
// KitsuCheckBox
// ============================================================
// Uso mínimo:
//   auto* cb = new KitsuCheckBox("Acepto los términos");
//   cb->onChange([](bool c) { ... });
//
// Constructor con texto vacío también vale (checkbox sin label).
// ============================================================
class KitsuCheckBox : public KitsuWidget {
public:
    explicit KitsuCheckBox(const std::string& text = "",
                          bool checked = false);
    ~KitsuCheckBox() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuCheckBox* text(const std::string& t);

    // ===== Estado =====
    bool isChecked() const { return checked_; }
    KitsuCheckBox* checked(bool c);
    KitsuCheckBox* toggle();
    KitsuCheckBox* disable();

    // ===== Apariencia =====
    KitsuCheckBox* font(TTF_Font* f);
    KitsuCheckBox* size(int w, int h);

    // ===== Callbacks =====
    KitsuCheckBox* onChange(std::function<void(bool)> cb) {
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
