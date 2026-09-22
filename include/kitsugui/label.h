	#ifndef KITSUGUI_LABEL_H
#define KITSUGUI_LABEL_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

enum class TextAlign {
    LEFT, CENTER, RIGHT
};

enum class TextVAlign {
    TOP, MIDDLE, BOTTOM
};

// ============================================================
// KitsuLabel — texto simple
// ============================================================
// Uso mínimo:
//   auto* l = new KitsuLabel("Hola mundo");
//
// El tamaño se calcula automáticamente midiendo el texto.
// Se puede sobreescribir:
//   l->size(300, 40);
//   l->font(KitsuFonts::title());
//   l->color(theme.accent);
//   l->align(TextAlign::CENTER);
// ============================================================
class KitsuLabel : public KitsuWidget {
public:
    explicit KitsuLabel(const std::string& text = "");
    ~KitsuLabel() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuLabel* text(const std::string& t);

    // ===== Apariencia =====
    KitsuLabel* font(TTF_Font* f);
    KitsuLabel* color(const Color& c);
    KitsuLabel* color(Uint8 r, Uint8 g, Uint8 b);
    KitsuLabel* align(TextAlign a);
    KitsuLabel* valign(TextVAlign va);
    KitsuLabel* wrap(int max_width);   // 0 = sin wrap
    KitsuLabel* size(int w, int h);

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;

private:
    std::string text_;
    TTF_Font* font_ = nullptr;

    // -1 = usar el color del tema (text_primary)
    Color color_ = Color(-1, -1, -1);

    TextAlign align_ = TextAlign::LEFT;
    TextVAlign valign_ = TextVAlign::TOP;
    int wrap_width_ = 0;
    bool manual_size_ = false;

    // Caché
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    int cached_r_ = -1, cached_g_ = -1, cached_b_ = -1;
    int cached_wrap_ = -1;

    TTF_Font* activeFont() const;
    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);
    void autoSize();
};

} // namespace KitsuGui

#endif
