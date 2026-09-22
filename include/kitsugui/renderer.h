#ifndef KITSUGUI_RENDERER_H
#define KITSUGUI_RENDERER_H

#include <SDL2/SDL.h>

namespace KitsuGui {

// ============================================================
// KitsuRenderer — Canvas retenido con render on-demand
// ============================================================
// Modelo: NO hay dirty rects. La app está a 0 FPS cuando nadie
// hace nada, y cuando CUALQUIER cosa cambia se marca TODO como
// sucio y se redibuja la ventana completa en el siguiente frame.
//
// Este es el modelo de GTK, Qt Widgets, Dear ImGui, etc.
// Simple, predecible, suficiente para el 95% de las UIs.
//
// El canvas (textura target) sigue existiendo porque:
//   - KitsuPopup lo usa para su propio KitsuRenderer
//   - KitsuViewport renderiza a textura target
//   - Deja la puerta abierta a dirty rects futuros sin reescribir
// ============================================================
class KitsuRenderer {
public:
    KitsuRenderer(SDL_Renderer* renderer, int width, int height);
    ~KitsuRenderer();

    // ===== Redimensionar =====
    bool resize(int width, int height);

    // ===== Estado =====
    void invalidate()  { dirty_ = true; }
    bool needsRender() const { return dirty_; }

    // ===== Ciclo de frame =====
    void begin();   // marca el canvas como target
    void end();     // presenta y limpia el flag

    // ===== Acceso =====
    SDL_Texture*  getCanvas()   const { return canvas; }
    SDL_Renderer* getRenderer() const { return renderer; }
    int getWidth()  const { return width; }
    int getHeight() const { return height; }

private:
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  canvas   = nullptr;
    int width = 0;
    int height = 0;
    bool dirty_ = true;
};

} // namespace KitsuGui

#endif
