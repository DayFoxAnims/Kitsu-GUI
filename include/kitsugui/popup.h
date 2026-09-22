#ifndef KITSUGUI_POPUP_H
#define KITSUGUI_POPUP_H

#include "kitsugui/box.h"
#include "kitsugui/renderer.h"
#include "kitsugui/color.h"
#include <string>
#include <vector>
#include <functional>

namespace KitsuGui {

enum class PopupType {
    MESSAGE,
    CONFIRM,
    INPUT,
    WARNING,
    ERROR,
    INFO,
    SUCCESS,
    CUSTOM
};

// ============================================================
// KitsuPopup
// ============================================================
// Uso simple (helpers):
//   Popup::message("Hola", "Esto es un mensaje");
//   Popup::confirm("¿?", "¿Continuar?", [](bool ok) { ... });
//
// Uso custom:
//   auto* p = new KitsuPopup(400, 200, "Título");
//   p->modal(true)->type(PopupType::MESSAGE);
//   p->add(new KitsuLabel("Contenido"));
//   p->addButton("OK", [p]() { p->close(); });
//
// Los métodos fluent devuelven KitsuPopup* (puntero a this),
// para poder encadenar siempre con '->'.
// ============================================================
class KitsuPopup {
public:
    KitsuPopup(int width, int height, const std::string& title);
    ~KitsuPopup();

    // ===== Configuración fluent =====
    KitsuPopup* type(PopupType t);
    KitsuPopup* message(const std::string& msg);
    KitsuPopup* modal(bool m);
    KitsuPopup* resizable(bool r);
    KitsuPopup* closeOnEscape(bool enabled);

    // ===== Widgets =====
    KitsuPopup* add(KitsuWidget* widget);
    KitsuPopup* addButton(const std::string& text, std::function<void()> cb);
    KitsuPopup* addLabel(const std::string& text);

    // ===== Ciclo de vida =====
    KitsuPopup* show();
    KitsuPopup* hide();
    KitsuPopup* close();

    // ===== Callbacks =====
    KitsuPopup* onClose(std::function<void()> cb);
    KitsuPopup* onResult(std::function<void(int)> cb);

    // ===== Estado (no fluent) =====
    bool isOpen() const { return open_; }
    bool isModal() const { return modal_; }

    // ===== Acceso (no fluent) =====
    SDL_Window*    sdlWindow()   const { return window_; }
    Uint32         windowId()    const { return window_id_; }
    KitsuBox*      root()        const { return root_; }

    // ===== Render y eventos (no fluent) =====
    void render();
    void handleEvent(const SDL_Event& e);
    void tick();
    void updateLayout();

private:
    SDL_Window*    window_    = nullptr;
    SDL_Renderer*  renderer_  = nullptr;
    KitsuRenderer* krenderer_ = nullptr;
    KitsuBox*      root_      = nullptr;

    Uint32 window_id_ = 0;
    bool open_ = false;
    bool modal_ = false;
    bool close_on_escape_ = true;
    PopupType type_ = PopupType::MESSAGE;

    int width_ = 0;
    int height_ = 0;

    std::vector<KitsuWidget*> owned_widgets_;
    std::function<void()> on_close_;
    std::function<void(int)> on_result_;
};

// ============================================================
// Helpers
// ============================================================
namespace Popup {
    KitsuPopup* message(const std::string& title, const std::string& msg);
    KitsuPopup* confirm(const std::string& title, const std::string& msg,
                       std::function<void(bool)> cb);
    KitsuPopup* input(const std::string& title, const std::string& msg,
                     std::function<void(const std::string&)> cb);
    KitsuPopup* warning(const std::string& title, const std::string& msg);
    KitsuPopup* error(const std::string& title, const std::string& msg);
    KitsuPopup* info(const std::string& title, const std::string& msg);
    KitsuPopup* success(const std::string& title, const std::string& msg);
}

// Lista global de popups activos
extern std::vector<KitsuPopup*> g_popups;

// Primer modal activo (o nullptr)
KitsuPopup* activeModal();

} // namespace KitsuGui

#endif
