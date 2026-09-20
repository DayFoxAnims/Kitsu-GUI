#ifndef KITSUGUI_WIDGET_H
#define KITSUGUI_WIDGET_H

#include <SDL2/SDL.h>
#include <functional>

namespace KitsuGui {

enum class KitsuAlign {
    START, CENTER, END, STRETCH
};

enum class KitsuJustify {
    START, CENTER, END,
    SPACE_BETWEEN, SPACE_AROUND, SPACE_EVENLY
};

class KitsuWidget {
public:
    KitsuWidget* parent = nullptr;
    SDL_Rect bounds = {0, 0, 0, 0};
    
    int requested_w = 0;
    int requested_h = 0;
    
    int render_offset_x = 0;
    int render_offset_y = 0;
    
    int margin = 0;
    int padding = 0;
    bool visible = true;
    bool enabled = true;
    
    // ===== Callback de click derecho =====
    // Recibe coordenadas absolutas (x, y) del click.
    // Si está definido y el click cae dentro del widget, se llama
    // y el evento se consume.
    std::function<void(int, int)> on_right_click;
    
    virtual ~KitsuWidget() {}
    
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void updateLayout() {}
    virtual bool handleEvent(const SDL_Event& e) { return false; }
    virtual void tick() {}
    
    void setBoundsInternal(int x, int y, int w, int h) {
        bounds.x = x;
        bounds.y = y;
        bounds.w = w;
        bounds.h = h;
        requested_w = w;
        requested_h = h;
        markDirty();
    }
    
    virtual void markDirty();
    bool isDirty() const { return dirty_; }
    void clearDirty() { dirty_ = false; }
    
    SDL_Rect getAbsoluteBounds() const;
    SDL_Rect getRenderBounds() const;
    
    bool contains(int x, int y) const;
    
    // Comprueba si un evento es un click derecho dentro de este widget.
    // Si lo es y hay callback, lo dispara y devuelve true.
    bool checkRightClick(const SDL_Event& e);
    
protected:
    bool dirty_ = true;
};

extern int g_active_animations;
extern int g_always_render;

} // namespace KitsuGui

#endif
