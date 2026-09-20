#ifndef KITSUGUI_PANEL_H
#define KITSUGUI_PANEL_H

#include "kitsugui/box.h"
#include "kitsugui/color.h"
#include "kitsugui/theme.h"
#include <string>

namespace KitsuGui {

class KitsuPanel : public KitsuBox {
public:
    KitsuPanel(bool horizontal = false);
    ~KitsuPanel() override;
    
    KitsuPanel& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    KitsuPanel& setPadding(int p) {
        KitsuBox::setPadding(p);
        return *this;
    }
    KitsuPanel& setSpacing(int s) {
        KitsuBox::setSpacing(s);
        return *this;
    }
    KitsuPanel& setMargin(int m) {
        KitsuBox::setMargin(m);
        return *this;
    }
    KitsuPanel& withPadding(int p)  { return setPadding(p); }
    KitsuPanel& withSpacing(int s)  { return setSpacing(s); }
    KitsuPanel& withMargin(int m)   { return setMargin(m); }
    
    KitsuPanel& withBackground(const Color& c);
    KitsuPanel& withBorder(const Color& c, int thickness = 1);
    KitsuPanel& withCorner(float radius);
    
    void render(SDL_Renderer* renderer) override;
    
private:
    // ===== Config =====
    // Si use_theme_bg es true, bg_color se ignora y se usa el del tema
    bool use_theme_bg = true;
    bool use_theme_border = true;
    bool use_theme_radius = true;
    
    Color bg_color;
    Color border_color;
    int border_thickness = 1;
    float corner_radius = 0;
};

} // namespace KitsuGui

#endif
