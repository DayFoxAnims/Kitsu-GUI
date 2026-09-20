#ifndef KITSUGUI_VIEWPORT_H
#define KITSUGUI_VIEWPORT_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include <functional>

namespace KitsuGui {

enum class ViewportScale {
    STRETCH,
    FIT,
    FILL,
    CENTER
};

class KitsuViewport : public KitsuWidget {
public:
    KitsuViewport(int internal_w = 640, int internal_h = 480);
    ~KitsuViewport() override;
    
    // ===== Render =====
    void setRenderCallback(std::function<void(SDL_Renderer*, int, int)> cb);
    void setTexture(SDL_Texture* tex, bool owns = false);
    
    // ===== Input =====
    // Callback de eventos. Devuelve true si el evento fue consumido.
    // SOLO se llama si el mouse está dentro del viewport (o si tiene foco para teclado).
    void setInputCallback(std::function<bool(SDL_Event&)> cb) {
        input_cb = cb;
    }
    
    bool hasFocus() const { return focused; }
    void setFocus(bool f);
    
    // ===== Configuración fluida =====
    KitsuViewport& withBackground(const Color& c);
    KitsuViewport& withBorder(const Color& c, int thickness = 2);
    KitsuViewport& withCorner(float radius);
    KitsuViewport& withScaleMode(ViewportScale mode);
    KitsuViewport& withInternalSize(int w, int h);
    KitsuViewport& withClearColor(const Color& c);
    KitsuViewport& withFocusBorder(const Color& c);
    
    KitsuViewport& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    void resizeInternal(int w, int h);
    
    void setAnimated(bool enabled);
    bool isAnimated() const { return animating; }
    
    SDL_Texture* getTargetTexture() const { return target_texture; }
    int getInternalWidth() const { return internal_w; }
    int getInternalHeight() const { return internal_h; }
    
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
private:
    SDL_Texture* target_texture = nullptr;
    SDL_Texture* external_texture = nullptr;
    bool owns_external = false;
    
    int internal_w = 0;
    int internal_h = 0;
    
    std::function<void(SDL_Renderer*, int, int)> render_cb;
    std::function<bool(SDL_Event&)> input_cb;
    
    ViewportScale scale_mode = ViewportScale::FIT;
    Color bg_color = {20, 20, 25, 255};
    Color border_color = {255, 136, 0, 255};
    Color border_focus_color = {255, 220, 60, 255};   // amarillo al focus
    Color clear_color = {15, 15, 20, 255};
    int border_thickness = 2;
    float corner_radius = 0;
    
    bool animating = false;
    bool focused = false;
    bool mouse_inside = false;
    bool mouse_dragging = false;
    
    void createTargetTexture(SDL_Renderer* renderer);
    void destroyTargetTexture();
    SDL_Rect computeDestRect(const SDL_Rect& abs) const;
};

} // namespace KitsuGui

#endif
