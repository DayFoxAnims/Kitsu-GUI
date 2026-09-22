#include "kitsugui/renderer.h"

namespace KitsuGui {

KitsuRenderer::KitsuRenderer(SDL_Renderer* r, int w, int h)
    : renderer(r), width(w), height(h) {

    if (!renderer || w <= 0 || h <= 0) {
        SDL_Log("KitsuRenderer: parámetros inválidos (%dx%d)", w, h);
        return;
    }

    canvas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );

    if (!canvas) {
        SDL_Log("KitsuRenderer: error creando canvas: %s", SDL_GetError());
        return;
    }

    SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);
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
    if (!renderer) return false;

    width  = w;
    height = h;

    if (canvas) {
        SDL_DestroyTexture(canvas);
        canvas = nullptr;
    }

    canvas = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        w, h
    );

    if (!canvas) {
        SDL_Log("KitsuRenderer: error recreando canvas: %s", SDL_GetError());
        return false;
    }

    SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);
    invalidate();
    return true;
}

void KitsuRenderer::begin() {
    if (!canvas || !renderer) return;
    SDL_SetRenderTarget(renderer, canvas);
}

void KitsuRenderer::end() {
    if (!canvas || !renderer) return;

    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderCopy(renderer, canvas, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    dirty_ = false;
}

} // namespace KitsuGui
