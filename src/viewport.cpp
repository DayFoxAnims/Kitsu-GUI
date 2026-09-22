#include "kitsugui/viewport.h"
#include "kitsugui/shapes.h"
#include "kitsugui/theme.h"
#include "internal.h"

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuViewport::KitsuViewport(int internal_w, int internal_h)
    : internal_w_(internal_w), internal_h_(internal_h) {
    desired_w = 320;
    desired_h = 240;
    bounds = {0, 0, 320, 240};

    g_always_render++;
    animating_ = true;
}

KitsuViewport::~KitsuViewport() {
    if (animating_ && g_always_render > 0) {
        g_always_render--;
        animating_ = false;
    }
    destroyTargetTexture();
    if (owns_external_ && external_texture_) {
        SDL_DestroyTexture(external_texture_);
        external_texture_ = nullptr;
    }
}

// ============================================================
// Callbacks
// ============================================================
KitsuViewport* KitsuViewport::onRender(
    std::function<void(SDL_Renderer*, int, int)> cb) {
    render_cb_ = std::move(cb);
    invalidate();
    return this;
}

KitsuViewport* KitsuViewport::onInput(
    std::function<bool(SDL_Event&)> cb) {
    input_cb_ = std::move(cb);
    return this;
}

// ============================================================
// Textura externa
// ============================================================
KitsuViewport* KitsuViewport::texture(SDL_Texture* tex, bool owns) {
    if (owns_external_ && external_texture_) {
        SDL_DestroyTexture(external_texture_);
    }
    external_texture_ = tex;
    owns_external_ = owns;

    if (tex) {
        int tw, th;
        SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
        internal_w_ = tw;
        internal_h_ = th;
    }
    invalidate();
    return this;
}

// ============================================================
// Foco
// ============================================================
KitsuViewport* KitsuViewport::focus(bool f) {
    if (focused_ != f) {
        focused_ = f;
        invalidate();
    }
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuViewport* KitsuViewport::bg(const Color& c) {
    bg_ = c; invalidate(); return this;
}
KitsuViewport* KitsuViewport::border_color(const Color& c, int th) {
    border_ = c; border_thickness_ = th; invalidate(); return this;
}
KitsuViewport* KitsuViewport::focus_border(const Color& c) {
    border_focus_ = c; invalidate(); return this;
}
KitsuViewport* KitsuViewport::corner(float radius) {
    corner_ = radius; invalidate(); return this;
}
KitsuViewport* KitsuViewport::scaleMode(ViewportScale mode) {
    scale_mode_ = mode; invalidate(); return this;
}
KitsuViewport* KitsuViewport::internalSize(int w, int h) {
    if (w == internal_w_ && h == internal_h_) return this;
    internal_w_ = w;
    internal_h_ = h;
    if (g_renderer) {
        createTargetTexture(g_renderer->getRenderer());
    }
    invalidate();
    return this;
}
KitsuViewport* KitsuViewport::clearColor(const Color& c) {
    clear_ = c; invalidate(); return this;
}
KitsuViewport* KitsuViewport::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    invalidate();
    return this;
}

KitsuViewport* KitsuViewport::animated(bool enabled) {
    if (enabled == animating_) return this;
    animating_ = enabled;
    if (enabled) {
        g_always_render++;
    } else {
        if (g_always_render > 0) g_always_render--;
    }
    invalidate();
    return this;
}

// ============================================================
// Target texture
// ============================================================
void KitsuViewport::destroyTargetTexture() {
    if (target_texture_) {
        SDL_DestroyTexture(target_texture_);
        target_texture_ = nullptr;
    }
}

void KitsuViewport::createTargetTexture(SDL_Renderer* renderer) {
    destroyTargetTexture();
    if (internal_w_ <= 0 || internal_h_ <= 0 || !renderer) return;

    target_texture_ = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        internal_w_, internal_h_);

    if (target_texture_) {
        SDL_SetTextureBlendMode(target_texture_, SDL_BLENDMODE_BLEND);
    }
}

// ============================================================
// Compute dest rect
// ============================================================
SDL_Rect KitsuViewport::computeDest(const SDL_Rect& abs) const {
    SDL_Rect dst = abs;
    if (scale_mode_ == ViewportScale::STRETCH ||
        internal_w_ <= 0 || internal_h_ <= 0) {
        return dst;
    }

    float va = (float)abs.w / (float)abs.h;
    float ca = (float)internal_w_ / (float)internal_h_;

    if (scale_mode_ == ViewportScale::FIT) {
        if (ca > va) {
            dst.w = abs.w;
            dst.h = (int)(abs.w / ca);
            dst.x = abs.x;
            dst.y = abs.y + (abs.h - dst.h) / 2;
        } else {
            dst.h = abs.h;
            dst.w = (int)(abs.h * ca);
            dst.x = abs.x + (abs.w - dst.w) / 2;
            dst.y = abs.y;
        }
    } else if (scale_mode_ == ViewportScale::FILL) {
        if (ca > va) {
            dst.h = abs.h;
            dst.w = (int)(abs.h * ca);
            dst.x = abs.x + (abs.w - dst.w) / 2;
            dst.y = abs.y;
        } else {
            dst.w = abs.w;
            dst.h = (int)(abs.w / ca);
            dst.x = abs.x;
            dst.y = abs.y + (abs.h - dst.h) / 2;
        }
    } else if (scale_mode_ == ViewportScale::CENTER) {
        dst.w = internal_w_;
        dst.h = internal_h_;
        dst.x = abs.x + (abs.w - dst.w) / 2;
        dst.y = abs.y + (abs.h - dst.h) / 2;
    }
    return dst;
}

// ============================================================
// Eventos
// ============================================================
bool KitsuViewport::handleEvent(const SDL_Event& e) {
    if (!visible || !enabled) return false;

    SDL_Rect r = globalRect();

    int mx = -1, my = -1;
    switch (e.type) {
        case SDL_MOUSEMOTION:   mx = e.motion.x; my = e.motion.y; break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: mx = e.button.x; my = e.button.y; break;
        case SDL_MOUSEWHEEL:    mx = e.wheel.mouseX; my = e.wheel.mouseY; break;
    }

    bool inside = (mx >= 0 && my >= 0 &&
                   mx >= r.x && mx < r.x + r.w &&
                   my >= r.y && my < r.y + r.h);

    if (e.type == SDL_MOUSEMOTION && inside != mouse_inside_) {
        mouse_inside_ = inside;
        invalidate();
    }

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (inside) focus(true);
        else        focus(false);
    }

    // Callback de input
    if (input_cb_) {
        bool is_mouse_event = (e.type == SDL_MOUSEMOTION ||
                               e.type == SDL_MOUSEBUTTONDOWN ||
                               e.type == SDL_MOUSEBUTTONUP ||
                               e.type == SDL_MOUSEWHEEL);
        bool is_key_event = (e.type == SDL_KEYDOWN ||
                            e.type == SDL_KEYUP ||
                            e.type == SDL_TEXTINPUT);

        if (is_mouse_event && inside) {
            SDL_Event adjusted = e;
            if (e.type == SDL_MOUSEMOTION) {
                adjusted.motion.x -= r.x;
                adjusted.motion.y -= r.y;
            } else if (e.type == SDL_MOUSEBUTTONDOWN ||
                       e.type == SDL_MOUSEBUTTONUP) {
                adjusted.button.x -= r.x;
                adjusted.button.y -= r.y;
            }
            if (input_cb_(adjusted)) return true;
        } else if (is_key_event && focused_) {
            SDL_Event adjusted = e;
            if (input_cb_(adjusted)) return true;
        }
    }

    if (e.type == SDL_MOUSEBUTTONDOWN && inside) return true;
    if (e.type == SDL_MOUSEBUTTONUP && inside) return true;

    return false;
}

// ============================================================
// Render
// ============================================================
void KitsuViewport::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    SDL_Rect r = visualRect();
    KitsuTheme& t = KitsuTheme::active();

    // Asegurar texture interna
    if (!target_texture_ && !external_texture_) {
        createTargetTexture(renderer);
    }

    // Render callback a textura interna
    if (render_cb_ && target_texture_) {
        SDL_Texture* prev = SDL_GetRenderTarget(renderer);
        if (SDL_SetRenderTarget(renderer, target_texture_) == 0) {
            SDL_SetRenderDrawColor(renderer,
                (Uint8)clear_.r, (Uint8)clear_.g,
                (Uint8)clear_.b, (Uint8)clear_.a);
            SDL_RenderClear(renderer);
            render_cb_(renderer, internal_w_, internal_h_);
            SDL_SetRenderTarget(renderer, prev);
        }
    }

    // Fondo
    Color bg = (bg_.r < 0) ? t.bg_primary : bg_;
    float radius = (corner_ < 0.0f) ? t.radius_viewport : corner_;

    KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
        .radius(radius)
        .fill(bg)
        .draw(renderer);

    // Textura
    SDL_Texture* tex = external_texture_ ? external_texture_ : target_texture_;
    if (tex) {
        SDL_Rect dst = computeDest(r);
        if (radius > 0) SDL_RenderSetClipRect(renderer, &r);
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        if (radius > 0) SDL_RenderSetClipRect(renderer, nullptr);
    }

    // Borde
    if (border_thickness_ > 0) {
        Color bc;
        if (focused_ && border_focus_.r >= 0) {
            bc = border_focus_;
        } else if (border_.r >= 0) {
            bc = border_;
        } else {
            bc = t.border;
        }

        KitsuRect((float)r.x, (float)r.y, (float)r.w, (float)r.h)
            .radius(radius)
            .fill(bc)
            .drawOutline(renderer, (float)border_thickness_);
    }

    clearNeedsRender();
}

} // namespace KitsuGui
