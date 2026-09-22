#include "kitsugui/panel.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuPanel::KitsuPanel(bool horizontal)
    : KitsuBox(horizontal) {
    align(KitsuAlign::CENTER);
    justify(KitsuJustify::CENTER);
    padding_ = 20;
    spacing_ = 12;
}

KitsuPanel::~KitsuPanel() = default;

// ============================================================
// Apariencia
// ============================================================
KitsuPanel* KitsuPanel::bg(const Color& c) {
    bg_ = c;
    invalidate();
    return this;
}

KitsuPanel* KitsuPanel::border_color(const Color& c) {
    border_ = c;
    invalidate();
    return this;
}

KitsuPanel* KitsuPanel::border_width(int thickness) {
    border_w_ = thickness;
    invalidate();
    return this;
}

KitsuPanel* KitsuPanel::corner(float radius) {
    corner_ = radius;
    invalidate();
    return this;
}

// ============================================================
// Overrides fluent
// ============================================================
KitsuPanel* KitsuPanel::padding(int p) {
    KitsuBox::padding(p);
    return this;
}

KitsuPanel* KitsuPanel::spacing(int s) {
    KitsuBox::spacing(s);
    return this;
}

KitsuPanel* KitsuPanel::margin(int m) {
    KitsuBox::margin(m);
    return this;
}

// ============================================================
// Render
// ============================================================
void KitsuPanel::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // Resolver colores/estilos: tema o custom
    Color bg     = (bg_.r < 0)     ? t.bg_secondary : bg_;
    Color border = (border_.r < 0) ? t.border       : border_;
    int   bw     = (border_w_ < 0) ? t.border_thickness_panel : border_w_;
    float radius = (corner_ < 0.0f) ? t.radius_panel : corner_;

    // ===== 1. Fondo =====
    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(radius)
        .fill(bg)
        .draw(renderer);

    // ===== 2. Borde =====
    if (bw > 0) {
        // Borde exterior (color)
        KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
            .radius(radius)
            .fill(border)
            .draw(renderer);

        // Relleno interior (tapa el centro y deja solo el marco)
        float fw = (float)bw;
        float inner_r = radius - fw;
        if (inner_r < 0) inner_r = 0;

        KitsuRect((float)r.x + fw, (float)r.y + fw,
                  (float)r.w - 2 * fw, (float)r.h - 2 * fw)
            .radius(inner_r)
            .fill(bg)
            .draw(renderer);
    }

    // ===== 3. Hijos =====
    KitsuBox::render(renderer);
}

} // namespace KitsuGui
