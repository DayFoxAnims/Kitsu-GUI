#ifndef KITSUGUI_ICON_H
#define KITSUGUI_ICON_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include "kitsugui/icon_theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

enum class IconSource {
    FONTAWESOME,
    SYSTEM_THEME
};

// ============================================================
// KitsuIconView
// ============================================================
// Icono FontAwesome (unicode):
//   auto* i = new KitsuIconView(0xf00c, 20);   // check
//
// Icono del sistema (nombre XDG):
//   auto* i = new KitsuIconView("document-save", 24);
//
// Fluent:
//   i->color(theme.accent).size(32);
// ============================================================
class KitsuIconView : public KitsuWidget {
public:
    // Constructor FontAwesome
    explicit KitsuIconView(uint16_t unicode, int size = 20);

    // Constructor system theme
    explicit KitsuIconView(const std::string& name,
                          int size = 24,
                          IconSource src = IconSource::SYSTEM_THEME);

    ~KitsuIconView() override;

    // ===== Contenido =====
    KitsuIconView* glyph(uint16_t u);
    KitsuIconView* name(const std::string& n);
    KitsuIconView* source(IconSource s);

    // ===== Apariencia =====
    KitsuIconView* size(int s);
    KitsuIconView* color(const Color& c);
    KitsuIconView* color(Uint8 r, Uint8 g, Uint8 b);

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;

private:
    IconSource source_ = IconSource::FONTAWESOME;
    int size_ = 20;

    // FontAwesome
    uint16_t unicode_ = 0;
    SDL_Texture* fa_texture_ = nullptr;
    uint16_t cached_unicode_ = 0;
    int cached_size_ = 0;
    Uint8 cached_r_ = 40, cached_g_ = 40, cached_b_ = 45;

    // System theme
    std::string icon_name_;
    SDL_Texture* system_texture_ = nullptr;
    SDL_Renderer* system_renderer_ = nullptr;
    bool owns_system_texture_ = false;

    void destroyFATexture();
    void destroySystemTexture();
    void updateFATexture(SDL_Renderer* renderer);
};

} // namespace KitsuGui

#endif
