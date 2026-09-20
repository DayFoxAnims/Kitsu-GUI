#include "kitsugui.h"
#include "internal.h"
#include <iostream>

namespace KitsuGui {

// ============================================================
// CONSTRUCTOR
// ============================================================
Window::Window(int width, int height, const std::string& title, bool resizable)
    : window(nullptr), renderer(nullptr), kitsu_renderer(nullptr),
      root(nullptr), bgColor(245, 240, 235), running(false) {
    
    // ===== Inicializar SDL =====
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL no pudo inicializarse: " << SDL_GetError() << std::endl;
        return;
    }
    
    // ===== Crear ventana =====
    Uint32 flags = SDL_WINDOW_SHOWN;
    if (resizable) flags |= SDL_WINDOW_RESIZABLE;
    
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, flags
    );
    if (!window) {
        std::cerr << "No se pudo crear ventana: " << SDL_GetError() << std::endl;
        return;
    }
    
    // ===== Crear renderer =====
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "No se pudo crear renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        window = nullptr;
        return;
    }
    
    // ===== Crear KitsuRenderer (canvas retenido) =====
    kitsu_renderer = new KitsuRenderer(renderer, width, height);
    
    // ===== Root: VBox, toda la ventana, sin auto-layout =====
    // Los widgets que añadas con add() se posicionan con setBounds.
    // Si quieres auto-layout, crea un HBox/VBox y añádelo como hijo.
    root = new KitsuBox(false);   // false = vertical
    root->setBounds(0, 0, width, height);
    root->setAutoLayout(false);
    root->setPadding(0);
    root->setSpacing(0);
    
    running = true;
    g_window = this;
}

// ============================================================
// DESTRUCTOR
// ============================================================
Window::~Window() {
    // Limpiar el cache de iconos para este renderer ANTES de destruirlo
    if (renderer) {
        KitsuIconCache::instance().clear();
    }
    
    // Limpiar overlays (no los destruimos, solo limpiamos la lista)
    overlays.clear();
    
    if (root) {
        delete root;
        root = nullptr;
    }
    if (kitsu_renderer) {
        delete kitsu_renderer;
        kitsu_renderer = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    // Limpiar referencias globales
    if (g_window == this) g_window = nullptr;
    if (g_renderer == kitsu_renderer) g_renderer = nullptr;
}

// ============================================================
// FONDO
// ============================================================
void Window::setBackground(const Color& color) {
    bgColor = color;
    if (kitsu_renderer) {
        kitsu_renderer->markAllDirty();
    }
}

// ============================================================
// AÑADIR WIDGETS
// ============================================================
void Window::add(KitsuWidget* widget) {
    if (!widget || !root) return;
    
    // Añadirlo al root (no owns: el usuario gestiona la memoria)
    root->addChild(widget, false);
    
    // Actualizar layout (por si el root tiene auto-layout)
    root->updateLayout();
    
    // Forzar redibujado
    if (kitsu_renderer) {
        kitsu_renderer->markAllDirty();
    }
}

void Window::addOverlay(KitsuWidget* widget) {
    if (!widget) return;
    
    // Los overlays no tienen padre (no participan en el layout)
    widget->parent = nullptr;
    overlays.push_back(widget);
    
    // Forzar redibujado
    if (kitsu_renderer) {
        kitsu_renderer->markAllDirty();
    }
}

// ============================================================
// SINCRONIZAR ROOT CON EL TAMAÑO DE VENTANA
// ============================================================
void Window::syncRootSize() {
    if (!root || !window) return;
    
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    
    root->setBounds(0, 0, w, h);
    root->markDirty();
    root->updateLayout();
    
    if (kitsu_renderer) {
        kitsu_renderer->markAllDirty();
    }
}

} // namespace KitsuGui
