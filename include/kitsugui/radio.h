#ifndef KITSUGUI_RADIO_H
#define KITSUGUI_RADIO_H

#include "kitsugui/widget.h"
#include "kitsugui/box.h"
#include "kitsugui/color.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>
#include <vector>

namespace KitsuGui {

class KitsuRadioGroup;

// ============================================================
// KitsuRadioButton
// ============================================================
// Uso mínimo (suelto):
//   auto* rb = new KitsuRadioButton("Opción");
//   rb->onChange([](bool c) { ... });
//
// Uso normal: dentro de un KitsuRadioGroup (solo uno activo).
// ============================================================
class KitsuRadioButton : public KitsuWidget {
    friend class KitsuRadioGroup;

public:
    explicit KitsuRadioButton(const std::string& text = "",
                             bool checked = false);
    ~KitsuRadioButton() override;

    // ===== Contenido =====
    const std::string& text() const { return text_; }
    KitsuRadioButton* text(const std::string& t);

    // ===== Estado =====
    bool isChecked() const { return checked_; }
    KitsuRadioButton* checked(bool c);
    KitsuRadioButton* disable();

    // ===== Apariencia =====
    KitsuRadioButton* font(TTF_Font* f);
    KitsuRadioButton* size(int w, int h);

    // ===== Callbacks =====
    KitsuRadioButton* onChange(std::function<void(bool)> cb) {
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
    KitsuRadioGroup* group_ = nullptr;

    // Caché
    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    TTF_Font* cached_font_ = nullptr;
    Uint8 cached_r_ = 0, cached_g_ = 0, cached_b_ = 0;

    TTF_Font* activeFont() const;
    void destroyTexture();
    void updateTextTexture(SDL_Renderer* renderer, Color color);
    void autoSize();

    // Llamado por el grupo cuando OTRO radio es seleccionado
    void activateFromGroup();
};

// ============================================================
// KitsuRadioGroup
// ============================================================
// Uso:
//   auto* group = new KitsuRadioGroup();
//   group->add(new KitsuRadioButton("A"));
//   group->add(new KitsuRadioButton("B", true));
//   group->add(new KitsuRadioButton("C"));
//   group->onChange([](auto* r) { ... });
// ============================================================
class KitsuRadioGroup : public KitsuBox {
public:
    KitsuRadioGroup();
    ~KitsuRadioGroup() override;

    // Añadir radio. Owning por defecto.
    KitsuRadioGroup* add(KitsuRadioButton* radio, bool owns = true);
    void remove(KitsuRadioButton* radio);

    KitsuRadioButton* selected() const { return selected_; }

    KitsuRadioGroup* onChange(std::function<void(KitsuRadioButton*)> cb) {
        on_change_ = std::move(cb);
        return this;
    }

    // Interno — llamado por KitsuRadioButton
    void notifySelected(KitsuRadioButton* r);

private:
    std::vector<KitsuRadioButton*> radios_;
    std::vector<KitsuRadioButton*> owned_;
    KitsuRadioButton* selected_ = nullptr;
    std::function<void(KitsuRadioButton*)> on_change_;
};

} // namespace KitsuGui

#endif
