#include "kitsugui/viewport.h"
#include "kitsugui/shapes.h"
#include "internal.h"

namespace KitsuGui {

KitsuViewport::KitsuViewport(int internal_w, int internal_h)
    : internal_w(internal_w), internal_h(internal_h) {
    bounds = {0, 0, 320, 240};
    requested_w = 320;
    requested_h = 240;
    
    g_always_render++;
    animating = true;
}

KitsuViewport::~KitsuViewport() {
    if (animating && g_always_render > 0) {
        g_always_render--;
        animating = false;
    }
    destroyTargetTexture();
    if (owns_external && external_texture) {
        SDL_DestroyTexture(external_texture);
        external_texture = nullptr;
    }
}

void KitsuViewport::destroyTargetTexture() {
    if (target_texture) {
        SDL_DestroyTexture(target_texture);
        target_texture = nullptr;
    }
}

void KitsuViewport::createTargetTexture(SDL_Renderer* renderer) {
    destroyTargetTexture();
    if (internal_w <= 0 || internal_h <= 0) return;
    
    target_texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        internal_w, internal_h
    );
}

void KitsuViewport::setRenderCallback(std::function<void(SDL_Renderer*, int, int)> cb) {
    render_cb = cb;
    markDirty();
}

void KitsuViewport::setFocus(bool f) {
    if (focused == f) return;
    focused = f;
    markDirty();
}

void KitsuViewport::setAnimated(bool enabled) {
    if (enabled == animating) return;
    animating = enabled;
    if (enabled) {
        g_always_render++;
    } else {
        if (g_always_render > 0) g_always_render--;
    }
    markDirty();
}

void KitsuViewport::resizeInternal(int w, int h) {
    if (w == internal_w && h == internal_h) return;
    internal_w = w;
    internal_h = h;
    if (g_renderer) createTargetTexture(g_renderer->getRenderer());
    markDirty();
}

// ===== CONFIGURACIÓN FLUIDA =====
KitsuViewport& KitsuViewport::withBackground(const Color& c) {
    bg_color = c; markDirty(); return *this;
}

KitsuViewport& KitsuViewport::withBorder(const Color& c, int thickness) {
    border_color = c; border_thickness = thickness; markDirty(); return *this;
}

KitsuViewport& KitsuViewport::withCorner(float radius) {
    corner_radius = radius; markDirty(); return *this;
}

KitsuViewport& KitsuViewport::withScaleMode(ViewportScale mode) {
    scale_mode = mode; markDirty(); return *this;
}

KitsuViewport& KitsuViewport::withInternalSize(int w, int h) {
    resizeInternal(w, h); return *this;
}

KitsuViewport& KitsuViewport::withClearColor(const Color& c) {
    clear_color = c; markDirty(); return *this;
}

KitsuViewport& KitsuViewport::withFocusBorder(const Color& c) {
    border_focus_color = c; markDirty(); return *this;
}

void KitsuViewport::setTexture(SDL_Texture* tex, bool owns) {
    if (owns_external && external_texture) {
        SDL_DestroyTexture(external_texture);
    }
    external_texture = tex;
    owns_external = owns;
    if (tex) {
        int tw, th;
        SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
        internal_w = tw;
        internal_h = th;
    }
    markDirty();
}

// ===== DEST RECT =====
SDL_Rect KitsuViewport::computeDestRect(const SDL_Rect& abs) const {
    SDL_Rect dst = abs;
    if (scale_mode == ViewportScale::STRETCH || internal_w <= 0 || internal_h <= 0) return dst;
    
    float va = (float)abs.w / (float)abs.h;
    float ca = (float)internal_w / (float)internal_h;
    
    if (scale_mode == ViewportScale::FIT) {
        if (ca > va) {
            dst.w = abs.w; dst.h = (int)(abs.w / ca);
            dst.x = abs.x; dst.y = abs.y + (abs.h - dst.h) / 2;
        } else {
            dst.h = abs.h; dst.w = (int)(abs.h * ca);
            dst.x = abs.x + (abs.w - dst.w) / 2; dst.y = abs.y;
        }
    } else if (scale_mode == ViewportScale::FILL) {
        if (ca > va) {
            dst.h = abs.h; dst.w = (int)(abs.h * ca);
            dst.x = abs.x + (abs.w - dst.w) / 2; dst.y = abs.y;
        } else {
            dst.w = abs.w; dst.h = (int)(abs.w / ca);
            dst.x = abs.x; dst.y = abs.y + (abs.h - dst.h) / 2;
        }
    } else if (scale_mode == ViewportScale::CENTER) {
        dst.w = internal_w; dst.h = internal_h;
        dst.x = abs.x + (abs.w - dst.w) / 2;
        dst.y = abs.y + (abs.h - dst.h) / 2;
    }
    return dst;
}

// ===== INPUT =====
bool KitsuViewport::handleEvent(const SDL_Event& e) {
    if (!visible || !enabled) return false;
    
    SDL_Rect abs = getAbsoluteBounds();
    
    // ===== Determinar si el mouse está dentro =====
    int mx = -1, my = -1;
    switch (e.type) {
        case SDL_MOUSEMOTION: mx = e.motion.x; my = e.motion.y; break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: mx = e.button.x; my = e.button.y; break;
        case SDL_MOUSEWHEEL: mx = e.wheel.mouseX; my = e.wheel.mouseY; break;
    }
    
    bool inside = (mx >= 0 && my >= 0 &&
                   mx >= abs.x && mx < abs.x + abs.w &&
                   my >= abs.y && my < abs.y + abs.h);
    
    // ===== Actualizar hover =====
    if (e.type == SDL_MOUSEMOTION) {
        if (inside != mouse_inside) {
            mouse_inside = inside;
            markDirty();
        }
    }
    
    // ===== Click: dar foco si el click está dentro =====
    if (e.type == SDL_MOUSEBUTTONDOWN && inside) {
        setFocus(true);
    }
    // Click fuera: quitar foco
    if (e.type == SDL_MOUSEBUTTONDOWN && !inside) {
        setFocus(false);
    }
    
    // ===== Pasar eventos al callback de input =====
    if (input_cb) {
        // Eventos de mouse: solo si el mouse está dentro
        bool is_mouse_event = (e.type == SDL_MOUSEMOTION ||
                               e.type == SDL_MOUSEBUTTONDOWN ||
                               e.type == SDL_MOUSEBUTTONUP ||
                               e.type == SDL_MOUSEWHEEL);
        
        // Eventos de teclado: solo si el viewport tiene foco
        bool is_key_event = (e.type == SDL_KEYDOWN ||
                            e.type == SDL_KEYUP ||
                            e.type == SDL_TEXTINPUT);
        
        if (is_mouse_event && inside) {
            SDL_Event adjusted = e;
            // Traducir coordenadas a espacio del viewport
            if (e.type == SDL_MOUSEMOTION) {
                adjusted.motion.x -= abs.x;
                adjusted.motion.y -= abs.y;
            } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
                adjusted.button.x -= abs.x;
                adjusted.button.y -= abs.y;
            }
            
            if (input_cb(adjusted)) return true;   // consumido
        }
        else if (is_key_event && focused) {
            SDL_Event adjusted = e;
            if (input_cb(adjusted)) return true;
        }
    }
    
    // El viewport consume clicks dentro para no propagarlos al padre
    if (e.type == SDL_MOUSEBUTTONDOWN && inside) return true;
    if (e.type == SDL_MOUSEBUTTONUP && inside) return true;
    
    return false;
}

// ===== RENDER =====
void KitsuViewport::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    SDL_Rect abs = getRenderBounds();
    
    if (!target_texture && !external_texture) {
        createTargetTexture(renderer);
    }
    
    // Render callback a la textura interna
    if (render_cb && target_texture) {
        SDL_Texture* prev_target = SDL_GetRenderTarget(renderer);
        if (SDL_SetRenderTarget(renderer, target_texture) == 0) {
            SDL_SetRenderDrawColor(renderer,
                (Uint8)clear_color.r, (Uint8)clear_color.g,
                (Uint8)clear_color.b, (Uint8)clear_color.a);
            SDL_RenderClear(renderer);
            render_cb(renderer, internal_w, internal_h);
            SDL_SetRenderTarget(renderer, prev_target);
        }
    }
    
    // Fondo
    KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
        .radius(corner_radius).fill(bg_color).draw(renderer);
    
    // Textura
    SDL_Texture* tex = external_texture ? external_texture : target_texture;
    if (tex) {
        SDL_Rect dst = computeDestRect(abs);
        if (corner_radius > 0) SDL_RenderSetClipRect(renderer, &abs);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        if (corner_radius > 0) SDL_RenderSetClipRect(renderer, nullptr);
    }
    
    // Borde (color distinto si tiene foco)
    if (border_thickness > 0) {
        Color bc = focused ? border_focus_color : border_color;
        KitsuRect((float)abs.x, (float)abs.y, (float)abs.w, (float)abs.h)
            .radius(corner_radius).fill(bc).drawOutline(renderer, (float)border_thickness);
    }
    
    clearDirty();
}

} // namespace KitsuGui
