#include "kitsugui/scroll_view.h"
#include "kitsugui/theme.h"
#include "internal.h"
#include <algorithm>

namespace KitsuGui {

// ============================================================
// Constructor / destructor
// ============================================================
KitsuScrollView::KitsuScrollView(bool vertical, bool horizontal)
    : vertical_(vertical), horizontal_(horizontal) {
    desired_w = 300;
    desired_h = 200;
    bounds = {0, 0, 300, 200};

    content_ = new KitsuBox(false);   // vertical
    content_->parent = this;
    content_->autoLayout(true);
    content_->align(KitsuAlign::START);
    content_->justify(KitsuJustify::START);
    content_->spacing(8);
    content_->padding(10);
    content_->desired_w = 300;
    content_->desired_h = 0;
    content_->bounds.x = 0;
    content_->bounds.y = 0;
}

KitsuScrollView::~KitsuScrollView() {
    if (scrollbar_animating_ && g_active_animations > 0) {
        g_active_animations--;
    }
    // content_ NO se destruye con delete porque su parent es this
    // pero lo borramos manualmente para evitar leaks
    delete content_;
    content_ = nullptr;
}

// ============================================================
// Contenido
// ============================================================
KitsuScrollView* KitsuScrollView::add(KitsuWidget* child, bool owns) {
    if (content_) content_->add(child, owns);
    return this;
}

void KitsuScrollView::remove(KitsuWidget* child) {
    if (content_) content_->remove(child);
}

void KitsuScrollView::clear() {
    if (content_) content_->clear();
}

// ============================================================
// Configuración
// ============================================================
KitsuScrollView* KitsuScrollView::size(int w, int h) {
    desired_w = w;
    desired_h = h;
    bounds.w = w;
    bounds.h = h;
    manual_size_ = true;
    invalidate();
    return this;
}

KitsuScrollView* KitsuScrollView::scrollbar(bool show) {
    show_scrollbar_ = show;
    invalidate();
    return this;
}

KitsuScrollView* KitsuScrollView::scrollbarColors(const Color& track,
                                                 const Color& thumb) {
    scrollbar_track_ = track;
    scrollbar_thumb_ = thumb;
    invalidate();
    return this;
}

KitsuScrollView* KitsuScrollView::bottomPadding(int p) {
    bottom_padding_ = p;
    invalidate();
    return this;
}

KitsuScrollView* KitsuScrollView::scrollbarWidth(float base, float expanded) {
    scrollbar_base_width_ = base;
    scrollbar_hover_width_ = expanded;
    invalidate();
    return this;
}

// ============================================================
// Scroll
// ============================================================
KitsuScrollView* KitsuScrollView::scrollTo(int y) {
    scroll_y_ = y;
    clampScroll();
    invalidate();
    return this;
}

KitsuScrollView* KitsuScrollView::scrollBy(int dy) {
    scroll_y_ += dy;
    clampScroll();
    invalidate();
    return this;
}

void KitsuScrollView::clampScroll() {
    if (scroll_y_ < 0) scroll_y_ = 0;
    if (scroll_y_ > max_scroll_y_) scroll_y_ = max_scroll_y_;
    if (scroll_x_ < 0) scroll_x_ = 0;
    if (scroll_x_ > max_scroll_x_) scroll_x_ = max_scroll_x_;
}

// ============================================================
// Layout
// ============================================================
void KitsuScrollView::updateLayout() {
    if (!content_) return;

    // El bounds lo pone el padre (KitsuBox::layoutHorizontal/Vertical).
    // Si aún no tiene tamaño, no podemos hacer nada.
    if (bounds.w <= 0 || bounds.h <= 0) return;

    // Ancho disponible para el content
    int content_w = bounds.w;
    if (vertical_ && show_scrollbar_) content_w -= 12;
    if (content_w < 0) content_w = 0;

    content_->bounds.w = content_w;
    content_->bounds.h = 0;
    content_->bounds.x = 0;
    content_->bounds.y = 0;

    // IMPORTANTE: no tocamos desired_w/h del content.
    // Los dejamos en 0 para que auto-calcule.

    content_->updateLayout();

	// Si el content quedó más chico que el viewport, estirarlo
	if (content_->bounds.w < content_w) {
		content_->bounds.w = content_w;
		content_->updateLayout();   // recalcular hijos con el nuevo ancho
	}

	computeLimits();
	clampScroll();

    // Aplicar el scroll como transform al content
    content_->transform.offset_x = -scroll_x_;
    content_->transform.offset_y = -scroll_y_;
}

void KitsuScrollView::computeLimits() {
    if (!content_) return;

    int content_h = content_->bounds.h;
    int content_w = content_->bounds.w;

    if (vertical_) {
        max_scroll_y_ = content_h + bottom_padding_ - bounds.h;
        if (max_scroll_y_ < 0) max_scroll_y_ = 0;
    } else {
        max_scroll_y_ = 0;
    }

    if (horizontal_) {
        max_scroll_x_ = content_w + bottom_padding_ - bounds.w;
        if (max_scroll_x_ < 0) max_scroll_x_ = 0;
    } else {
        max_scroll_x_ = 0;
    }
}

// ============================================================
// Geometría del scrollbar
// ============================================================
SDL_Rect KitsuScrollView::scrollbarTrack(const SDL_Rect& abs) const {
    SDL_Rect tr;
    tr.x = abs.x + abs.w - (int)scrollbar_width_ - 2;
    tr.y = abs.y + 2;
    tr.w = (int)scrollbar_width_;
    tr.h = abs.h - 4;
    return tr;
}

SDL_Rect KitsuScrollView::scrollbarThumb(const SDL_Rect& abs) const {
    SDL_Rect track = scrollbarTrack(abs);
    if (max_scroll_y_ <= 0) return {0, 0, 0, 0};

    int total = bounds.h + max_scroll_y_;
    float visible_ratio = (float)bounds.h / (float)total;

    int thumb_h = (int)(track.h * visible_ratio);
    if (thumb_h < 30) thumb_h = 30;
    if (thumb_h > track.h) thumb_h = track.h;

    float scroll_ratio = (float)scroll_y_ / (float)max_scroll_y_;
    int thumb_y = track.y + (int)((track.h - thumb_h) * scroll_ratio);

    return { track.x, thumb_y, track.w, thumb_h };
}

bool KitsuScrollView::isInScrollbar(int mx, int my,
                                    const SDL_Rect& abs) const {
    if (!show_scrollbar_ || !vertical_ || max_scroll_y_ <= 0) return false;

    SDL_Rect zone = {
        abs.x + abs.w - 14,
        abs.y,
        14,
        abs.h
    };
    return (mx >= zone.x && mx < zone.x + zone.w &&
            my >= zone.y && my < zone.y + zone.h);
}

// ============================================================
// Eventos
// ============================================================
bool KitsuScrollView::handleEvent(const SDL_Event& e) {
    if (!visible || !enabled || disabled_) return false;

    SDL_Rect r = globalRect();

    switch (e.type) {
        case SDL_MOUSEMOTION: {
            int mx = e.motion.x;
            int my = e.motion.y;

            bool was_hover = scrollbar_hover_;
            scrollbar_hover_ = isInScrollbar(mx, my, r);
            if (was_hover != scrollbar_hover_) {
                scrollbar_target_width_ = scrollbar_hover_
                    ? scrollbar_hover_width_
                    : scrollbar_base_width_;
            }

            if (scrollbar_dragging_) {
                int dy = my - scrollbar_drag_start_y_;
                int track_h = r.h - 4;
                if (max_scroll_y_ > 0) {
                    SDL_Rect thumb = scrollbarThumb(r);
                    float per_px = (float)max_scroll_y_ /
                                   (float)(track_h - thumb.h);
                    scroll_y_ = scrollbar_drag_start_scroll_ +
                                (int)(dy * per_px);
                    clampScroll();
                    invalidate();
                }
                return true;
            }

            if (mouse_down_inside_ && vertical_ &&
                max_scroll_y_ > 0 && !dragging_) {
                int dy = my - drag_start_y_;
                if (dy > 8 || dy < -8) dragging_ = true;
            }

            if (dragging_) {
                int dy = my - drag_start_y_;
                scroll_y_ = drag_start_scroll_y_ - dy;
                clampScroll();
                invalidate();
                return true;
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button != SDL_BUTTON_LEFT) break;
            int mx = e.button.x, my = e.button.y;

            if (isInScrollbar(mx, my, r)) {
                SDL_Rect thumb = scrollbarThumb(r);
                if (mx >= thumb.x && mx < thumb.x + thumb.w &&
                    my >= thumb.y && my < thumb.y + thumb.h) {
                    scrollbar_dragging_ = true;
                    scrollbar_drag_start_y_ = my;
                    scrollbar_drag_start_scroll_ = scroll_y_;
                    scrollbar_target_width_ = scrollbar_hover_width_;
                    invalidate();
                    return true;
                }

                SDL_Rect track = scrollbarTrack(r);
                if (my >= track.y && my < track.y + track.h) {
                    float ratio = (float)(my - track.y) / (float)track.h;
                    scroll_y_ = (int)(ratio * (float)max_scroll_y_);
                    clampScroll();
                    invalidate();
                    return true;
                }
            }

            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside) {
                mouse_down_inside_ = true;
                drag_start_y_ = my;
                drag_start_scroll_y_ = scroll_y_;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (scrollbar_dragging_) {
                scrollbar_dragging_ = false;
                if (!scrollbar_hover_) {
                    scrollbar_target_width_ = scrollbar_base_width_;
                }
                invalidate();
                return true;
            }
            mouse_down_inside_ = false;
            dragging_ = false;
            break;
        }

        case SDL_MOUSEWHEEL: {
            int mx = e.wheel.mouseX;
            int my = e.wheel.mouseY;
            if (mx == 0 && my == 0) SDL_GetMouseState(&mx, &my);

            bool inside = (mx >= r.x && mx < r.x + r.w &&
                           my >= r.y && my < r.y + r.h);
            if (inside) {
                if (vertical_ && e.wheel.y != 0) {
                    scrollBy(-e.wheel.y * 40);
                    return true;
                }
                if (horizontal_ && e.wheel.x != 0) {
                    scroll_x_ -= e.wheel.x * 40;
                    clampScroll();
                    invalidate();
                    return true;
                }
            }
            break;
        }
    }

    // ===== Propagar a hijos — SIN traducir coordenadas =====
    // Los hijos usan globalRect() que YA incluye el transform
    // de este scroll view, así que el hit-test funciona con las
    // coordenadas reales del evento. Magia.
    if (content_) {
        if (content_->handleEvent(e)) return true;
    }

    return false;
}

// ============================================================
// Tick
// ============================================================
void KitsuScrollView::tick() {
    if (scrollbar_width_ != scrollbar_target_width_) {
        float diff = scrollbar_target_width_ - scrollbar_width_;
        float step = diff * 0.3f;

        if (step > -0.5f && step < 0.5f) {
            scrollbar_width_ = scrollbar_target_width_;
        } else {
            scrollbar_width_ += step;
        }
        invalidate();

        if (!scrollbar_animating_) {
            scrollbar_animating_ = true;
            g_active_animations++;
        }
    } else if (scrollbar_animating_) {
        scrollbar_animating_ = false;
        if (g_active_animations > 0) g_active_animations--;
    }

    if (content_) content_->tick();
}

// ============================================================
// Render
// ============================================================
void KitsuScrollView::render(SDL_Renderer* renderer) {
    if (!visible || !renderer) return;
    if (!content_) return;

    SDL_Rect r = visualRect();

    // Clipping al área del scroll view
    SDL_RenderSetClipRect(renderer, &r);

    // El content ya tiene su transform con -scroll_y, así que sus
    // hijos calculan visualRect() correctamente. No hay que tocar nada.
    content_->render(renderer);

    SDL_RenderSetClipRect(renderer, nullptr);

    // ===== Scrollbar =====
    if (show_scrollbar_ && vertical_ && max_scroll_y_ > 0) {
        SDL_Rect track = scrollbarTrack(r);
        SDL_Rect thumb = scrollbarThumb(r);

        KitsuTheme& t = KitsuTheme::active();

        Color track_c = (scrollbar_track_.r < 0)
            ? t.scrollbar_track : scrollbar_track_;
        Color thumb_c = (scrollbar_thumb_.r < 0)
            ? t.scrollbar_thumb : scrollbar_thumb_;
        Color thumb_hover_c = (scrollbar_thumb_hover_.r < 0)
            ? t.scrollbar_thumb_hover : scrollbar_thumb_hover_;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(renderer,
            (Uint8)track_c.r, (Uint8)track_c.g,
            (Uint8)track_c.b, (Uint8)track_c.a);
        SDL_RenderFillRect(renderer, &track);

        Color final_thumb = (scrollbar_hover_ || scrollbar_dragging_)
            ? thumb_hover_c : thumb_c;

        SDL_SetRenderDrawColor(renderer,
            (Uint8)final_thumb.r, (Uint8)final_thumb.g,
            (Uint8)final_thumb.b, (Uint8)final_thumb.a);
        SDL_RenderFillRect(renderer, &thumb);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    clearNeedsRender();
}

} // namespace KitsuGui
