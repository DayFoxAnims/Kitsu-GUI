#include "kitsugui/widget.h"
#include "internal.h"

namespace KitsuGui {

int g_active_animations = 0;
int g_always_render = 0;

void KitsuWidget::markDirty() {
    dirty_ = true;
    if (g_renderer) {
        SDL_Rect abs = getRenderBounds();
        g_renderer->markDirty(abs.x, abs.y, abs.w, abs.h);
    }
}

SDL_Rect KitsuWidget::getAbsoluteBounds() const {
    SDL_Rect abs = bounds;
    const KitsuWidget* p = parent;
    while (p) {
        abs.x += p->bounds.x;
        abs.y += p->bounds.y;
        p = p->parent;
    }
    return abs;
}

SDL_Rect KitsuWidget::getRenderBounds() const {
    SDL_Rect abs = bounds;
    const KitsuWidget* p = parent;
    while (p) {
        abs.x += p->bounds.x + p->render_offset_x;
        abs.y += p->bounds.y + p->render_offset_y;
        p = p->parent;
    }
    return abs;
}

bool KitsuWidget::contains(int x, int y) const {
    SDL_Rect abs = getAbsoluteBounds();
    return (x >= abs.x && x < abs.x + abs.w &&
            y >= abs.y && y < abs.y + abs.h);
}

// ============================================================
// checkRightClick
// ============================================================
// Devuelve true si el evento es un click derecho dentro del widget
// Y hay callback configurado. Llama al callback con coordenadas absolutas.
bool KitsuWidget::checkRightClick(const SDL_Event& e) {
    // ¿Es un click derecho?
    if (e.type != SDL_MOUSEBUTTONDOWN) return false;
    if (e.button.button != SDL_BUTTON_RIGHT) return false;
    
    // ¿Hay callback?
    if (!on_right_click) return false;
    
    // ¿Está visible y enabled?
    if (!visible || !enabled) return false;
    
    // ¿El click está dentro?
    SDL_Rect abs = getAbsoluteBounds();
    int mx = e.button.x;
    int my = e.button.y;
    bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                   my >= abs.y && my < abs.y + abs.h);
    
    if (inside) {
        on_right_click(mx, my);
        return true;
    }
    
    return false;
}

} // namespace KitsuGui
