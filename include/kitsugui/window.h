#ifndef KITSUGUI_WINDOW_H
#define KITSUGUI_WINDOW_H

#include "kitsugui/widget.h"
#include "kitsugui/color.h"
#include "kitsugui/renderer.h"
#include <SDL2/SDL.h>
#include <string>
#include <vector>

namespace KitsuGui {

class KitsuBox;

// ============================================================
// KitsuWindow
// ============================================================
// Uso mínimo (¡la magia!):
//   KitsuWindow win(1000, 720, "Mi App");
//   win.add(new KitsuLabel("Hola mundo"));
//   win.add(new KitsuButton("Click").onClick([](){ ... }));
//   run();
//
// El constructor se encarga automáticamente de:
//   - SDL_Init(SDL_INIT_VIDEO)
//   - TTF_Init() vía KitsuFonts::init()
//   - Cargar config, tema e iconos (KitsuConfig)
//   - Crear la ventana y el renderer
//   - Configurar el root con auto-layout
//
// No hace falta tocar fuentes, temas ni bounds manuales.
// ============================================================
class KitsuWindow {
public:
    KitsuWindow(int width, int height,
                const std::string& title,
                bool resizable = true);
    ~KitsuWindow();

    // ===== Configuración de fondo =====
    KitsuWindow& background(const Color& c);
    KitsuWindow& background(int r, int g, int b);

    // ===== Añadir widgets =====
    // add() → participan en el layout del root
    KitsuWindow* add(KitsuWidget* widget, bool owns = true);

    // addOverlay() → se dibujan encima, NO participan en el layout
    KitsuWindow* addOverlay(KitsuWidget* widget, bool owns = false);

    // ===== Acceso =====
    KitsuBox*         root()         const { return root_; }
    SDL_Window*       sdlWindow()    const { return window_; }
    SDL_Renderer*     sdlRenderer()  const { return renderer_; }
    KitsuRenderer*    renderer()     const { return krenderer_; }
    const Color&      bgColor()      const { return bg_color_; }

    // ===== Sincronización =====
    void syncRootSize();

    // ===== Estado =====
    bool isRunning() const { return running_; }

    // ===== Overlays (usado por run.cpp) =====
    std::vector<KitsuWidget*>& overlays() { return overlays_; }

private:
    SDL_Window*    window_    = nullptr;
    SDL_Renderer*  renderer_  = nullptr;
    KitsuRenderer* krenderer_ = nullptr;
    KitsuBox*      root_      = nullptr;

    Color bg_color_ = Color(245, 240, 235);
    bool running_ = false;

    std::vector<KitsuWidget*> overlays_;
    std::vector<KitsuWidget*> owned_overlays_;

    bool initSDL();
    bool initWindow(const std::string& title, int w, int h, bool resizable);
    bool initRenderer();

    friend void run();
};

} // namespace KitsuGui

#endif
