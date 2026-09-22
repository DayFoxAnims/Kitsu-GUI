#ifndef KITSUGUI_SCROLL_VIEW_H
#define KITSUGUI_SCROLL_VIEW_H

#include "kitsugui/widget.h"
#include "kitsugui/box.h"
#include "kitsugui/color.h"

namespace KitsuGui {

// ============================================================
// KitsuScrollView — contenedor con scroll
// ============================================================
// Uso mínimo:
//   auto* sv = new KitsuScrollView();     // vertical por defecto
//   auto* content = sv->content();
//   content->padding(20).spacing(10);
//   content->add(new KitsuLabel("Hola"));
//   content->add(new KitsuButton("Click"));
//
// El scroll se aplica como transform al content. Los hijos
// lo ven automáticamente vía globalRect() → hit-test correcto
// incluso con scroll anidado.
// ============================================================
class KitsuScrollView : public KitsuWidget {
public:
    explicit KitsuScrollView(bool vertical = true,
                            bool horizontal = false);
    ~KitsuScrollView() override;

    // ===== Contenido =====
    KitsuBox* content() const { return content_; }
    KitsuScrollView* add(KitsuWidget* child, bool owns = true);
    void remove(KitsuWidget* child);
    void clear();

    // ===== Configuración (fluent) =====
    KitsuScrollView* size(int w, int h);
    KitsuScrollView* scrollbar(bool show);
    KitsuScrollView* scrollbarColors(const Color& track, const Color& thumb);
    KitsuScrollView* bottomPadding(int p);
    KitsuScrollView* scrollbarWidth(float base, float expanded);

    // ===== Scroll =====
    int scrollY() const { return scroll_y_; }
    int scrollX() const { return scroll_x_; }
    KitsuScrollView* scrollTo(int y);
    KitsuScrollView* scrollBy(int dy);

    // ===== Overrides =====
    void updateLayout() override;
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    void tick() override;

private:
    KitsuBox* content_ = nullptr;

    bool vertical_ = true;
    bool horizontal_ = false;
    bool show_scrollbar_ = true;
    bool disabled_ = false;
    bool manual_size_ = false;

    int scroll_y_ = 0;
    int scroll_x_ = 0;
    int max_scroll_y_ = 0;
    int max_scroll_x_ = 0;
    int bottom_padding_ = 20;

    // Drag con mouse
    bool dragging_ = false;
    int drag_start_y_ = 0;
    int drag_start_scroll_y_ = 0;
    bool mouse_down_inside_ = false;

    // Scrollbar
    bool scrollbar_hover_ = false;
    bool scrollbar_dragging_ = false;
    int  scrollbar_drag_start_y_ = 0;
    int  scrollbar_drag_start_scroll_ = 0;

    // Animación del ancho del scrollbar
    float scrollbar_width_ = 6.0f;
    float scrollbar_target_width_ = 6.0f;
    float scrollbar_base_width_ = 6.0f;
    float scrollbar_hover_width_ = 12.0f;
    bool  scrollbar_animating_ = false;

    // Colores
    Color scrollbar_track_       = Color(-1, -1, -1);
    Color scrollbar_thumb_       = Color(-1, -1, -1);
    Color scrollbar_thumb_hover_ = Color(-1, -1, -1);

    void computeLimits();
    void clampScroll();

    SDL_Rect scrollbarTrack(const SDL_Rect& abs) const;
    SDL_Rect scrollbarThumb(const SDL_Rect& abs) const;
    bool isInScrollbar(int mx, int my, const SDL_Rect& abs) const;
};

} // namespace KitsuGui

#endif
