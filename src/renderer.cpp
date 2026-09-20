#include "kitsugui/renderer.h"
#include <algorithm>

namespace KitsuGui {

static const int MERGE_THRESHOLD = 32;

KitsuRenderer::KitsuRenderer(SDL_Renderer* r, int w, int h)
    : renderer(r), canvas(nullptr), width(w), height(h) {
    
    canvas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );
    
    if (!canvas) {
        SDL_Log("Error creando canvas: %s", SDL_GetError());
        return;
    }
    
    // Configurar blending para transparencias
    SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);
    
    markAllDirty();
}

KitsuRenderer::~KitsuRenderer() {
    if (canvas) {
        SDL_DestroyTexture(canvas);
        canvas = nullptr;
    }
}

bool KitsuRenderer::resize(int w, int h) {
    if (w == width && h == height) return true;
    if (w <= 0 || h <= 0) return false;
    
    width = w;
    height = h;
    
    if (canvas) SDL_DestroyTexture(canvas);
    canvas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );
    
    if (!canvas) return false;
    
    SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);
    markAllDirty();
    return true;
}

void KitsuRenderer::markDirty(int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    
    SDL_Rect r = {x, y, w, h};
    // Recortar al canvas
    if (r.x < 0) { r.w += r.x; r.x = 0; }
    if (r.y < 0) { r.h += r.y; r.y = 0; }
    if (r.x + r.w > width) r.w = width - r.x;
    if (r.y + r.h > height) r.h = height - r.y;
    if (r.w <= 0 || r.h <= 0) return;
    
    dirty_rects.push_back(r);
    
    // Si hay demasiados, redibujar todo
    if (dirty_rects.size() > 16) {
        markAllDirty();
        return;
    }
    
    mergeDirtyRects();
}

void KitsuRenderer::markAllDirty() {
    dirty_rects.clear();
    dirty_rects.push_back({0, 0, width, height});
}

bool KitsuRenderer::shouldMerge(const SDL_Rect& a, const SDL_Rect& b) {
    return !(a.x + a.w + MERGE_THRESHOLD < b.x ||
             b.x + b.w + MERGE_THRESHOLD < a.x ||
             a.y + a.h + MERGE_THRESHOLD < b.y ||
             b.y + b.h + MERGE_THRESHOLD < a.y);
}

SDL_Rect KitsuRenderer::mergeRects(const SDL_Rect& a, const SDL_Rect& b) {
    int x1 = std::min(a.x, b.x);
    int y1 = std::min(a.y, b.y);
    int x2 = std::max(a.x + a.w, b.x + b.w);
    int y2 = std::max(a.y + a.h, b.y + b.h);
    return {x1, y1, x2 - x1, y2 - y1};
}

void KitsuRenderer::mergeDirtyRects() {
    bool merged = true;
    while (merged) {
        merged = false;
        for (size_t i = 0; i < dirty_rects.size() && !merged; i++) {
            for (size_t j = i + 1; j < dirty_rects.size() && !merged; j++) {
                if (shouldMerge(dirty_rects[i], dirty_rects[j])) {
                    dirty_rects[i] = mergeRects(dirty_rects[i], dirty_rects[j]);
                    dirty_rects.erase(dirty_rects.begin() + j);
                    merged = true;
                }
            }
        }
    }
}

void KitsuRenderer::beginFrame() {
    if (!canvas) return;
    SDL_SetRenderTarget(renderer, canvas);
}

void KitsuRenderer::endFrame() {
    if (!canvas) return;
    
    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderCopy(renderer, canvas, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    
    dirty_rects.clear();
}

} // namespace KitsuGui
