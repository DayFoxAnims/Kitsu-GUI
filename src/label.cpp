#include "kitsugui/label.h"
#include "kitsugui/fonts.h"
#include "kitsugui/theme.h"
#include "kitsugui/utils.h"

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuLabel::KitsuLabel(const std::string& text)
    : text_(text) {
    autoSize();
}

KitsuLabel::~KitsuLabel() {
    destroyTexture();
}

// ============================================================
// Contenido
// ============================================================
KitsuLabel* KitsuLabel::text(const std::string& t) {
    if (text_ != t) {
        text_ = t;
        autoSize();
        invalidate();
    }
    return this;
}

// ============================================================
// Apariencia
// ============================================================
KitsuLabel* KitsuLabel::font(TTF_Font* f) {
    font_ = f;
    autoSize();
    invalidate();
    return this;
}

KitsuLabel* KitsuLabel::color(const Color& c) {
    color_ = c;
    invalidate();
    return this;
}

KitsuLabel* KitsuLabel::color(Uint8 r, Uint8 g, Uint8 b) {
    color_ = Color(r, g, b);
    invalidate();
    return this;
}

KitsuLabel* KitsuLabel::align(TextAlign a) {
    align_ = a;
    invalidate();
    return this;
}

KitsuLabel* KitsuLabel::valign(TextVAlign va) {
    valign_ = va;
    invalidate();
    return this;
}

KitsuLabel* KitsuLabel::wrap(int max_width) {
    wrap_width_ = max_width;
    autoSize();
    invalidate();
    return this;
}

KitsuLabel* KitsuLabel::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    invalidate();
    return this;
}

// ============================================================
// Helpers internos
// ============================================================
TTF_Font* KitsuLabel::activeFont() const {
    return font_ ? font_ : KitsuFonts::normal();
}

void KitsuLabel::autoSize() {
    if (manual_size_) return;
    TTF_Font* f = activeFont();
    if (!f || text_.empty()) {
        if (desired_w <= 0) desired_w = 1;
        if (desired_h <= 0) desired_h = 1;
        return;
    }

    int tw = 0, th = 0;

    if (wrap_width_ > 0) {
        // Con wrap: calcular ancho/alto con TTF_SizeUTF8 + wrap manual
        // (TTF no tiene una función que solo mida con wrap sin renderizar,
        //  así que estimamos el alto con el número de líneas)
        Utils::measureText(f, text_, tw, th);
        tw = wrap_width_;
        // Estimar número de líneas
        int line_h = TTF_FontLineSkip(f);
        // Aproximación: cuántas líneas caben
        int full_w = Utils::textWidth(f, text_);
        int lines = (full_w + wrap_width_ - 1) / wrap_width_;
        if (lines < 1) lines = 1;
        th = lines * line_h;
    } else {
        Utils::measureText(f, text_, tw, th);
    }

    desired_w = tw;
    desired_h = th;

    // Solo sobreescribir bounds si el usuario no fijó tamaño manual
    if (bounds.w <= 0) bounds.w = tw;
    if (bounds.h <= 0) bounds.h = th;
}

void KitsuLabel::destroyTexture() {
    if (text_texture_) {
        SDL_DestroyTexture(text_texture_);
        text_texture_ = nullptr;
        tex_w_ = tex_h_ = 0;
    }
}

void KitsuLabel::updateTextTexture(SDL_Renderer* renderer, Color color) {
    TTF_Font* active = activeFont();

    Uint8 r = (Uint8)color.r;
    Uint8 g = (Uint8)color.g;
    Uint8 b = (Uint8)color.b;

    if (text_texture_ &&
        cached_text_ == text_ &&
        cached_font_ == active &&
        cached_r_ == r && cached_g_ == g && cached_b_ == b &&
        cached_wrap_ == wrap_width_) {
        return;
    }

    destroyTexture();
    if (!active || text_.empty() || !renderer) return;

    SDL_Color fg = { r, g, b, 255 };
    SDL_Surface* surface = nullptr;

    if (wrap_width_ > 0) {
        surface = TTF_RenderUTF8_Blended_Wrapped(
            active, text_.c_str(), fg, wrap_width_);
    } else {
        surface = TTF_RenderUTF8_Blended(active, text_.c_str(), fg);
    }

    if (!surface) return;

    text_texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    tex_w_ = surface->w;
    tex_h_ = surface->h;
    SDL_FreeSurface(surface);

    cached_text_ = text_;
    cached_font_ = active;
    cached_r_ = r;
    cached_g_ = g;
    cached_b_ = b;
    cached_wrap_ = wrap_width_;
}

// ============================================================
// Render
// ============================================================
void KitsuLabel::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    // Resolver color
    Color final_color = color_;
    if (final_color.r < 0) {
        final_color = KitsuTheme::active().text_primary;
    }

    updateTextTexture(renderer, final_color);
    if (!text_texture_) {
        clearNeedsRender();
        return;
    }

    SDL_Rect r = visualRect();

    int x = r.x;
    int y = r.y;

    if (r.w > 0) {
        switch (align_) {
            case TextAlign::LEFT:   x = r.x; break;
            case TextAlign::CENTER: x = r.x + (r.w - tex_w_) / 2; break;
            case TextAlign::RIGHT:  x = r.x + r.w - tex_w_; break;
        }
    }
    if (r.h > 0) {
        switch (valign_) {
            case TextVAlign::TOP:    y = r.y; break;
            case TextVAlign::MIDDLE: y = r.y + (r.h - tex_h_) / 2; break;
            case TextVAlign::BOTTOM: y = r.y + r.h - tex_h_; break;
        }
    }

    SDL_Rect dst = { x, y, tex_w_, tex_h_ };
    SDL_RenderCopy(renderer, text_texture_, nullptr, &dst);

    clearNeedsRender();
}

} // namespace KitsuGui
