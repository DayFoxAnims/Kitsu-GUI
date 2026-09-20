#include "kitsugui/icon.h"
#include "internal.h"

namespace KitsuGui {

TTF_Font* KitsuIconView::g_font = nullptr;

// ============================================================
// CONSTRUCTORES
// ============================================================

// FontAwesome (unicode)
KitsuIconView::KitsuIconView(uint16_t unicode, int size)
    : unicode(unicode), size(size) {
    source = IconSource::FONTAWESOME;
    bounds.w = size;
    bounds.h = size;
    requested_w = size;
    requested_h = size;
}

// System theme (por nombre)
KitsuIconView::KitsuIconView(const std::string& name, int size, IconSource src)
    : icon_name(name), size(size) {
    source = src;
    bounds.w = size;
    bounds.h = size;
    requested_w = size;
    requested_h = size;
}

KitsuIconView::~KitsuIconView() {
    destroyFATexture();
    destroySystemTexture();
}

// ============================================================
// DESTRUCTORES DE TEXTURAS
// ============================================================
void KitsuIconView::destroyFATexture() {
    if (fa_texture) {
        SDL_DestroyTexture(fa_texture);
        fa_texture = nullptr;
    }
}

void KitsuIconView::destroySystemTexture() {
    // Solo destruimos la textura si es nuestra (no viene del cache global)
    if (system_texture && owns_system_texture) {
        SDL_DestroyTexture(system_texture);
    }
    system_texture = nullptr;
    system_renderer = nullptr;
    owns_system_texture = false;
}

// ============================================================
// CONFIGURACIÓN
// ============================================================
void KitsuIconView::setUnicode(uint16_t u) {
    if (unicode != u || source != IconSource::FONTAWESOME) {
        unicode = u;
        source = IconSource::FONTAWESOME;
        destroySystemTexture();
        markDirty();
    }
}

void KitsuIconView::setIconName(const std::string& name) {
    if (icon_name != name || source != IconSource::SYSTEM_THEME) {
        icon_name = name;
        source = IconSource::SYSTEM_THEME;
        destroySystemTexture();
        markDirty();
    }
}

void KitsuIconView::setSize(int s) {
    if (size != s) {
        size = s;
        bounds.w = s;
        bounds.h = s;
        requested_w = s;
        requested_h = s;
        destroySystemTexture();   // el tamaño cambió → recargar
        markDirty();
    }
}

KitsuIconView& KitsuIconView::withColor(Uint8 r, Uint8 g, Uint8 b) {
    cached_r = r;
    cached_g = g;
    cached_b = b;
    markDirty();
    return *this;
}

KitsuIconView& KitsuIconView::withSource(IconSource src) {
    if (source != src) {
        source = src;
        destroySystemTexture();
        markDirty();
    }
    return *this;
}

// ============================================================
// FONTAWESOME
// ============================================================
void KitsuIconView::updateFATexture(SDL_Renderer* renderer) {
    // Cache válida
    if (fa_texture &&
        cached_unicode == unicode &&
        cached_size == size) {
        return;
    }
    
    destroyFATexture();
    
    if (!g_font || unicode == 0) return;
    
    // Color por defecto si no se configuró
    Uint8 cr = cached_r ? cached_r : 40;
    Uint8 cg = cached_g ? cached_g : 40;
    Uint8 cb = cached_b ? cached_b : 45;
    
    SDL_Color c = { cr, cg, cb, 255 };
    SDL_Surface* surface = TTF_RenderGlyph_Blended(g_font, unicode, c);
    if (!surface) return;
    
    fa_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    cached_unicode = unicode;
    cached_size = size;
}

// ============================================================
// RENDER
// ============================================================
void KitsuIconView::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    if (bounds.w <= 0 || bounds.h <= 0) return;
    
    SDL_Rect abs = getRenderBounds();
    
    if (source == IconSource::SYSTEM_THEME) {
        // ===== ¿Cambió el renderer? → recargar =====
        if (system_renderer != renderer) {
            destroySystemTexture();
            system_renderer = renderer;
        }
        
        // ===== Cargar textura si no la tenemos =====
        if (!system_texture) {
            bool cached = false;
            system_texture = KitsuIconCache::instance().getTexture(
                renderer, icon_name, size, cached);
            
            // Si NO viene del cache, la textura es nuestra → destruirla
            owns_system_texture = !cached;
        }
        
        if (!system_texture) {
            // Icono no encontrado
            clearDirty();
            return;
        }
        
        // ===== Dibujar centrado =====
        int tex_w = 0, tex_h = 0;
        SDL_QueryTexture(system_texture, nullptr, nullptr, &tex_w, &tex_h);
        
        SDL_Rect dst = {
            abs.x + (abs.w - tex_w) / 2,
            abs.y + (abs.h - tex_h) / 2,
            tex_w, tex_h
        };
        SDL_RenderCopy(renderer, system_texture, nullptr, &dst);
    }
    else {
        // ===== FontAwesome =====
        updateFATexture(renderer);
        if (!fa_texture) {
            clearDirty();
            return;
        }
        
        int tex_w = 0, tex_h = 0;
        SDL_QueryTexture(fa_texture, nullptr, nullptr, &tex_w, &tex_h);
        
        SDL_Rect dst = {
            abs.x + (abs.w - tex_w) / 2,
            abs.y + (abs.h - tex_h) / 2,
            tex_w, tex_h
        };
        SDL_RenderCopy(renderer, fa_texture, nullptr, &dst);
    }
    
    clearDirty();
}

} // namespace KitsuGui
