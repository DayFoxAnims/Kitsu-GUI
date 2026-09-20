#include "kitsugui/scroll_view.h"
#include "internal.h"

namespace KitsuGui {

KitsuScrollView::KitsuScrollView(bool vertical, bool horizontal)
    : vertical(vertical), horizontal(horizontal) {
    bounds = {0, 0, 300, 200};
    requested_w = 300;
    requested_h = 200;
    
    content = new KitsuBox(false);
    content->setBounds(0, 0, 300, 0);
    content->requested_w = 300;
    content->requested_h = 0;
    content->setAutoLayout(true);
    content->setAlignment(KitsuAlign::START);
    content->setJustify(KitsuJustify::START);
    content->setSpacing(8);
    content->setPadding(10);
    
    content->parent = this;
}

KitsuScrollView::~KitsuScrollView() {
    if (scrollbar_animating && g_active_animations > 0) g_active_animations--;
    delete content;
}

KitsuScrollView& KitsuScrollView::setBounds(int x, int y, int w, int h) {
    setBoundsInternal(x, y, w, h);
    return *this;
}

KitsuScrollView& KitsuScrollView::withScrollbar(bool show) {
    show_scrollbar = show;
    markDirty();
    return *this;
}

KitsuScrollView& KitsuScrollView::withScrollbarColors(const Color& track, const Color& thumb) {
    scrollbar_track = track;
    scrollbar_thumb = thumb;
    markDirty();
    return *this;
}

KitsuScrollView& KitsuScrollView::withBottomPadding(int p) {
    bottom_padding = p;
    markDirty();
    return *this;
}

KitsuScrollView& KitsuScrollView::withScrollbarWidth(float base, float expanded) {
    scrollbar_base_width = base;
    scrollbar_hover_width = expanded;
    markDirty();
    return *this;
}

void KitsuScrollView::addChild(KitsuWidget* child, bool owns) {
    if (!child) return;
    content->addChild(child, owns);
    markDirty();
}

void KitsuScrollView::removeChild(KitsuWidget* child) {
    content->removeChild(child);
    markDirty();
}

void KitsuScrollView::clearChildren() {
    content->clearChildren();
    markDirty();
}

// ===== LAYOUT =====
void KitsuScrollView::updateLayout() {
    if (!content) return;
    if (bounds.w == 0 || bounds.h == 0) return;
    
    int content_w = bounds.w - (vertical && show_scrollbar ? 10 : 0);
    if (content_w < 0) content_w = 0;
    
    content->bounds.w = content_w;
    content->requested_w = content_w;
    content->bounds.h = 0;
    content->requested_h = 0;
    content->bounds.x = 0;
    content->bounds.y = 0;
    
    content->updateLayout();
    
    computeScrollLimits();
    clampScroll();
}

void KitsuScrollView::computeScrollLimits() {
    if (!content) return;
    
    int content_h = content->bounds.h;
    
    if (vertical) {
        max_scroll_y = content_h + bottom_padding - bounds.h;
        if (max_scroll_y < 0) max_scroll_y = 0;
    } else {
        max_scroll_y = 0;
    }
    
    if (horizontal) {
        max_scroll_x = content->bounds.w - bounds.w;
        if (max_scroll_x < 0) max_scroll_x = 0;
    } else {
        max_scroll_x = 0;
    }
}

void KitsuScrollView::clampScroll() {
    if (scroll_y < 0) scroll_y = 0;
    if (scroll_y > max_scroll_y) scroll_y = max_scroll_y;
    
    if (scroll_x < 0) scroll_x = 0;
    if (scroll_x > max_scroll_x) scroll_x = max_scroll_x;
}

void KitsuScrollView::scrollTo(int y) {
    scroll_y = y;
    clampScroll();
    markDirty();
}

void KitsuScrollView::scrollBy(int dy) {
    scroll_y += dy;
    clampScroll();
    markDirty();
}

// ===== GEOMETRÍA DEL SCROLLBAR =====
SDL_Rect KitsuScrollView::getScrollbarTrack(const SDL_Rect& abs) const {
    SDL_Rect track;
    track.x = abs.x + abs.w - (int)scrollbar_width - 2;
    track.y = abs.y + 2;
    track.w = (int)scrollbar_width;
    track.h = abs.h - 4;
    return track;
}

SDL_Rect KitsuScrollView::getScrollbarThumb(const SDL_Rect& abs) const {
    SDL_Rect track = getScrollbarTrack(abs);
    
    if (max_scroll_y <= 0) return {0, 0, 0, 0};
    
    int total_scrollable = bounds.h + max_scroll_y;
    float visible_ratio = (float)bounds.h / (float)total_scrollable;
    
    int thumb_h = (int)(track.h * visible_ratio);
    if (thumb_h < 30) thumb_h = 30;
    if (thumb_h > track.h) thumb_h = track.h;
    
    float scroll_ratio = (max_scroll_y > 0) ? (float)scroll_y / (float)max_scroll_y : 0;
    int thumb_y = track.y + (int)((track.h - thumb_h) * scroll_ratio);
    
    SDL_Rect thumb = { track.x, thumb_y, track.w, thumb_h };
    return thumb;
}

bool KitsuScrollView::isInScrollbar(int mx, int my, const SDL_Rect& abs) const {
    if (!show_scrollbar || !vertical || max_scroll_y <= 0) return false;
    
    SDL_Rect zone = {
        abs.x + abs.w - 14,
        abs.y,
        14,
        abs.h
    };
    
    return (mx >= zone.x && mx < zone.x + zone.w &&
            my >= zone.y && my < zone.y + zone.h);
}

// ===== EVENTOS =====
bool KitsuScrollView::handleEvent(const SDL_Event& e) {
    if (!visible) return false;
    
    SDL_Rect abs = getAbsoluteBounds();
    
    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x;
            int my = e.motion.y;
            
            // Hover sobre el scrollbar
            bool was_hover = scrollbar_hover;
            scrollbar_hover = isInScrollbar(mx, my, abs);
            
            if (was_hover != scrollbar_hover) {
                scrollbar_target_width = scrollbar_hover
                    ? scrollbar_hover_width
                    : scrollbar_base_width;
            }
            
            // Drag del scrollbar
            if (scrollbar_dragging) {
                int dy = my - scrollbar_drag_start_y;
                int track_h = abs.h - 4;
                
                if (max_scroll_y > 0) {
                    SDL_Rect thumb = getScrollbarThumb(abs);
                    float scroll_per_pixel = (float)max_scroll_y / (float)(track_h - thumb.h);
                    
                    scroll_y = scrollbar_drag_start_scroll + (int)(dy * scroll_per_pixel);
                    clampScroll();
                    markDirty();
                }
                return true;
            }
            
            // Drag normal (scroll con mouse dentro del área)
            if (mouse_down_inside && vertical && max_scroll_y > 0 && !dragging) {
                int dy = my - drag_start_y;
                if (dy > 8 || dy < -8) {
                    dragging = true;
                }
            }
            
            if (dragging) {
                int dy = my - drag_start_y;
                scroll_y = drag_start_scroll_y - dy;
                clampScroll();
                markDirty();
                return true;
            }
            
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;
            
            // Click en el scrollbar
            if (isInScrollbar(mx, my, abs)) {
                SDL_Rect thumb = getScrollbarThumb(abs);
                
                if (mx >= thumb.x && mx < thumb.x + thumb.w &&
                    my >= thumb.y && my < thumb.y + thumb.h) {
                    scrollbar_dragging = true;
                    scrollbar_drag_start_y = my;
                    scrollbar_drag_start_scroll = scroll_y;
                    scrollbar_target_width = scrollbar_hover_width;
                    markDirty();
                    return true;
                }
                
                SDL_Rect track = getScrollbarTrack(abs);
                if (my >= track.y && my < track.y + track.h) {
                    float ratio = (float)(my - track.y) / (float)track.h;
                    scroll_y = (int)(ratio * (float)max_scroll_y);
                    clampScroll();
                    markDirty();
                    return true;
                }
            }
            
            // Click normal dentro del scrollview
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (inside) {
                mouse_down_inside = true;
                drag_start_y = my;
                drag_start_scroll_y = scroll_y;
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            if (scrollbar_dragging) {
                scrollbar_dragging = false;
                if (!scrollbar_hover) {
                    scrollbar_target_width = scrollbar_base_width;
                }
                markDirty();
                return true;
            }
            mouse_down_inside = false;
            dragging = false;
            break;
        }
        
        case SDL_MOUSEWHEEL: {
            int mx = e.wheel.mouseX;
            int my = e.wheel.mouseY;
            
            if (mx == 0 && my == 0) {
                SDL_GetMouseState(&mx, &my);
            }
            
            bool inside = (mx >= abs.x && mx < abs.x + abs.w &&
                           my >= abs.y && my < abs.y + abs.h);
            
            if (inside) {
                if (vertical && e.wheel.y != 0) {
                    scrollBy(-e.wheel.y * 40);
                    return true;
                }
                if (horizontal && e.wheel.x != 0) {
                    scroll_x -= e.wheel.x * 40;
                    clampScroll();
                    markDirty();
                    return true;
                }
            }
            break;
        }
    }
    
    // ===== Propagar a hijos con coordenadas traducidas =====
    // Los hijos usan getAbsoluteBounds() (lógica) para el hit-test,
    // y como content->bounds nunca se modifica, la posición lógica es
    // consistente. Solo hay que sumar scroll_y al evento del mouse.
    if (content) {
        SDL_Event adjusted = e;
        switch (e.type) {
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                adjusted.button.y += scroll_y;
                break;
            case SDL_MOUSEMOTION:
                adjusted.motion.y += scroll_y;
                break;
        }
        
        if (content->handleEvent(adjusted)) return true;
    }
    
    return false;
}

// ===== TICK =====
void KitsuScrollView::tick() {
    // Animación del ancho del scrollbar
    if (scrollbar_width != scrollbar_target_width) {
        float diff = scrollbar_target_width - scrollbar_width;
        float step = diff * 0.3f;
        
        if (step > -0.5f && step < 0.5f) {
            scrollbar_width = scrollbar_target_width;
        } else {
            scrollbar_width += step;
        }
        
        markDirty();
        
        if (!scrollbar_animating) {
            scrollbar_animating = true;
            g_active_animations++;
        }
    } else if (scrollbar_animating) {
        scrollbar_animating = false;
        if (g_active_animations > 0) g_active_animations--;
    }
    
    if (content) content->tick();
}

// ===== RENDER =====
void KitsuScrollView::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    if (!content) return;
    
    SDL_Rect abs = getRenderBounds();
    
    // Clipping: solo dentro del viewport
    SDL_RenderSetClipRect(renderer, &abs);
    
    // ===== Aplicar el offset visual al content (temporal) =====
    // content->bounds NO se modifica. Solo se le asigna un render_offset.
    // Los hijos calcularán su posición de dibujo con getRenderBounds(),
    // que suma los render_offset de sus ancestros.
    content->render_offset_x = 0;
    content->render_offset_y = -scroll_y;
    
    // Renderizar el contenido
    content->render(renderer);
    
    // Limpiar el offset (por si acaso)
    content->render_offset_x = 0;
    content->render_offset_y = 0;
    
    SDL_RenderSetClipRect(renderer, nullptr);
    
    // ===== Scrollbar =====
    if (show_scrollbar && vertical && max_scroll_y > 0) {
        SDL_Rect track = getScrollbarTrack(abs);
        SDL_Rect thumb = getScrollbarThumb(abs);
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        
        // Track
        SDL_SetRenderDrawColor(renderer,
            (Uint8)scrollbar_track.r, (Uint8)scrollbar_track.g,
            (Uint8)scrollbar_track.b, (Uint8)scrollbar_track.a);
        SDL_RenderFillRect(renderer, &track);
        
        // Thumb
        Color thumb_color = scrollbar_thumb;
        if (scrollbar_hover || scrollbar_dragging) {
            thumb_color = scrollbar_thumb_hover;
        }
        
        SDL_SetRenderDrawColor(renderer,
            (Uint8)thumb_color.r, (Uint8)thumb_color.g,
            (Uint8)thumb_color.b, (Uint8)thumb_color.a);
        SDL_RenderFillRect(renderer, &thumb);
        
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
    
    clearDirty();
}

} // namespace KitsuGui
