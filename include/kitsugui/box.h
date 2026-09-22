#ifndef KITSUGUI_BOX_H
#define KITSUGUI_BOX_H

#include "kitsugui/widget.h"
#include <vector>

namespace KitsuGui {

// ============================================================
// KitsuBox — contenedor con layout flex
// ============================================================
// Uso mínimo:
//   auto* box = new KitsuVBox();
//   box->padding(16).spacing(8);
//   box->add(new KitsuLabel("Hola"));
//   box->add(new KitsuButton("Click"));
//
// Auto-layout activo por defecto. No tienes que llamar
// updateLayout() manualmente: el padre lo hace por ti.
//
// Para posicionamiento absoluto (overlays, canvas), usa
// autoLayout(false) y place() a mano.
// ============================================================
class KitsuBox : public KitsuWidget {
public:
    explicit KitsuBox(bool horizontal = true);
    ~KitsuBox() override;

    // ===== Layout =====
    KitsuBox* align(KitsuAlign a);
    KitsuBox* justify(KitsuJustify j);
    KitsuBox* spacing(int s);
    KitsuBox* padding(int p);   // override
    KitsuBox* margin(int m);    // override
    KitsuBox* size(int w, int h);
    KitsuBox* autoLayout(bool enabled);

    bool isHorizontal() const { return horizontal_; }
    bool isAutoLayout() const { return auto_layout_; }

    // ===== Hijos =====
    // add() con owning=true (default) destruye los hijos al morir.
    // add() devuelve el propio box para encadenar:
    //   box->add(a)->add(b)->add(c);
    KitsuBox* add(KitsuWidget* child, bool owns = true);
    void remove(KitsuWidget* child);
    void clear();

    const std::vector<KitsuWidget*>& children() const { return children_; }

    // ===== Overrides =====
    void updateLayout() override;
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;

protected:
    bool horizontal_ = true;
    bool auto_layout_ = true;
    KitsuAlign align_ = KitsuAlign::START;
    KitsuJustify justify_ = KitsuJustify::START;
    int spacing_ = 0;

    std::vector<KitsuWidget*> children_;
    std::vector<KitsuWidget*> owned_;   // los que hay que destruir

    void layoutHorizontal();
    void layoutVertical();

    // Asegura que un hijo tenga desired_w/h razonables tras
    // su propio updateLayout() (arregla el bug histórico del
    // requested_w == 0).
    void syncChildDesired(KitsuWidget* c);
};

// Alias semánticos
using KitsuHBox = KitsuBox;
using KitsuVBox = KitsuBox;

} // namespace KitsuGui

#endif
