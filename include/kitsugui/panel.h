#ifndef KITSUGUI_PANEL_H
#define KITSUGUI_PANEL_H

#include "kitsugui/box.h"
#include "kitsugui/color.h"

namespace KitsuGui {

// ============================================================
// KitsuPanel — caja con fondo y borde
// ============================================================
// Hereda de KitsuBox: tiene layout flex + fondo + borde.
//
// Uso mínimo:
//   auto* p = new KitsuPanel();
//   p->padding(20).spacing(12);
//   p->add(new KitsuLabel("Hola"));
//
// Por defecto usa los colores/radio/grosor del tema.
// Se pueden sobreescribir:
//   p->bg(Color(30, 30, 40));
//   p->border_color(Color(255, 136, 0)).border_width(3);
//   p->corner(12.0f);
//
// Nota: la API interna del layout la hereda de KitsuBox, no
// la duplicamos aquí. Los "with" fluent están en la base.
// ============================================================
class KitsuPanel : public KitsuBox {
public:
    explicit KitsuPanel(bool horizontal = false);
    ~KitsuPanel() override;

    // ===== Apariencia (fluent) =====
    KitsuPanel* bg(const Color& c);
    KitsuPanel* border_color(const Color& c);
    KitsuPanel* border_width(int thickness);
    KitsuPanel* corner(float radius);

    // ===== Overrides para encadenar (devuelven KitsuPanel&) =====
    KitsuPanel* padding(int p);
    KitsuPanel* spacing(int s);
    KitsuPanel* margin(int m);

    // ===== Override de render =====
    void render(SDL_Renderer* renderer) override;

private:
    // -1 = "usar el del tema"
    Color bg_         = Color(-1, -1, -1);
    Color border_     = Color(-1, -1, -1);
    int   border_w_   = -1;
    float corner_     = -1.0f;
};

} // namespace KitsuGui

#endif
