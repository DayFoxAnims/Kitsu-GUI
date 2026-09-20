#include "kitsugui/label.h"

namespace KitsuGui {

TTF_Font* KitsuLabel::g_default_font = nullptr;

void KitsuLabel::setDefaultFont(TTF_Font* font) { g_default_font = font; }
TTF_Font* KitsuLabel::getDefaultFont() { return g_default_font; }

KitsuLabel::KitsuLabel(const std::string& text, int width, int height)
    : text(text) {
    bounds.w = width;
    bounds.h = height;
    
    // Color por defecto: text_primary del tema actual
    KitsuTheme& t = KitsuTheme::current();
    color_r = (Uint8)t.text_primary.r;
    color_g = (Uint8)t.text_primary.g;
    color_b = (Uint8)t.text_primary.b;
}

KitsuLabel::~KitsuLabel() { destroyTexture(); }

void KitsuLabel::destroyTexture() {
    if (text_texture) {
        SDL_DestroyTexture(text_texture);
        text_texture = nullptr;
        tex_w = tex_h = 0;
    }
}

KitsuLabel& KitsuLabel::withFont(TTF_Font* f) { font = f; markDirty(); return *this; }
KitsuLabel& KitsuLabel::withColor(Uint8 r, Uint8 g, Uint8 b) {
    color_r = r; color_g = g; color_b = b; markDirty(); return *this;
}
KitsuLabel& KitsuLabel::withAlign(TextAlign a) { align = a; markDirty(); return *this; }
KitsuLabel& KitsuLabel::withVAlign(TextVAlign va) { valign = va; markDirty(); return *this; }
KitsuLabel& KitsuLabel::withWrap(int max_width) { wrap_width = max_width; markDirty(); return *this; }

void KitsuLabel::setText(const std::string& new_text) {
    if (text != new_text) {
        text = new_text;
        markDirty();
    }
}

void KitsuLabel::updateTextTexture(SDL_Renderer* renderer) {
    TTF_Font* active = getActiveFont();
    
    if (text_texture &&
        cached_text == text &&
        cached_r == color_r && cached_g == color_g && cached_b == color_b &&
        cached_wrap == wrap_width &&
        cached_font == active) {
        return;
    }
    
    destroyTexture();
    if (!active || text.empty() || !renderer) return;
    
    SDL_Color fg = {color_r, color_g, color_b, 255};
    SDL_Surface* surface = nullptr;
    
    if (wrap_width > 0) {
        surface = TTF_RenderUTF8_Blended_Wrapped(active, text.c_str(), fg, wrap_width);
    } else {
        surface = TTF_RenderUTF8_Blended(active, text.c_str(), fg);
    }
    
    if (!surface) return;
    
    text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w = surface->w;
    tex_h = surface->h;
    SDL_FreeSurface(surface);
    
    cached_text = text;
    cached_r = color_r; cached_g = color_g; cached_b = color_b;
    cached_wrap = wrap_width;
    cached_font = active;
}

void KitsuLabel::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    updateTextTexture(renderer);
    if (!text_texture) { clearDirty(); return; }
    
    SDL_Rect abs = getRenderBounds();
    int x = abs.x, y = abs.y;
    
    if (bounds.w > 0) {
        switch (align) {
            case TextAlign::LEFT:   x = abs.x; break;
            case TextAlign::CENTER: x = abs.x + (bounds.w - tex_w) / 2; break;
            case TextAlign::RIGHT:  x = abs.x + bounds.w - tex_w; break;
        }
    }
    if (bounds.h > 0) {
        switch (valign) {
            case TextVAlign::TOP:    y = abs.y; break;
            case TextVAlign::MIDDLE: y = abs.y + (bounds.h - tex_h) / 2; break;
            case TextVAlign::BOTTOM: y = abs.y + bounds.h - tex_h; break;
        }
    }
    
    SDL_Rect dst = {x, y, tex_w, tex_h};
    SDL_RenderCopy(renderer, text_texture, nullptr, &dst);
    
    clearDirty();
}

} // namespace KitsuGui
