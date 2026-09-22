#include "kitsugui/icon.h"
#include "kitsugui/fonts.h"
#include "kitsugui/theme.h"
#include "internal.h"

namespace KitsuGui {

// ============================================================
// Constructores
// ============================================================
KitsuIconView::KitsuIconView(uint16_t unicode, int size)
    : source_(IconSource::FONTAWESOME), size_(size), unicode_(unicode) {
    desired_w = size;
    desired_h = size;
    bounds = {0, 0, size, size};
}

KitsuIconView::KitsuIconView(const std::string& name, int size,
                            IconSource src)
    : source_(src), size_(size), icon_name_(name) {
    desired_w = size;
    desired_h = size;
    bounds = {0, 0, size, size};
}

KitsuIconView::~KitsuIconView() {
    destroyFATexture();
    destroySystemTexture();
}

// ============================================================
// Destrucción de texturas
// ============================================================
void KitsuIconView::destroyFATexture() {
    if (fa_texture_) {
        SDL_DestroyTexture(fa_texture_);
        fa_texture_ = nullptr;
    }
}

void KitsuIconView::destroySystemTexture() {
    if (system_texture_ && owns_system_texture_) {
        SDL_DestroyTexture(system_texture_);
    }
    system_texture_ = nullptr;
    system_renderer_ = nullptr;
    owns_system_texture_ = false;
}

// ============================================================
// Contenido
// ============================================================
KitsuIconView* KitsuIconView::glyph(uint16_t u) {
    if (unicode_ != u || source_ != IconSource::FONTAWESOME) {
        unicode_ = u;
        source_ = IconSource::FONTAWESOME;
        destroySystemTexture();
        invalidate();
    }
    return this;
}

KitsuIconView* KitsuIconView::name(const std::string& n) {
    if (icon_name_ != n || source_ != IconSource::SYSTEM_THEME) {
        icon_name_ = n;
        source_ = IconSource::SYSTEM_THEME;
        destroySystemTexture();
        invalidate();
    }
    return this;
}

KitsuIconView* KitsuIconView::source(IconSource s) {
    if (source_ != s) {
        source_ = s;
        destroySystemTexture();
        invalidate();
    }
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuIconView* KitsuIconView::size(int s) {
    if (size_ != s) {
        size_ = s;
        desired_w = s;
        desired_h = s;
        bounds.w = s;
        bounds.h = s;
        destroySystemTexture();
        invalidate();
    }
    return this;
}

KitsuIconView* KitsuIconView::color(const Color& c) {
    cached_r_ = (Uint8)c.r;
    cached_g_ = (Uint8)c.g;
    cached_b_ = (Uint8)c.b;
    // Invalidar la caché de textura FA (depende del color)
    destroyFATexture();
    invalidate();
    return this;
}

KitsuIconView* KitsuIconView::color(Uint8 r, Uint8 g, Uint8 b) {
    return color(Color(r, g, b));
}

// ============================================================
// FontAwesome
// ============================================================
void KitsuIconView::updateFATexture(SDL_Renderer* renderer) {
    // Cache válida
    if (fa_texture_ &&
        cached_unicode_ == unicode_ &&
        cached_size_ == size_) {
        return;
    }

    destroyFATexture();
    if (unicode_ == 0) return;

    TTF_Font* font = KitsuFonts::normal();
    if (!font) return;

    SDL_Color c = { cached_r_, cached_g_, cached_b_, 255 };
    SDL_Surface* surface = TTF_RenderGlyph_Blended(font, unicode_, c);
    if (!surface) return;

    // Si el tamaño del glyph no coincide con size_, escalar
    SDL_Surface* final_surface = surface;
    if (surface->w != size_ || surface->h != size_) {
        SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(
            0, size_, size_, 32, SDL_PIXELFORMAT_RGBA32);
        if (scaled) {
            SDL_BlitScaled(surface, nullptr, scaled, nullptr);
            SDL_FreeSurface(surface);
            final_surface = scaled;
        }
    }

    fa_texture_ = SDL_CreateTextureFromSurface(renderer, final_surface);
    SDL_FreeSurface(final_surface);

    cached_unicode_ = unicode_;
    cached_size_ = size_;
}

// ============================================================
// Render
// ============================================================
void KitsuIconView::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    if (bounds.w <= 0 || bounds.h <= 0) return;

    SDL_Rect r = visualRect();

    if (source_ == IconSource::SYSTEM_THEME) {
        // ¿Cambió el renderer? → recargar
        if (system_renderer_ != renderer) {
            destroySystemTexture();
            system_renderer_ = renderer;
        }

        if (!system_texture_) {
            bool cached = false;
            system_texture_ = KitsuIconCache::instance().getTexture(
                renderer, icon_name_, size_, cached);
            owns_system_texture_ = !cached;
        }

        if (!system_texture_) {
            clearNeedsRender();
            return;
        }

        int tw = 0, th = 0;
        SDL_QueryTexture(system_texture_, nullptr, nullptr, &tw, &th);

        SDL_Rect dst = {
            r.x + (r.w - tw) / 2,
            r.y + (r.h - th) / 2,
            tw, th
        };
        SDL_RenderCopy(renderer, system_texture_, nullptr, &dst);
    } else {
        updateFATexture(renderer);
        if (!fa_texture_) {
            clearNeedsRender();
            return;
        }

        int tw = 0, th = 0;
        SDL_QueryTexture(fa_texture_, nullptr, nullptr, &tw, &th);

        SDL_Rect dst = {
            r.x + (r.w - tw) / 2,
            r.y + (r.h - th) / 2,
            tw, th
        };
        SDL_RenderCopy(renderer, fa_texture_, nullptr, &dst);
    }

    clearNeedsRender();
}

} // namespace KitsuGui
