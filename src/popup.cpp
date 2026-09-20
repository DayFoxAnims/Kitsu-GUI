#include "kitsugui/popup.h"
#include "kitsugui/button.h"
#include "kitsugui/label.h"
#include "kitsugui/text_input.h"
#include "kitsugui/icon.h"
#include "internal.h"

namespace KitsuGui {

std::vector<KitsuPopup*> g_popups;

// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================
KitsuPopup::KitsuPopup(int width, int height, const std::string& title)
    : width(width), height(height) {
    
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        SDL_Log("KitsuPopup: error creando ventana: %s", SDL_GetError());
        return;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("KitsuPopup: error creando renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        return;
    }
    
    kitsu_renderer = new KitsuRenderer(renderer, width, height);
    
    root = new KitsuBox(false);   // vertical
    root->setBounds(0, 0, width, height);
    root->setAutoLayout(true);
    root->setAlignment(KitsuAlign::CENTER);
    root->setJustify(KitsuJustify::CENTER);
    root->setSpacing(15);
    root->setPadding(20);
    
    window_id = SDL_GetWindowID(window);
    open = true;
    
    g_popups.push_back(this);
}

KitsuPopup::~KitsuPopup() {
    for (size_t i = 0; i < g_popups.size(); i++) {
        if (g_popups[i] == this) {
            g_popups.erase(g_popups.begin() + i);
            break;
        }
    }
    
    for (auto* w : owned_widgets) delete w;
    owned_widgets.clear();
    if (root) delete root;
    if (kitsu_renderer) delete kitsu_renderer;
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
}

// ============================================================
// CONFIGURACIÓN
// ============================================================
KitsuPopup& KitsuPopup::withType(PopupType t) {
    type = t;
    return *this;
}

KitsuPopup& KitsuPopup::withMessage(const std::string& msg) {
    auto* label = new KitsuLabel(msg);
    label->setBounds(0, 0, width - 40, 60);
    label->withWrap(width - 40);
    label->withAlign(TextAlign::CENTER);
    owned_widgets.push_back(label);
    root->addChild(label, false);
    return *this;
}

KitsuPopup& KitsuPopup::withModal(bool m) {
    modal = m;
    
    if (modal && window && g_window) {
        SDL_Window* parent = g_window->getSDLWindow();
        if (parent) {
            SDL_SetWindowModalFor(window, parent);
        }
    }
    
    return *this;
}

KitsuPopup& KitsuPopup::withResizable(bool resizable) {
    if (window) {
        SDL_SetWindowResizable(window, resizable ? SDL_TRUE : SDL_FALSE);
    }
    return *this;
}

KitsuPopup& KitsuPopup::withCloseOnEscape(bool enabled) {
    close_on_escape = enabled;
    return *this;
}

// ============================================================
// WIDGETS
// ============================================================
void KitsuPopup::add(KitsuWidget* widget) {
    if (!widget) return;
    root->addChild(widget, false);
    root->updateLayout();
}

void KitsuPopup::addButton(const std::string& text, std::function<void()> cb) {
    auto* btn = new KitsuButton(text);
    btn->setBounds(0, 0, 120, 40);
    btn->withCallback(cb);
    owned_widgets.push_back(btn);
    root->addChild(btn, false);
}

void KitsuPopup::addLabel(const std::string& text) {
    auto* lbl = new KitsuLabel(text);
    lbl->setBounds(0, 0, width - 40, 30);
    lbl->withAlign(TextAlign::CENTER);
    owned_widgets.push_back(lbl);
    root->addChild(lbl, false);
}

// ============================================================
// CICLO DE VIDA
// ============================================================
void KitsuPopup::show() {
    if (window) {
        SDL_ShowWindow(window);
        SDL_RaiseWindow(window);
        open = true;
        markAllDirty();
    }
}

void KitsuPopup::hide() {
    if (window) {
        SDL_HideWindow(window);
    }
}

void KitsuPopup::close() {
    open = false;
    if (on_close) on_close();
}

// ============================================================
// RENDER
// ============================================================
void KitsuPopup::render() {
    if (!window || !renderer || !kitsu_renderer) return;
    if (!open) return;
    
    root->setBounds(0, 0, width, height);
    root->updateLayout();
    
    kitsu_renderer->beginFrame();
    
    KitsuTheme& t = KitsuTheme::current();
    SDL_SetRenderDrawColor(renderer,
        (Uint8)t.bg_primary.r, (Uint8)t.bg_primary.g, (Uint8)t.bg_primary.b, 255);
    SDL_RenderClear(renderer);
    
    root->render(renderer);
    
    kitsu_renderer->endFrame();
}

// ============================================================
// EVENTOS
// ============================================================
void KitsuPopup::handleEvent(const SDL_Event& e) {
    if (!open || !window) return;
    
    // Window events (cerrar, resize)
    if (e.type == SDL_WINDOWEVENT) {
        if (e.window.windowID != window_id) return;
        
        if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
            close();
            return;
        }
        
        if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
            int w = e.window.data1;
            int h = e.window.data2;
            width = w;
            height = h;
            if (kitsu_renderer) kitsu_renderer->resize(w, h);
            root->setBounds(0, 0, w, h);
            root->updateLayout();
        }
        return;
    }
    
    // Mouse: solo si esta ventana tiene el foco del ratón
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP ||
        e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEWHEEL) {
        SDL_Window* focused = SDL_GetMouseFocus();
        if (!focused) return;
        if (SDL_GetWindowID(focused) != window_id) return;
    }
    
    // Teclado: solo si esta ventana tiene el foco del teclado
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_TEXTINPUT) {
        SDL_Window* kb_focus = SDL_GetKeyboardFocus();
        if (!kb_focus) return;
        if (SDL_GetWindowID(kb_focus) != window_id) return;
    }
    
    // ESC para cerrar
    if (e.type == SDL_KEYDOWN && close_on_escape) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            close();
            return;
        }
    }
    
    root->handleEvent(e);
}

void KitsuPopup::tick() {
    if (!open) return;
    root->tick();
}

void KitsuPopup::updateLayout() {
    if (!root) return;
    root->setBounds(0, 0, width, height);
    root->updateLayout();
}

void KitsuPopup::markAllDirty() {
    if (kitsu_renderer) kitsu_renderer->markAllDirty();
}

// ============================================================
// HELPERS PARA POPUPS COMUNES
// ============================================================
namespace Popup {

// ============================================================
// BÁSICOS
// ============================================================
KitsuPopup* message(const std::string& title, const std::string& msg) {
    auto* popup = new KitsuPopup(400, 200, title);
    popup->withType(PopupType::MESSAGE);
    popup->withModal(true);
    popup->withMessage(msg);
    popup->addButton("OK", [popup]() {
        popup->close();
    });
    popup->updateLayout();
    return popup;
}

KitsuPopup* confirm(const std::string& title, const std::string& msg,
                   std::function<void(bool)> cb) {
    auto* popup = new KitsuPopup(400, 220, title);
    popup->withType(PopupType::CONFIRM);
    popup->withModal(true);
    popup->withMessage(msg);
    
    // HBox para los botones (lado a lado)
    auto* buttons = new KitsuHBox();
    buttons->setBounds(0, 0, 300, 45);
    buttons->setSpacing(10);
    buttons->setPadding(0);
    buttons->setAlignment(KitsuAlign::CENTER);
    buttons->setJustify(KitsuJustify::CENTER);
    buttons->setAutoLayout(true);
    
    auto* btn_yes = new KitsuButton("Sí");
    btn_yes->setBounds(0, 0, 120, 40);
    btn_yes->withCallback([popup, cb]() {
        if (cb) cb(true);
        popup->close();
    });
    buttons->addChild(btn_yes, true);
    
    auto* btn_no = new KitsuButton("No");
    btn_no->setBounds(0, 0, 120, 40);
    btn_no->withCallback([popup, cb]() {
        if (cb) cb(false);
        popup->close();
    });
    buttons->addChild(btn_no, true);
    
    popup->add(buttons);
    popup->updateLayout();
    return popup;
}

KitsuPopup* input(const std::string& title, const std::string& msg,
                 std::function<void(const std::string&)> cb) {
    auto* popup = new KitsuPopup(420, 260, title);
    popup->withType(PopupType::INPUT);
    popup->withModal(true);
    popup->withMessage(msg);
    
    auto* ti = new KitsuTextInput("");
    ti->setBounds(0, 0, 380, 32);
    ti->withPlaceholder("Escribe aquí...");
    popup->add(ti);
    
    // Botones lado a lado
    auto* buttons = new KitsuHBox();
    buttons->setBounds(0, 0, 300, 45);
    buttons->setSpacing(10);
    buttons->setPadding(0);
    buttons->setAlignment(KitsuAlign::CENTER);
    buttons->setJustify(KitsuJustify::CENTER);
    buttons->setAutoLayout(true);
    
    auto* btn_ok = new KitsuButton("OK");
    btn_ok->setBounds(0, 0, 120, 40);
    btn_ok->withCallback([popup, ti, cb]() {
        if (cb) cb(ti->getText());
        popup->close();
    });
    buttons->addChild(btn_ok, true);
    
    auto* btn_cancel = new KitsuButton("Cancelar");
    btn_cancel->setBounds(0, 0, 120, 40);
    btn_cancel->withCallback([popup]() {
        popup->close();
    });
    buttons->addChild(btn_cancel, true);
    
    popup->add(buttons);
    popup->updateLayout();
    return popup;
}

// ============================================================
// POPUPS CON ICONO
// ============================================================

// Helper interno: crea un popup con icono + mensaje + botón OK
static KitsuPopup* createIconPopup(const std::string& title,
                                   const std::string& msg,
                                   const std::string& icon_name) {
    auto* popup = new KitsuPopup(460, 220, title);
    popup->withType(PopupType::MESSAGE);
    popup->withModal(true);
    
    // ===== Fila superior: icono + mensaje =====
    auto* row = new KitsuHBox();
    row->setBounds(0, 0, 420, 100);
    row->setSpacing(15);
    row->setPadding(0);
    row->setAlignment(KitsuAlign::CENTER);   // centrado vertical
    row->setJustify(KitsuJustify::START);
    row->setAutoLayout(true);
    
    // Icono del sistema (tamaño fijo 48x48)
    auto* icon = new KitsuIconView(icon_name, 48, IconSource::SYSTEM_THEME);
    row->addChild(icon, true);
    
    // Label con el mensaje
    int label_width = 420 - 48 - 15;   // total - icono - spacing
    auto* label = new KitsuLabel(msg);
    label->setBounds(0, 0, label_width, 100);
    label->withWrap(label_width);
    label->withAlign(TextAlign::LEFT);
    label->withVAlign(TextVAlign::MIDDLE);
    row->addChild(label, true);
    
    popup->add(row);
    
    // ===== Botón OK abajo =====
    popup->addButton("OK", [popup]() {
        popup->close();
    });
    
    popup->updateLayout();
    return popup;
}

KitsuPopup* warning(const std::string& title, const std::string& msg) {
    return createIconPopup(title, msg, "dialog-warning");
}

KitsuPopup* error(const std::string& title, const std::string& msg) {
    return createIconPopup(title, msg, "dialog-error");
}

KitsuPopup* info(const std::string& title, const std::string& msg) {
    return createIconPopup(title, msg, "dialog-information");
}

KitsuPopup* success(const std::string& title, const std::string& msg) {
    return createIconPopup(title, msg, "dialog-information");
}

} // namespace Popup

// ============================================================
// MODAL ACTIVO
// ============================================================
KitsuPopup* getActiveModal() {
    for (auto* p : g_popups) {
        if (p && p->isOpen() && p->isModal()) {
            return p;
        }
    }
    return nullptr;
}

} // namespace KitsuGui
