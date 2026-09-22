#include "kitsugui.h"
#include "internal.h"

namespace KitsuGui {

KitsuWindow*   g_window   = nullptr;
KitsuRenderer* g_renderer = nullptr;

// ============================================================
// run() — bucle principal
// ============================================================
// Modelo: render on-demand completo.
//   - Cuando hay eventos o animaciones: redibuja toda la ventana.
//   - Cuando no pasa nada: espera bloqueante (0% CPU).
//
// Prioridad de eventos:
//   1. Menú contextual activo  → captura global, root NO ve nada.
//   2. Modal activo            → root NO ve nada.
//   3. Root                    → procesa normalmente.
//   4. Popups                  → cada uno filtra por window id.
// ============================================================
void run() {
    if (!g_window) {
        g_window = new KitsuWindow(800, 600, "KitsuGui App");
    }

    g_renderer = g_window->renderer();
    if (!g_renderer) return;

    KitsuRenderer* kr = g_renderer;
    SDL_Renderer* renderer = g_window->sdlRenderer();
    KitsuBox* root = g_window->root();
    Uint32 main_id = SDL_GetWindowID(g_window->sdlWindow());

    SDL_Event e;

    while (g_window->running_) {
        bool had_events = false;
        bool quit = false;

        // ====================================================
        // 1. Procesar eventos
        // ====================================================
        while (SDL_PollEvent(&e)) {
            had_events = true;

            if (e.type == SDL_QUIT) {
                quit = true;
                break;
            }

            // Resize de la ventana principal
            if (e.type == SDL_WINDOWEVENT &&
                e.window.windowID == main_id &&
                e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w = e.window.data1;
                int h = e.window.data2;
                kr->resize(w, h);
                g_window->syncRootSize();
                continue;
            }

            // ¿Es para la ventana principal?
            bool is_for_main = true;
            if (e.type == SDL_WINDOWEVENT) {
                is_for_main = (e.window.windowID == main_id);
            }
            else if (e.type == SDL_MOUSEMOTION ||
                     e.type == SDL_MOUSEBUTTONDOWN ||
                     e.type == SDL_MOUSEBUTTONUP ||
                     e.type == SDL_MOUSEWHEEL) {
                SDL_Window* f = SDL_GetMouseFocus();
                if (f) is_for_main = (SDL_GetWindowID(f) == main_id);
            }
            else if (e.type == SDL_KEYDOWN ||
                     e.type == SDL_KEYUP ||
                     e.type == SDL_TEXTINPUT) {
                SDL_Window* f = SDL_GetKeyboardFocus();
                if (f) is_for_main = (SDL_GetWindowID(f) == main_id);
            }

            // ====================================================
            // PRIORIDAD 1: Menú contextual activo
            // ====================================================
            KitsuContextMenu* menu = KitsuContextMenu::active();
            if (is_for_main && menu && menu->isOpen()) {
                menu->handleEvent(e);
                continue;
            }

            // ====================================================
            // PRIORIDAD 2: Root (solo si no hay modal)
            // ====================================================
            KitsuPopup* modal = activeModal();
            if (is_for_main && !modal && root) {
                root->handleEvent(e);
            }

            // ====================================================
            // PRIORIDAD 3: Popups
            // ====================================================
            auto popups_copy = g_popups;
            for (auto* p : popups_copy) {
                if (p && p->isOpen()) p->handleEvent(e);
            }
        }

        if (quit) {
            g_window->running_ = false;
            break;
        }

        // ====================================================
        // 2. Tick
        // ====================================================
        if (root) root->tick();
        for (auto* p : g_popups) {
            if (p && p->isOpen()) p->tick();
        }

        // ====================================================
        // 3. Render ventana principal
        // ====================================================
        bool force = (g_always_render > 0);

        KitsuContextMenu* menu = KitsuContextMenu::active();
        if (menu && menu->isOpen()) force = true;

        if (kr->needsRender() || force) {
            kr->begin();

            SDL_SetRenderDrawColor(renderer,
                g_window->bg_color_.r,
                g_window->bg_color_.g,
                g_window->bg_color_.b,
                255);
            SDL_RenderClear(renderer);

            // Root
            if (root) {
                root->updateLayout();
                root->render(renderer);
            }

            // Overlays
            for (auto* ov : g_window->overlays_) {
                if (ov && ov->visible) ov->render(renderer);
            }

            // Menú contextual (encima de todo)
            if (menu && menu->isOpen()) {
                menu->render(renderer);
            }

            kr->end();

            if (auto* fps = KitsuFPSView::instance()) {
                fps->frameRendered();
            }
        }

        // ====================================================
        // 4. Render popups
        // ====================================================
        auto popups_to_render = g_popups;
        for (auto* p : popups_to_render) {
            if (p && p->isOpen()) p->render();
        }

        // ====================================================
        // 5. Limpiar popups cerrados
        // ====================================================
        for (size_t i = 0; i < g_popups.size(); ) {
            KitsuPopup* p = g_popups[i];
            if (!p || !p->isOpen()) {
                delete p;
            } else {
                i++;
            }
        }

        // ====================================================
        // 6. Gestión de inactividad
        // ====================================================
        bool has_anim = (g_active_animations > 0);
        bool has_popups = !g_popups.empty();
        bool has_menu = (menu && menu->isOpen());
        bool pending = kr->needsRender() || force;

        if (had_events) {
            SDL_Delay(16);
        } else if (has_anim || has_popups || has_menu || pending) {
            SDL_WaitEventTimeout(nullptr, 16);
        } else {
            // 0% CPU cuando no hay nada
            SDL_WaitEvent(nullptr);
        }
    }

    // Limpiar popups restantes
    for (auto* p : g_popups) delete p;
    g_popups.clear();

    g_renderer = nullptr;
    delete g_window;
    g_window = nullptr;
    SDL_Quit();
}

} // namespace KitsuGui
