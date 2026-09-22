#include "kitsugui/window.h"
#include "kitsugui.h"
#include "kitsugui/box.h"
#include "kitsugui/fonts.h"
#include "kitsugui/config.h"
#include "kitsugui/icon_theme.h"
#include "kitsugui/theme.h"
#include "internal.h"
#include <iostream>

namespace KitsuGui {

// ============================================================
// Constructor — hace TODA la magia
// ============================================================
KitsuWindow::KitsuWindow(int width, int height,
                        const std::string& title,
                        bool resizable) {

    // ===== 1. SDL + TTF =====
    if (!initSDL()) return;

    // ===== 2. Fuentes globales =====
    KitsuFonts::init();

    // ===== 3. Config + tema + iconos =====
    // Solo se aplican si no se ha hecho antes. El usuario
    // puede seguir llamando a config.load() si quiere.
    auto& cfg = KitsuConfig::instance();
    if (!cfg.configDir().empty()) {
        cfg.load();
        cfg.applyTheme();
        cfg.applyIconTheme();
    }

    // ===== 4. Ventana + renderer =====
    if (!initWindow(title, width, height, resizable)) return;
    if (!initRenderer()) return;

    // ===== 5. Root =====
    root_ = new KitsuBox(false);   // vertical
    root_->place(0, 0, width, height);
    root_->autoLayout(false);      // posicionamiento absoluto
    root_->padding(0)->spacing(0);

    // Fondo por defecto = bg_primary del tema
    bg_color_ = KitsuTheme::active().bg_primary;

    running_ = true;
    g_window = this;
}

// ============================================================
// Inicialización por partes
// ============================================================
bool KitsuWindow::initSDL() {
    if (SDL_WasInit(SDL_INIT_VIDEO)) return true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init falló: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool KitsuWindow::initWindow(const std::string& title,
                             int w, int h, bool resizable) {
    Uint32 flags = SDL_WINDOW_SHOWN;
    if (resizable) flags |= SDL_WINDOW_RESIZABLE;

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h, flags
    );

    if (!window_) {
        std::cerr << "SDL_CreateWindow falló: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool KitsuWindow::initRenderer() {
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer falló: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    int w, h;
    SDL_GetWindowSize(window_, &w, &h);
    krenderer_ = new KitsuRenderer(renderer_, w, h);

    return true;
}

// ============================================================
// Destructor
// ============================================================
KitsuWindow::~KitsuWindow() {
    // Limpiar cache de iconos ANTES de destruir el renderer
    if (renderer_) {
        KitsuIconCache::instance().clear();
    }

    // Limpiar overlays propios
    for (auto* ov : owned_overlays_) delete ov;
    owned_overlays_.clear();
    overlays_.clear();

    if (root_) {
        delete root_;
        root_ = nullptr;
    }
    if (krenderer_) {
        delete krenderer_;
        krenderer_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    // Cerrar fuentes
    KitsuFonts::shutdown();

    if (g_window == this) g_window = nullptr;
    if (g_renderer == krenderer_) g_renderer = nullptr;
}

// ============================================================
// Configuración
// ============================================================
KitsuWindow& KitsuWindow::background(const Color& c) {
    bg_color_ = c;
    if (krenderer_) krenderer_->invalidate();
    return *this;
}

KitsuWindow& KitsuWindow::background(int r, int g, int b) {
    return background(Color(r, g, b));
}

// ============================================================
// Añadir widgets
// ============================================================
KitsuWindow* KitsuWindow::add(KitsuWidget* widget, bool owns) {
    if (!widget || !root_) return this;

    int win_w, win_h;
    SDL_GetWindowSize(window_, &win_w, &win_h);

    // Solo damos tamaño inicial a widgets flexibles (-1).
    // NO tocamos desired_w/h.
    if (widget->desired_w < 0) widget->bounds.w = win_w;
    if (widget->desired_h < 0) widget->bounds.h = win_h;

    root_->add(widget, owns);
    root_->updateLayout();

    if (krenderer_) krenderer_->invalidate();
    return this;
}

KitsuWindow* KitsuWindow::addOverlay(KitsuWidget* widget, bool owns) {
    if (!widget) return this;

    widget->parent = nullptr;   // no participa en el layout
    overlays_.push_back(widget);
    if (owns) owned_overlays_.push_back(widget);

    if (krenderer_) krenderer_->invalidate();
    return this;
}

// ============================================================
// Sincronizar root con el tamaño de la ventana
// ============================================================
void KitsuWindow::syncRootSize() {
    if (!root_ || !window_) return;

    int w, h;
    SDL_GetWindowSize(window_, &w, &h);

    // Cambiamos SOLO bounds, sin tocar desired_w/h del root.
    // Así el root sigue siendo "flexible" si lo era.
    root_->bounds = {0, 0, w, h};

    root_->updateLayout();
    if (krenderer_) krenderer_->invalidate();
}

} // namespace KitsuGui
