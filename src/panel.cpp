#include "kitsugui/panel.h"
#include "kitsugui/shapes.h"

namespace KitsuGui {

KitsuPanel::KitsuPanel(bool horizontal)
    : KitsuBox(horizontal) {
    setAlignment(KitsuAlign::CENTER);
    setJustify(KitsuJustify::CENTER);
    padding = 20;
    spacing = 12;
    
    // Inicializar con el tema actual
    KitsuTheme& t = KitsuTheme::current();
    bg_color = t.bg_secondary;
    border_color = t.border;
    border_thickness = t.border_normal;
    corner_radius = t.radius_medium;
}

KitsuPanel::~KitsuPanel() = default;

KitsuPanel& KitsuPanel::withBackground(const Color& c) {
    bg_color = c;
    use_theme_bg = false;
    markDirty();
    return *this;
}

KitsuPanel& KitsuPanel::withBorder(const Color& c, int thickness) {
    border_color = c;
    border_thickness = thickness;
    use_theme_border = false;
    markDirty();
    return *this;
}

KitsuPanel& KitsuPanel::withCorner(float radius) {
    corner_radius = radius;
    use_theme_radius = false;
    markDirty();
    return *this;
}

void KitsuPanel::render(SDL_Renderer* renderer) {
    if (!visible) return;
    
    SDL_Rect abs = getRenderBounds();
    
    // Colores del tema (o custom si el usuario los cambió)
    KitsuTheme& t = KitsuTheme::current();
    
    Color bg = use_theme_bg ? t.bg_secondary : bg_color;
    Color border = use_theme_border ? t.border : border_color;
    float radius = use_theme_radius ? t.radius_medium : corner_radius;
    int thickness = use_theme_border ? t.border_normal : border_thickness;
    
    // 1. Fondo redondeado
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(radius)
        .fill(bg)
        .draw(renderer);
    
    // 2. Borde (usando doble rect para bordes limpios)
    if (thickness > 0) {
        // Borde exterior
        KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
            .radius(radius)
            .fill(border)
            .draw(renderer);
        
        // Fondo interior
        float fth = (float)thickness;
        KitsuRect((float)abs.x + fth, (float)abs.y + fth,
                  (float)abs.w - 2*fth, (float)abs.h - 2*fth)
            .radius(SDL_max(0.0f, radius - fth))
            .fill(bg)
            .draw(renderer);
    }
    
    // 3. Hijos
    KitsuBox::render(renderer);
    
    clearDirty();
}

} // namespace KitsuGui
