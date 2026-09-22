#include "kitsugui/fps.h"
#include "kitsugui/fonts.h"
#include <cstdio>

namespace KitsuGui {

KitsuFPSView* KitsuFPSView::s_instance_ = nullptr;

// ============================================================
// Constructor / destructor
// ============================================================
KitsuFPSView::KitsuFPSView() {
    desired_w = 90;
    desired_h = 24;
    bounds = {10, 10, 90, 24};
    last_second_ = SDL_GetTicks();
    s_instance_ = this;
}

KitsuFPSView::~KitsuFPSView() {
    if (text_texture_) SDL_DestroyTexture(text_texture_);
    if (s_instance_ == this) s_instance_ = nullptr;
}

// ============================================================
// Medición
// ============================================================
void KitsuFPSView::frameRendered() {
    frame_count_++;
    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - last_second_;

    if (elapsed >= 500) {
        float fps = frame_count_ * 1000.0f / (float)elapsed;

        history_[history_idx_] = fps;
        history_idx_ = (history_idx_ + 1) % 8;

        float sum = 0;
        int count = 0;
        for (int i = 0; i < 8; i++) {
            if (history_[i] > 0) { sum += history_[i]; count++; }
        }
        float avg = (count > 0) ? sum / count : 0;

        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f fps", avg);

        if (cached_text_ != buf) {
            cached_text_ = buf;
            invalidate();
        }

        frame_count_ = 0;
        last_second_ = now;
    }
}

// ============================================================
// Render
// ============================================================
void KitsuFPSView::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;

    if (cached_text_.empty()) cached_text_ = "0 fps";

    if (!text_texture_ || last_rendered_ != cached_text_) {
        if (text_texture_) {
            SDL_DestroyTexture(text_texture_);
            text_texture_ = nullptr;
        }
        last_rendered_ = cached_text_;

        TTF_Font* font = KitsuFonts::mono();
        if (font) {
            SDL_Color fg = { 255, 220, 60, 255 };
            SDL_Surface* surface = TTF_RenderUTF8_Blended(
                font, cached_text_.c_str(), fg);
            if (surface) {
                text_texture_ = SDL_CreateTextureFromSurface(renderer, surface);
                tex_w_ = surface->w;
                tex_h_ = surface->h;
                SDL_FreeSurface(surface);

                desired_w = tex_w_ + 16;
                desired_h = tex_h_ + 8;
                bounds.w = desired_w;
                bounds.h = desired_h;
            }
        }
    }

    if (!text_texture_) {
        clearNeedsRender();
        return;
    }

    SDL_Rect r = visualRect();

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &r);

    SDL_Rect text_rect = {
        r.x + (r.w - tex_w_) / 2,
        r.y + (r.h - tex_h_) / 2,
        tex_w_, tex_h_
    };
    SDL_RenderCopy(renderer, text_texture_, nullptr, &text_rect);

    clearNeedsRender();
}

} // namespace KitsuGui
