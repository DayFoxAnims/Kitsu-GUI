#ifndef KITSUGUI_COLOR_H
#define KITSUGUI_COLOR_H

namespace KitsuGui {

enum class Theme {
    LIGHT,
    DARK
};

struct Color {
    int r, g, b, a;
    
    Color() : r(255), g(136), b(0), a(255) {}
    Color(int r, int g, int b) : r(r), g(g), b(b), a(255) {}
    Color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}
    Color(const char* hex);
    
    static Color fromHSV(float h, float s, float v);
    
    Color withAlpha(int alpha) const {
        return Color(r, g, b, alpha);
    }
    
    // ===== Colores del tema actual =====
    // NOTA: Estos métodos NO van a funcionar bien hasta que se
    // incluya theme.h. Por eso los movemos a theme.h.
    // Aquí dejamos solo los "primitivos" que no dependen del tema.
    
    // Colores neutros (no dependen del tema)
    static Color Transparent()  { return Color(0, 0, 0, 0); }
    static Color White()        { return Color(255, 255, 255); }
    static Color Black()        { return Color(0, 0, 0); }
};

} // namespace KitsuGui

#endif
