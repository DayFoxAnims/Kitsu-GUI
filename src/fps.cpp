#include "kitsugui/fps.h"
#include <cstdio>

namespace KitsuGui {

TTF_Font* KitsuFPSView::g_font = nullptr;
KitsuFPSView* KitsuFPSView::s_instance = nullptr;

KitsuFPSView::KitsuFPSView() {
    bounds = {10, 10, 90, 24};
    requested_w = 90;
    requested_h = 24;
    
    last_second = SDL_GetTicks();
    s_instance = this;
}

KitsuFPSView::~KitsuFPSView() {
    if (text_texture) SDL_DestroyTexture(text_texture);
    if (s_instance == this) s_instance = nullptr;
}

void KitsuFPSView::frameRendered() {
    frame_count++;
    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - last_second;
    
    if (elapsed >= 500) {
        float fps = frame_count * 1000.0f / (float)elapsed;
        
        history[history_idx] = fps;
        history_idx = (history_idx + 1) % 8;
        
        float sum = 0;
        int count = 0;
        for (int i = 0; i < 8; i++) {
            if (history[i] > 0) { sum += history[i]; count++; }
        }
        float avg = (count > 0) ? sum / count : 0;
        
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f fps", avg);
        
        if (cached_text != buf) {
            cached_text = buf;
            markDirty();
        }
        
        frame_count = 0;
        last_second = now;
    }
}

void KitsuFPSView::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    
    if (cached_text.empty()) cached_text = "0 fps";
    
    // Regenerar textura si el texto cambió
    if (!text_texture || last_rendered != cached_text) {
        if (text_texture) {
            SDL_DestroyTexture(text_texture);
            text_texture = nullptr;
        }
        last_rendered = cached_text;
        
        if (g_font) {
            SDL_Color fg = { 255, 220, 60, 255 };
            SDL_Surface* surface = TTF_RenderUTF8_Blended(g_font, cached_text.c_str(), fg);
            if (surface) {
                text_texture = SDL_CreateTextureFromSurface(renderer, surface);
                tex_w = surface->w;
                tex_h = surface->h;
                SDL_FreeSurface(surface);
                
                bounds.w = tex_w + 16;
                bounds.h = tex_h + 8;
                requested_w = bounds.w;
                requested_h = bounds.h;
            }
        }
    }
    
    if (!text_texture) {
        clearDirty();
        return;
    }
    
    SDL_Rect abs = getAbsoluteBounds();
    
    // Fondo negro semi-transparente
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &abs);
    
    // Texto centrado
    SDL_Rect text_rect = {
        abs.x + (abs.w - tex_w) / 2,
        abs.y + (abs.h - tex_h) / 2,
        tex_w, tex_h
    };
    SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);
    
    clearDirty();
}

} // namespace KitsuGui
