#ifndef KITSUGUI_BUTTON_H
#define KITSUGUI_BUTTON_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
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

// ============================================================
// KitsuButton
// ============================================================
// Uso mínimo:
//   auto* b = new KitsuButton("Guardar");
//   b->onClick([]() { ... });
//
// Si no le das tamaño, mide el texto automáticamente.
// ============================================================
class KitsuButton : public KitsuWidget {
public:
    explicit KitsuButton(const std::string& text);

    ~KitsuButton() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuButton* text(const std::string& t);

    // ===== Apariencia =====
    KitsuButton* font(TTF_Font* f);
    KitsuButton* corner(float r);
    KitsuButton* padding(int p);   // override del base
    KitsuButton* size(int w, int h);

    // ===== Estado =====
    ButtonState state() const { return state_; }
    KitsuButton* disable();

    // ===== Callbacks =====
    KitsuButton* onClick(std::function<void()> cb) {
        on_click_ = std::move(cb);
        return this;
    }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    std::string text_;
    ButtonState state_ = ButtonState::NORMAL;
    std::function<void()> on_click_;
    bool mouse_inside_ = false;

    TTF_Font* font_ = nullptr;
    float corner_ = -1.0f;   // -1 = usar el del tema

    // Caché de textura
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    TTF_Font* activeFont() const;
    void updateTextTexture(SDL_Renderer* renderer,
                          Uint8 r, Uint8 g, Uint8 b);
    void destroyTextTexture();
    void autoSize();   // mide el texto y ajusta desired_w/h
};

} // namespace KitsuGui

#endif
