#include "kitsugui/widget.h"
#include "internal.h"

namespace KitsuGui {

int g_active_animations = 0;
int g_always_render = 0;

// ============================================================
// Invalidación
// ============================================================
void KitsuWidget::invalidate() {
    dirty_ = true;
    if (g_renderer) {
        g_renderer->invalidate();
    }
}

// ============================================================
// Rectángulos
// ============================================================
// globalRect() incluye el transform de los padres.
// Esto hace que el hit-test respete el scroll automáticamente
// (bug #7 resuelto de raíz).
// ============================================================
SDL_Rect KitsuWidget::globalRect() const {
    SDL_Rect r = bounds;
    const KitsuWidget* p = parent;
    while (p) {
        r.x += p->bounds.x + p->transform.offset_x;
        r.y += p->bounds.y + p->transform.offset_y;
        p = p->parent;
    }
    return r;
}

// visualRect() es un alias conceptual por ahora.
// En el futuro: si añadimos rotación/escala, globalRect() NO
// las incluiría (para hit-test AABB), y visualRect() sí.
SDL_Rect KitsuWidget::visualRect() const {
    return globalRect();
}

bool KitsuWidget::contains(int x, int y) const {
    SDL_Rect r = globalRect();
    return (x >= r.x && x < r.x + r.w &&
            y >= r.y && y < r.y + r.h);
}

// ============================================================
// Menú contextual
// ============================================================
bool KitsuWidget::tryContextMenu(const SDL_Event& e) {
    if (e.type != SDL_MOUSEBUTTONDOWN) return false;
    if (e.button.button != SDL_BUTTON_RIGHT) return false;
    if (!onContextMenu) return false;
    if (!visible || !enabled) return false;

    SDL_Rect r = globalRect();
    int mx = e.button.x;
    int my = e.button.y;

    if (mx >= r.x && mx < r.x + r.w &&
        my >= r.y && my < r.y + r.h) {
        onContextMenu(mx, my);
        return true;
    }
    return false;
}

} // namespace KitsuGui
