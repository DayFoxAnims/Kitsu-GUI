#ifndef KITSUGUI_RENDERER_H
#define KITSUGUI_RENDERER_H

#include <SDL2/SDL.h>
#include <vector>

namespace KitsuGui {

class KitsuRenderer {
public:
    KitsuRenderer(SDL_Renderer* renderer, int width, int height);
    ~KitsuRenderer();
    
    // Redimensionar el canvas
    bool resize(int width, int height);
    
    // Marcar un área como sucia
    void markDirty(int x, int y, int w, int h);
    void markAllDirty();
    
    // ¿Hay algo sucio?
    bool hasDirty() const { return !dirty_rects.empty(); }
    
    // Ciclo de frame
    void beginFrame();
    void endFrame();
    
    // Acceso
    SDL_Texture* getCanvas() const { return canvas; }
    SDL_Renderer* getRenderer() const { return renderer; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
private:
    SDL_Renderer* renderer;
    SDL_Texture* canvas;
    int width, height;
    
    std::vector<SDL_Rect> dirty_rects;
    
    void mergeDirtyRects();
    static bool shouldMerge(const SDL_Rect& a, const SDL_Rect& b);
    static SDL_Rect mergeRects(const SDL_Rect& a, const SDL_Rect& b);
};

} // namespace KitsuGui

#endif
