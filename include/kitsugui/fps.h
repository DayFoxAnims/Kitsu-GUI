#ifndef KITSUGUI_FPS_H
#define KITSUGUI_FPS_H

#include "kitsugui/widget.h"
#include <SDL2/SDL_ttf.h>
#include <string>

namespace KitsuGui {

// ============================================================
// KitsuFPSView — overlay de FPS
// ============================================================
// Uso:
//   win.addOverlay(new KitsuFPSView());
//
// Es una instancia única. `KitsuFPSView::instance()` la devuelve.
// ============================================================
class KitsuFPSView : public KitsuWidget {
public:
    KitsuFPSView();
    ~KitsuFPSView() override;

    // Llamar cuando se renderiza un frame real (run.cpp lo hace)
    void frameRendered();

    void render(SDL_Renderer* renderer) override;

    static KitsuFPSView* instance() { return s_instance_; }

private:
    int frame_count_ = 0;
    Uint32 last_second_ = 0;
    float history_[8] = {0};
    int history_idx_ = 0;

    SDL_Texture* text_texture_ = nullptr;
    int tex_w_ = 0, tex_h_ = 0;
    std::string cached_text_;
    std::string last_rendered_;

    static KitsuFPSView* s_instance_;
};

} // namespace KitsuGui

#endif
