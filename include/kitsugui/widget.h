#ifndef KITSUGUI_WIDGET_H
#define KITSUGUI_WIDGET_H

#include <SDL2/SDL.h>
#include <functional>

namespace KitsuGui {

// ============================================================
// Enums de layout
// ============================================================
enum class KitsuAlign {
    START, CENTER, END, STRETCH
};

enum class KitsuJustify {
    START, CENTER, END,
    SPACE_BETWEEN, SPACE_AROUND, SPACE_EVENLY
};

// ============================================================
// KitsuTransform — offsets visuales (scroll, parallax, etc.)
// ============================================================
// No cambia la posición lógica (bounds). Solo afecta al dibujado
// y al hit-test a través de visualRect().
// ============================================================
struct KitsuTransform {
    int offset_x = 0;
    int offset_y = 0;

    void reset() { offset_x = 0; offset_y = 0; }
};

// ============================================================
// KitsuWidget — base de todo widget
// ============================================================
class KitsuWidget {
public:
    // ===== Jerarquía =====
    KitsuWidget* parent = nullptr;

    // ===== Geometría =====
    // bounds: posición y tamaño lógicos (los que ve el layout)
    // desired_w/h: lo que el widget PIDE. 0 = auto, <0 = flex
    SDL_Rect bounds = {0, 0, 0, 0};
    int desired_w = 0;
    int desired_h = 0;

    // ===== Transform visual =====
    KitsuTransform transform;

    // ===== Apariencia =====
    int margin_  = 0;
    int padding_ = 0;
    bool visible = true;
    bool enabled = true;

    // ===== Evento de click derecho =====
    // Recibe coordenadas globales.
    std::function<void(int, int)> onContextMenu;

    virtual ~KitsuWidget() {}

    // ===== Ciclo de vida =====
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void updateLayout() {}
    virtual bool handleEvent(const SDL_Event& e) { return false; }
    virtual void tick() {}

    // ===== Geometría =====
    // Coloca el widget. Sincroniza desired_w/h con w/h.
    void place(int x, int y, int w, int h) {
		bounds.x = x;
		bounds.y = y;
		bounds.w = w;
		bounds.h = h;
		// Solo sobreescribir desired si era 0 (auto por contenido).
		// NO tocar -1 (flexible) ni >0 (fijo).
		if (desired_w == 0) desired_w = w;
		if (desired_h == 0) desired_h = h;
		invalidate();
	}

    // ===== Invalidación =====
    virtual void invalidate();
    bool needsRender() const { return dirty_; }
    void clearNeedsRender() { dirty_ = false; }

    // ===== Rectángulos =====
    // globalRect(): posición lógica acumulada (sin transform).
    //   Se usa para hit-test y para el layout.
    // visualRect(): posición visual acumulada (con transform).
    //   Se usa para dibujar.
    SDL_Rect globalRect() const;
    SDL_Rect visualRect() const;

    bool contains(int x, int y) const;

    // ===== Menú contextual =====
    // Si el evento es un click derecho dentro del widget y hay
    // callback, lo dispara y devuelve true.
    bool tryContextMenu(const SDL_Event& e);

protected:
    bool dirty_ = true;
};

// ===== Contadores globales (usados por run.cpp) =====
extern int g_active_animations;
extern int g_always_render;

} // namespace KitsuGui

#endif
