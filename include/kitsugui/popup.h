#ifndef KITSUGUI_POPUP_H
#define KITSUGUI_POPUP_H

#include "kitsugui/box.h"
#include "kitsugui/renderer.h"
#include "kitsugui/color.h"
#include <string>
#include <vector>
#include <functional>

namespace KitsuGui {

// ===== Tipos de popup =====
enum class PopupType {
    MESSAGE,      // Solo mensaje + OK
    CONFIRM,      // Mensaje + Sí/No
    INPUT,        // Mensaje + campo texto + OK/Cancel
    WARNING,      // Icono advertencia + mensaje + OK
    ERROR,        // Icono error + mensaje + OK
    INFO,         // Icono info + mensaje + OK
    SUCCESS,      // Icono éxito + mensaje + OK
    CUSTOM        // El usuario añade sus widgets
};

class KitsuPopup {
public:
    KitsuPopup(int width, int height, const std::string& title);
    ~KitsuPopup();
    
    // ===== Configuración fluida =====
    KitsuPopup& withType(PopupType t);
    KitsuPopup& withMessage(const std::string& msg);
    KitsuPopup& withModal(bool modal);
    KitsuPopup& withResizable(bool resizable);
    KitsuPopup& withCloseOnEscape(bool enabled);
    
    // ===== Widgets =====
    void add(KitsuWidget* widget);
    void addButton(const std::string& text, std::function<void()> cb);
    void addLabel(const std::string& text);
    
    // ===== Ciclo de vida =====
    void show();
    void hide();
    void close();
    bool isOpen() const { return open; }
    
    // ===== Render y eventos =====
    void render();
    void handleEvent(const SDL_Event& e);
    void tick();
    void updateLayout();
    void markAllDirty();
    
    // ===== Callbacks =====
    void setOnClose(std::function<void()> cb) { on_close = cb; }
    void setOnResult(std::function<void(int)> cb) { on_result = cb; }
    
    // ===== Estado =====
    bool isModal() const { return modal; }
    
    // ===== Acceso =====
    SDL_Window* getSDLWindow() const { return window; }
    Uint32 getWindowID() const { return window_id; }
    KitsuBox* getRoot() const { return root; }
    
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    KitsuRenderer* kitsu_renderer = nullptr;
    KitsuBox* root = nullptr;
    
    Uint32 window_id = 0;
    bool open = false;
    bool modal = false;
    bool close_on_escape = true;
    PopupType type = PopupType::MESSAGE;
    
    int width = 0;
    int height = 0;
    
    std::vector<KitsuWidget*> owned_widgets;
    std::function<void()> on_close;
    std::function<void(int)> on_result;
};

// ============================================================
// Helpers para popups comunes
// ============================================================
namespace Popup {
    // Básicos
    KitsuPopup* message(const std::string& title, const std::string& msg);
    
    KitsuPopup* confirm(const std::string& title, const std::string& msg,
                       std::function<void(bool)> cb);
    
    KitsuPopup* input(const std::string& title, const std::string& msg,
                     std::function<void(const std::string&)> cb);
    
    // Con icono del tema del sistema
    KitsuPopup* warning(const std::string& title, const std::string& msg);
    KitsuPopup* error(const std::string& title, const std::string& msg);
    KitsuPopup* info(const std::string& title, const std::string& msg);
    KitsuPopup* success(const std::string& title, const std::string& msg);
}

// ===== Lista global de popups activos =====
extern std::vector<KitsuPopup*> g_popups;

// ===== Devuelve el primer modal activo (o nullptr) =====
KitsuPopup* getActiveModal();

} // namespace KitsuGui

#endif
