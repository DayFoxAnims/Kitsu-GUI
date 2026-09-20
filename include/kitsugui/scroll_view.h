#ifndef KITSUGUI_SCROLL_VIEW_H
#define KITSUGUI_SCROLL_VIEW_H

#include "kitsugui/box.h"
#include "kitsugui/color.h"

namespace KitsuGui {

class KitsuScrollView : public KitsuWidget {
public:
    KitsuScrollView(bool vertical = true, bool horizontal = false);
    ~KitsuScrollView() override;
    
    void addChild(KitsuWidget* child, bool owns = true);
    void removeChild(KitsuWidget* child);
    void clearChildren();
    
    KitsuBox* getContent() const { return content; }
    
    KitsuScrollView& setBounds(int x, int y, int w, int h);
    KitsuScrollView& withScrollbar(bool show);
    KitsuScrollView& withScrollbarColors(const Color& track, const Color& thumb);
    KitsuScrollView& withBottomPadding(int p);
    KitsuScrollView& withScrollbarWidth(float base, float expanded);
    
    int getScrollY() const { return scroll_y; }
    int getScrollX() const { return scroll_x; }
    void scrollTo(int y);
    void scrollBy(int dy);
    
    void updateLayout() override;
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;
    
private:
    KitsuBox* content = nullptr;
    bool vertical = true;
    bool horizontal = false;
    bool show_scrollbar = true;
    int bottom_padding = 20;
    
    int scroll_y = 0;
    int scroll_x = 0;
    int max_scroll_y = 0;
    int max_scroll_x = 0;
    
    // Drag con mouse (scroll normal)
    bool dragging = false;
    int drag_start_y = 0;
    int drag_start_scroll_y = 0;
    bool mouse_down_inside = false;
    
    // Scrollbar interactivo
    bool scrollbar_hover = false;
    bool scrollbar_dragging = false;
    int  scrollbar_drag_start_y = 0;
    int  scrollbar_drag_start_scroll = 0;
    
    // Transición de ancho
    float scrollbar_width = 6.0f;
    float scrollbar_target_width = 6.0f;
    float scrollbar_base_width = 6.0f;
    float scrollbar_hover_width = 12.0f;
    bool  scrollbar_animating = false;
    
    // Colores
    Color scrollbar_track = {200, 200, 205, 80};
    Color scrollbar_thumb = {140, 140, 150, 180};
    Color scrollbar_thumb_hover = {255, 136, 0, 220};
    
    void computeScrollLimits();
    void clampScroll();
    
    SDL_Rect getScrollbarTrack(const SDL_Rect& abs) const;
    SDL_Rect getScrollbarThumb(const SDL_Rect& abs) const;
    bool isInScrollbar(int mx, int my, const SDL_Rect& abs) const;
};

} // namespace KitsuGui

#endif
