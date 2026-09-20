#include "kitsugui/box.h"
#include "internal.h"
#include <algorithm>

namespace KitsuGui {

KitsuBox::KitsuBox(bool horizontal)
    : horizontal(horizontal) {
    bounds = {0, 0, 0, 0};
    requested_w = 0;
    requested_h = 0;
}

KitsuBox::~KitsuBox() {
    clearChildren();
}

void KitsuBox::addChild(KitsuWidget* child, bool owns) {
    if (!child) return;
    child->parent = this;
    children.push_back(child);
    if (owns) owned_children.push_back(child);
    markDirty();
}

void KitsuBox::removeChild(KitsuWidget* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it == children.end()) return;
    
    children.erase(it);
    
    auto it_own = std::find(owned_children.begin(), owned_children.end(), child);
    if (it_own != owned_children.end()) {
        owned_children.erase(it_own);
        delete child;
    } else {
        child->parent = nullptr;
    }
    markDirty();
}

void KitsuBox::clearChildren() {
    for (auto* c : owned_children) delete c;
    owned_children.clear();
    children.clear();
    markDirty();
}

void KitsuBox::updateLayout() {
    if (children.empty()) return;
    
    // FASE 0: Fill para contenedores SIN auto-layout
    if (!auto_layout) {
        int inner_w = bounds.w - 2 * padding;
        int inner_h = bounds.h - 2 * padding;
        if (inner_w < 0) inner_w = 0;
        if (inner_h < 0) inner_h = 0;
        
        for (auto* c : children) {
            if (!c || !c->visible) continue;
            
            if (c->requested_w < 0) {
                c->bounds.w = inner_w - 2 * c->margin;
                if (c->bounds.w < 0) c->bounds.w = 0;
            }
            if (c->requested_h < 0) {
                c->bounds.h = inner_h - 2 * c->margin;
                if (c->bounds.h < 0) c->bounds.h = 0;
            }
        }
    }
    
    if (auto_layout) {
        if (horizontal) layoutHorizontal();
        else            layoutVertical();
    }
    
    for (auto* c : children) {
        if (c && c->visible) c->updateLayout();
    }
}

// ===== HORIZONTAL =====
void KitsuBox::layoutHorizontal() {
    bool auto_width  = (requested_w == 0);
    bool auto_height = (requested_h == 0);
    
    int avail_w = auto_width  ? 99999 : bounds.w;
    int avail_h = auto_height ? 99999 : bounds.h;
    
    int inner_x = padding;
    int inner_y = padding;
    int inner_w = avail_w - 2 * padding;
    int inner_h = avail_h - 2 * padding;
    
    if (inner_w <= 0) inner_w = 99999;
    if (inner_h <= 0) inner_h = 99999;
    
    struct Item { KitsuWidget* w; int width; bool flexible; };
    std::vector<Item> items;
    int total_width = 0;
    int flexible_count = 0;
    int max_height = 0;
    
    for (auto* c : children) {
        if (!c || !c->visible) continue;
        
        if (c->requested_w == 0 || c->requested_h == 0) {
            c->updateLayout();
        }
        
        Item it;
        it.w = c;
        it.flexible = (c->requested_w < 0);
        it.width = it.flexible ? 0 : c->requested_w;
        total_width += it.width;
        if (it.flexible) flexible_count++;
        if (c->bounds.h > max_height) max_height = c->bounds.h;
        items.push_back(it);
    }
    
    if (items.empty()) {
        if (auto_width)  bounds.w = padding * 2;
        if (auto_height) bounds.h = padding * 2;
        return;
    }
    
    int spacing_total = (int)(items.size() - 1) * spacing;
    int remaining = inner_w - total_width - spacing_total;
    if (remaining < 0) remaining = 0;
    if (auto_width) remaining = 0;
    
    int flex_width = (flexible_count > 0) ? remaining / flexible_count : 0;
    
    int used_width = total_width + (flexible_count * flex_width) + spacing_total;
    int free_space = inner_w - used_width;
    if (free_space < 0) free_space = 0;
    if (auto_width) free_space = 0;
    
    int start_x = inner_x;
    int extra_spacing = 0;
    
    switch (justify_content) {
        case KitsuJustify::START: break;
        case KitsuJustify::CENTER: start_x += free_space / 2; break;
        case KitsuJustify::END:    start_x += free_space; break;
        case KitsuJustify::SPACE_BETWEEN:
            if (items.size() > 1) extra_spacing = free_space / (int)(items.size() - 1);
            break;
        case KitsuJustify::SPACE_AROUND:
            if (!items.empty()) {
                extra_spacing = free_space / (int)items.size();
                start_x += extra_spacing / 2;
            }
            break;
        case KitsuJustify::SPACE_EVENLY:
            if (!items.empty()) {
                extra_spacing = free_space / (int)(items.size() + 1);
                start_x += extra_spacing;
            }
            break;
    }
    
    int cursor_x = start_x;
    int total_h_used = 0;
    
    for (auto& it : items) {
        KitsuWidget* c = it.w;
        
        int w = it.flexible ? flex_width : it.width;
        if (auto_width && w <= 0) w = c->bounds.w;
        
        int h;
        if (c->requested_h <= 0) {
            h = inner_h;
        } else {
            h = c->requested_h;
        }
        
        c->bounds.w = w;
        c->bounds.h = h;
        c->bounds.x = cursor_x;
        
        switch (align) {
            case KitsuAlign::START:   c->bounds.y = inner_y; break;
            case KitsuAlign::CENTER:  c->bounds.y = inner_y + (inner_h - h) / 2; break;
            case KitsuAlign::END:     c->bounds.y = inner_y + inner_h - h; break;
            case KitsuAlign::STRETCH: c->bounds.y = inner_y; c->bounds.h = inner_h; break;
        }
        
        if (c->bounds.h > total_h_used) total_h_used = c->bounds.h;
        
        cursor_x += w + spacing + extra_spacing;
        c->markDirty();
    }
    
    if (auto_width) {
        int final_w = 0;
        int count = 0;
        for (auto& it : items) {
            final_w += it.w->bounds.w;
            count++;
        }
        if (count > 0) final_w += (count - 1) * spacing;
        final_w += padding * 2;
        bounds.w = final_w;
    }
    
    if (auto_height && total_h_used > 0) {
        bounds.h = total_h_used + padding * 2;
    }
}

// ===== VERTICAL =====
void KitsuBox::layoutVertical() {
    bool auto_width  = (requested_w == 0);
    bool auto_height = (requested_h == 0);
    
    int avail_w = auto_width  ? 99999 : bounds.w;
    int avail_h = auto_height ? 99999 : bounds.h;
    
    int inner_x = padding;
    int inner_y = padding;
    int inner_w = avail_w - 2 * padding;
    int inner_h = avail_h - 2 * padding;
    
    if (inner_w <= 0) inner_w = 99999;
    if (inner_h <= 0) inner_h = 99999;
    
    struct Item { KitsuWidget* w; int height; bool flexible; };
    std::vector<Item> items;
    int total_height = 0;
    int flexible_count = 0;
    int max_width = 0;
    
    for (auto* c : children) {
        if (!c || !c->visible) continue;
        
        if (c->requested_w == 0 || c->requested_h == 0) {
            c->updateLayout();
        }
        
        Item it;
        it.w = c;
        it.flexible = (c->requested_h < 0);
        it.height = it.flexible ? 0 : c->requested_h;
        total_height += it.height;
        if (it.flexible) flexible_count++;
        if (c->bounds.w > max_width) max_width = c->bounds.w;
        items.push_back(it);
    }
    
    if (items.empty()) {
        if (auto_width)  bounds.w = padding * 2;
        if (auto_height) bounds.h = padding * 2;
        return;
    }
    
    int spacing_total = (int)(items.size() - 1) * spacing;
    int remaining = inner_h - total_height - spacing_total;
    if (remaining < 0) remaining = 0;
    if (auto_height) remaining = 0;
    
    int flex_height = (flexible_count > 0) ? remaining / flexible_count : 0;
    
    int used_height = total_height + (flexible_count * flex_height) + spacing_total;
    int free_space = inner_h - used_height;
    if (free_space < 0) free_space = 0;
    if (auto_height) free_space = 0;
    
    int start_y = inner_y;
    int extra_spacing = 0;
    
    switch (justify_content) {
        case KitsuJustify::START: break;
        case KitsuJustify::CENTER: start_y += free_space / 2; break;
        case KitsuJustify::END:    start_y += free_space; break;
        case KitsuJustify::SPACE_BETWEEN:
            if (items.size() > 1) extra_spacing = free_space / (int)(items.size() - 1);
            break;
        case KitsuJustify::SPACE_AROUND:
            if (!items.empty()) {
                extra_spacing = free_space / (int)items.size();
                start_y += extra_spacing / 2;
            }
            break;
        case KitsuJustify::SPACE_EVENLY:
            if (!items.empty()) {
                extra_spacing = free_space / (int)(items.size() + 1);
                start_y += extra_spacing;
            }
            break;
    }
    
    int cursor_y = start_y;
    int total_w_used = 0;
    
    for (auto& it : items) {
        KitsuWidget* c = it.w;
        
        int h = it.flexible ? flex_height : it.height;
        if (auto_height && h <= 0) {
            h = c->bounds.h;
            if (h <= 0) h = 24;
        }
        
        int w;
        if (c->requested_w <= 0) {
            w = inner_w;
        } else {
            w = c->requested_w;
        }
        
        c->bounds.h = h;
        c->bounds.w = w;
        c->bounds.y = cursor_y;
        
        switch (align) {
            case KitsuAlign::START:   c->bounds.x = inner_x; break;
            case KitsuAlign::CENTER:  c->bounds.x = inner_x + (inner_w - w) / 2; break;
            case KitsuAlign::END:     c->bounds.x = inner_x + inner_w - w; break;
            case KitsuAlign::STRETCH: c->bounds.x = inner_x; c->bounds.w = inner_w; break;
        }
        
        if (c->bounds.w > total_w_used) total_w_used = c->bounds.w;
        
        cursor_y += h + spacing + extra_spacing;
        c->markDirty();
    }
    
    if (auto_height) {
        int final_h = 0;
        int count = 0;
        for (auto& it : items) {
            final_h += it.w->bounds.h;
            count++;
        }
        if (count > 0) final_h += (count - 1) * spacing;
        final_h += padding * 2;
        bounds.h = final_h;
    }
    
    if (auto_width && total_w_used > 0) {
        bounds.w = total_w_used + padding * 2;
    }
}

void KitsuBox::render(SDL_Renderer* renderer) {
    if (!visible) return;
    for (auto* c : children) {
        if (c && c->visible) c->render(renderer);
    }
    clearDirty();
}

bool KitsuBox::handleEvent(const SDL_Event& e) {
    // ===== Click derecho: buscar el hijo más profundo que lo capture =====
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            KitsuWidget* c = *it;
            if (!c || !c->visible || !c->enabled) continue;
            if (c->handleEvent(e)) return true;
        }
        return false;
    }
    
    // ===== Resto de eventos: comportamiento normal =====
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        KitsuWidget* c = *it;
        if (!c || !c->visible || !c->enabled) continue;
        if (c->handleEvent(e)) return true;
    }
    return false;
}

void KitsuBox::tick() {
    for (auto* c : children) {
        if (c && c->visible) c->tick();
    }
}

} // namespace KitsuGui
