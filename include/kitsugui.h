#ifndef KITSUGUI_H
#define KITSUGUI_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <algorithm>

// ===== SUB-MÓDULOS =====
#include "kitsugui/color.h"
#include "kitsugui/theme.h"
#include "kitsugui/config.h"
#include "kitsugui/renderer.h"
#include "kitsugui/icon.h"
#include "kitsugui/icon_theme.h"
#include "kitsugui/widget.h"
#include "kitsugui/box.h"
#include "kitsugui/shapes.h"
#include "kitsugui/button.h"
#include "kitsugui/label.h"
#include "kitsugui/panel.h"
#include "kitsugui/fps.h"
#include "kitsugui/check_box.h"
#include "kitsugui/switch.h"
#include "kitsugui/slider.h"
#include "kitsugui/progress_bar.h"
#include "kitsugui/radio.h"
#include "kitsugui/text_input.h"
#include "kitsugui/scroll_view.h"
#include "kitsugui/shapes.h"
#include "kitsugui/viewport.h"
#include "kitsugui/popup.h"
#include "kitsugui/context_menu.h"
#include "kitsugui/dropdown.h"


namespace KitsuGui {

// ===== VENTANA =====
// ===== VENTANA =====
class Window {
public:
    Window(int width, int height, const std::string& title, bool resizable = true);
    ~Window();
    
    void setBackground(const Color& color);
    void setBackground(int r, int g, int b) { setBackground(Color(r, g, b)); }
    
    // Añadir widget al root (participa en el layout)
    void add(KitsuWidget* widget);
    
    // Añadir overlay (se dibuja encima, NO participa en el layout)
    void addOverlay(KitsuWidget* widget);
    
    // El root box (contenedor principal)
    KitsuBox* getRoot() const { return root; }
    
    // Sincronizar el root con el tamaño de la ventana
    void syncRootSize();
    
    // Acceso interno
    SDL_Window*    getSDLWindow()   const { return window; }
    SDL_Renderer*  getSDLRenderer() const { return renderer; }
    KitsuRenderer* getKitsuRenderer() const { return kitsu_renderer; }
    
    // Overlays (para run.cpp)
    std::vector<KitsuWidget*> overlays;
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    KitsuRenderer* kitsu_renderer;
    KitsuBox* root;
    Color bgColor;
    bool running;
    
    friend void run();
};

// ===== FUNCIONES GLOBALES =====
void run();

} // namespace KitsuGui

#endif
