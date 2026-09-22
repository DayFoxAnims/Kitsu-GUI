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

// ============================================================
// KitsuViewport
// ============================================================
// Renderiza a una textura interna, que luego se muestra escalada
// según el modo. Útil para canvas, juegos embebidos, previews.
//
// Uso mínimo:
//   auto* vp = new KitsuViewport(640, 480);
//   vp->onRender([](SDL_Renderer* r, int w, int h) {
//       // dibuja en espacio 640x480
//   });
//
// Con input:
//   vp->onInput([](SDL_Event& e) -> bool {
//       // recibe eventos traducidos al viewport
//       return false;
//   });
// ============================================================
class KitsuViewport : public KitsuWidget {
public:
    explicit KitsuViewport(int internal_w = 640, int internal_h = 480);
    ~KitsuViewport() override;

    // ===== Render callback =====
    KitsuViewport* onRender(std::function<void(SDL_Renderer*, int, int)> cb);

    // ===== Input callback =====
    KitsuViewport* onInput(std::function<bool(SDL_Event&)> cb);

    // ===== Textura externa =====
    KitsuViewport* texture(SDL_Texture* tex, bool owns = false);

    // ===== Foco =====
    bool hasFocus() const { return focused_; }
    KitsuViewport* focus(bool f);

    // ===== Apariencia =====
    KitsuViewport* bg(const Color& c);
    KitsuViewport* border_color(const Color& c, int thickness = 2);
    KitsuViewport* focus_border(const Color& c);
    KitsuViewport* corner(float radius);
    KitsuViewport* scaleMode(ViewportScale mode);
    KitsuViewport* internalSize(int w, int h);
    KitsuViewport* clearColor(const Color& c);
    KitsuViewport* size(int w, int h);

    // ===== Animación =====
    KitsuViewport* animated(bool enabled);
    bool isAnimated() const { return animating_; }

    // ===== Acceso =====
    SDL_Texture* targetTexture() const { return target_texture_; }
    int internalWidth()  const { return internal_w_; }
    int internalHeight() const { return internal_h_; }

    // ===== Overrides =====
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    SDL_Texture* target_texture_ = nullptr;
    SDL_Texture* external_texture_ = nullptr;
    bool owns_external_ = false;

    int internal_w_ = 0;
    int internal_h_ = 0;

    std::function<void(SDL_Renderer*, int, int)> render_cb_;
    std::function<bool(SDL_Event&)> input_cb_;

    ViewportScale scale_mode_ = ViewportScale::FIT;
    Color bg_           = Color(-1, -1, -1);
    Color border_       = Color(-1, -1, -1);
    Color border_focus_ = Color(-1, -1, -1);
    Color clear_        = Color(15, 15, 20, 255);
    int border_thickness_ = 2;
    float corner_ = -1.0f;

    bool animating_ = false;
    bool focused_ = false;
    bool mouse_inside_ = false;
    bool manual_size_ = false;

    void destroyTargetTexture();
    void createTargetTexture(SDL_Renderer* renderer);
    SDL_Rect computeDest(const SDL_Rect& abs) const;
};

} // namespace KitsuGui

#endif
