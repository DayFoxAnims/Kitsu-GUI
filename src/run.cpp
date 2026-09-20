#include "kitsugui.h"
#include "internal.h"

namespace KitsuGui {

Window* g_window = nullptr;
KitsuRenderer* g_renderer = nullptr;

void run() {
    if (!g_window) {
        g_window = new Window(800, 600, "KitsuGui App", true);
    }
    
    g_renderer = g_window->getKitsuRenderer();
    KitsuRenderer* kr = g_renderer;
    SDL_Renderer* renderer = g_window->getSDLRenderer();
    KitsuBox* root = g_window->getRoot();
    
    Uint32 main_window_id = SDL_GetWindowID(g_window->getSDLWindow());
    
    SDL_Event event;
    
    while (g_window->running) {
        bool had_events = false;
        bool quit = false;
        
        // ===== EVENTOS =====
        while (SDL_PollEvent(&event)) {
            had_events = true;
            
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }
            
            // ===== ¿Es para la ventana principal? =====
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.windowID == main_window_id) {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        int w = event.window.data1;
                        int h = event.window.data2;
                        kr->resize(w, h);
                        g_window->syncRootSize();
                        continue;
                    }
                }
            }
            
            // ===== Comprobar si hay un modal activo =====
            KitsuPopup* active_modal = getActiveModal();
            
            // ===== Determinar si el evento es para la ventana principal =====
            bool is_for_main = true;
            
            if (event.type == SDL_WINDOWEVENT) {
                is_for_main = (event.window.windowID == main_window_id);
            }
            else if (event.type == SDL_MOUSEMOTION ||
                     event.type == SDL_MOUSEBUTTONDOWN ||
                     event.type == SDL_MOUSEBUTTONUP ||
                     event.type == SDL_MOUSEWHEEL) {
                SDL_Window* focused = SDL_GetMouseFocus();
                if (focused) {
                    is_for_main = (SDL_GetWindowID(focused) == main_window_id);
                }
            }
            else if (event.type == SDL_KEYDOWN ||
                     event.type == SDL_KEYUP ||
                     event.type == SDL_TEXTINPUT) {
                SDL_Window* kb_focus = SDL_GetKeyboardFocus();
                if (kb_focus) {
                    is_for_main = (SDL_GetWindowID(kb_focus) == main_window_id);
                }
            }
            
            // =========================================================
            // PRIORIDAD 1: MENÚ CONTEXTUAL (captura global)
            // Mientras un menú esté abierto, tiene prioridad absoluta.
            // Consume TODOS los eventos y el root nunca los ve.
            // =========================================================
            KitsuContextMenu* active_menu = KitsuContextMenu::getActive();
            if (is_for_main && active_menu && active_menu->isOpen()) {
                active_menu->handleEvent(event);
                continue;   // ← NO enviar al root ni a popups
            }
            
            // =========================================================
            // PRIORIDAD 2: ROOT (ventana principal)
            // Solo se procesa si no hay modal activo.
            // =========================================================
            if (is_for_main && !active_modal && root) {
                root->handleEvent(event);
            }
            
            // ===== Enviar a popups =====
            std::vector<KitsuPopup*> popups_copy = g_popups;
            for (auto* popup : popups_copy) {
                if (popup && popup->isOpen()) {
                    popup->handleEvent(event);
                }
            }
        }
        
        if (quit) {
            g_window->running = false;
            break;
        }
        
        // ===== TICK =====
        if (root) root->tick();
        
        for (auto* popup : g_popups) {
            if (popup && popup->isOpen()) {
                popup->tick();
            }
        }
        
        // ===== RENDER VENTANA PRINCIPAL =====
        bool force_render = (g_always_render > 0);
        
        // Si hay un menú contextual abierto, forzar render
        // (para que se dibuje encima de todo correctamente)
        KitsuContextMenu* active_menu = KitsuContextMenu::getActive();
        if (active_menu && active_menu->isOpen()) {
            force_render = true;
        }
        
        if (kr->hasDirty() || force_render) {
            kr->beginFrame();
            
            SDL_SetRenderDrawColor(renderer,
                g_window->bgColor.r,
                g_window->bgColor.g,
                g_window->bgColor.b,
                255);
            SDL_RenderClear(renderer);
            
            // 1. Árbol principal
            if (root) {
                root->updateLayout();
                root->render(renderer);
            }
            
            // 2. Overlays (FPS, etc.)
            for (auto* ov : g_window->overlays) {
                if (ov && ov->visible) ov->render(renderer);
            }
            
            // 3. Menú contextual (encima de todo)
            if (active_menu && active_menu->isOpen()) {
                active_menu->render(renderer);
            }
            
            kr->endFrame();
            
            if (auto* fps = KitsuFPSView::getInstance()) {
                fps->frameRendered();
            }
        }
        
        // ===== RENDER POPUPS =====
        std::vector<KitsuPopup*> popups_to_render = g_popups;
        for (auto* popup : popups_to_render) {
            if (popup && popup->isOpen()) {
                popup->render();
            }
        }
        
        // ===== LIMPIAR POPUPS CERRADOS =====
        for (size_t i = 0; i < g_popups.size(); ) {
            KitsuPopup* popup = g_popups[i];
            if (!popup || !popup->isOpen()) {
                delete popup;
            } else {
                i++;
            }
        }
        
        // ===== GESTIÓN DE INACTIVIDAD =====
        bool has_animations = (g_active_animations > 0);
        bool has_popups = !g_popups.empty();
        bool has_menu = (active_menu && active_menu->isOpen());
        
        if (had_events) {
            SDL_Delay(16);
        } else if (has_animations || force_render || has_popups || has_menu) {
            SDL_WaitEventTimeout(nullptr, 16);
        } else {
            SDL_WaitEvent(nullptr);
        }
    }
    
    // ===== LIMPIAR POPUPS AL SALIR =====
    for (auto* popup : g_popups) {
        delete popup;
    }
    g_popups.clear();
    
    g_renderer = nullptr;
    delete g_window;
    g_window = nullptr;
    SDL_Quit();
}

} // namespace KitsuGui
