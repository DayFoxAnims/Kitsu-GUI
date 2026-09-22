#include "kitsugui/popup.h"
#include "kitsugui/button.h"
#include "kitsugui/label.h"
#include "kitsugui/text_input.h"
#include "kitsugui/icon.h"
#include "kitsugui/theme.h"
#include "internal.h"

namespace KitsuGui {

std::vector<KitsuPopup*> g_popups;

// ============================================================
// Constructor / destructor
// ============================================================
KitsuPopup::KitsuPopup(int width, int height, const std::string& title)
    : width_(width), height_(height) {

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    if (!window_) {
        SDL_Log("KitsuPopup: error creando ventana: %s", SDL_GetError());
        open_ = false;
        return;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        SDL_Log("KitsuPopup: error creando renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        open_ = false;
        return;
    }

    krenderer_ = new KitsuRenderer(renderer_, width, height);

    root_ = new KitsuBox(false);
    root_->place(0, 0, width, height);
    root_->autoLayout(true);
    root_->align(KitsuAlign::CENTER);
    root_->justify(KitsuJustify::CENTER);
    root_->spacing(15);
    root_->padding(20);

    window_id_ = SDL_GetWindowID(window_);
    open_ = true;

    g_popups.push_back(this);
}

KitsuPopup::~KitsuPopup() {
    for (size_t i = 0; i < g_popups.size(); i++) {
        if (g_popups[i] == this) {
            g_popups.erase(g_popups.begin() + i);
            break;
        }
    }

    for (auto* w : owned_widgets_) delete w;
    owned_widgets_.clear();
    if (root_)      delete root_;
    if (krenderer_) delete krenderer_;
    if (renderer_)  SDL_DestroyRenderer(renderer_);
    if (window_)    SDL_DestroyWindow(window_);
}

// ============================================================
// Configuración fluent
// ============================================================
KitsuPopup* KitsuPopup::type(PopupType t) {
    type_ = t;
    return this;
}

KitsuPopup* KitsuPopup::message(const std::string& msg) {
    auto* label = new KitsuLabel(msg);
    label->wrap(width_ - 40);
    label->align(TextAlign::CENTER);
    owned_widgets_.push_back(label);
    if (root_) root_->add(label, false);
    return this;
}

KitsuPopup* KitsuPopup::modal(bool m) {
    modal_ = m;
    if (modal_ && window_ && g_window) {
        SDL_Window* parent = g_window->sdlWindow();   // ← fix
        if (parent) SDL_SetWindowModalFor(window_, parent);
    }
    return this;
}

KitsuPopup* KitsuPopup::resizable(bool r) {
    if (window_) {
        SDL_SetWindowResizable(window_, r ? SDL_TRUE : SDL_FALSE);
    }
    return this;
}

KitsuPopup* KitsuPopup::closeOnEscape(bool enabled) {
    close_on_escape_ = enabled;
    return this;
}

// ============================================================
// Widgets
// ============================================================
KitsuPopup* KitsuPopup::add(KitsuWidget* widget) {
    if (!widget || !root_) return this;
    root_->add(widget, false);
    root_->updateLayout();
    return this;
}

KitsuPopup* KitsuPopup::addButton(const std::string& text,
                                 std::function<void()> cb) {
    auto* btn = new KitsuButton(text);
    btn->onClick(std::move(cb));
    owned_widgets_.push_back(btn);
    if (root_) root_->add(btn, false);
    return this;
}

KitsuPopup* KitsuPopup::addLabel(const std::string& text) {
    auto* lbl = new KitsuLabel(text);
    lbl->align(TextAlign::CENTER);
    owned_widgets_.push_back(lbl);
    if (root_) root_->add(lbl, false);
    return this;
}

// ============================================================
// Ciclo de vida
// ============================================================
KitsuPopup* KitsuPopup::show() {
    if (window_) {
        SDL_ShowWindow(window_);
        SDL_RaiseWindow(window_);
        open_ = true;
        if (krenderer_) krenderer_->invalidate();
    }
    return this;
}

KitsuPopup* KitsuPopup::hide() {
    if (window_) SDL_HideWindow(window_);
    return this;
}

KitsuPopup* KitsuPopup::close() {
    open_ = false;
    if (on_close_) on_close_();
    return this;
}

// ============================================================
// Callbacks
// ============================================================
KitsuPopup* KitsuPopup::onClose(std::function<void()> cb) {
    on_close_ = std::move(cb);
    return this;
}

KitsuPopup* KitsuPopup::onResult(std::function<void(int)> cb) {
    on_result_ = std::move(cb);
    return this;
}

// ============================================================
// Layout / render
// ============================================================
void KitsuPopup::updateLayout() {
    if (!root_) return;
    root_->place(0, 0, width_, height_);
    root_->updateLayout();
}

void KitsuPopup::render() {
    if (!window_ || !renderer_ || !krenderer_) return;
    if (!open_) return;

    updateLayout();

    krenderer_->begin();

    KitsuTheme& t = KitsuTheme::active();
    SDL_SetRenderDrawColor(renderer_,
        (Uint8)t.bg_primary.r, (Uint8)t.bg_primary.g,
        (Uint8)t.bg_primary.b, 255);
    SDL_RenderClear(renderer_);

    root_->render(renderer_);

    krenderer_->end();
}

// ============================================================
// Eventos
// ============================================================
void KitsuPopup::handleEvent(const SDL_Event& e) {
    if (!open_ || !window_) return;

    if (e.type == SDL_WINDOWEVENT) {
        if (e.window.windowID != window_id_) return;

        if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
            close();
            return;
        }

        if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
            width_ = e.window.data1;
            height_ = e.window.data2;
            if (krenderer_) krenderer_->resize(width_, height_);
            if (root_) root_->place(0, 0, width_, height_);
            updateLayout();
        }
        return;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP ||
        e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEWHEEL) {
        SDL_Window* focused = SDL_GetMouseFocus();
        if (!focused) return;
        if (SDL_GetWindowID(focused) != window_id_) return;
    }

    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_TEXTINPUT) {
        SDL_Window* kb = SDL_GetKeyboardFocus();
        if (!kb) return;
        if (SDL_GetWindowID(kb) != window_id_) return;
    }

    if (e.type == SDL_KEYDOWN && close_on_escape_) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            close();
            return;
        }
    }

    if (root_) root_->handleEvent(e);
}

void KitsuPopup::tick() {
    if (!open_) return;
    if (root_) root_->tick();
}

// ============================================================
// Helpers Popup::
// ============================================================
namespace Popup {

KitsuPopup* message(const std::string& title, const std::string& msg) {
    auto* p = new KitsuPopup(400, 200, title);
    p->type(PopupType::MESSAGE)->modal(true)->message(msg);
    p->addButton("OK", [p]() { p->close(); });
    p->updateLayout();
    return p;
}

KitsuPopup* confirm(const std::string& title, const std::string& msg,
                   std::function<void(bool)> cb) {
    auto* p = new KitsuPopup(400, 220, title);
    p->type(PopupType::CONFIRM)->modal(true)->message(msg);

    auto* row = new KitsuHBox();
    row->spacing(10)->padding(0);
    row->align(KitsuAlign::CENTER);
    row->justify(KitsuJustify::CENTER);

    auto* yes = new KitsuButton("Sí");
    yes->onClick([p, cb]() {
        if (cb) cb(true);
        p->close();
    });
    row->add(yes, true);

    auto* no = new KitsuButton("No");
    no->onClick([p, cb]() {
        if (cb) cb(false);
        p->close();
    });
    row->add(no, true);

    p->add(row);
    p->updateLayout();
    return p;
}

KitsuPopup* input(const std::string& title, const std::string& msg,
                 std::function<void(const std::string&)> cb) {
    auto* p = new KitsuPopup(420, 260, title);
    p->type(PopupType::INPUT)->modal(true)->message(msg);

    auto* ti = new KitsuTextInput();
    ti->placeholder("Escribe aquí...");
    ti->size(380, 32);
    p->add(ti);

    auto* row = new KitsuHBox();
    row->spacing(10)->padding(0);
    row->align(KitsuAlign::CENTER);
    row->justify(KitsuJustify::CENTER);

    auto* ok = new KitsuButton("OK");
    ok->onClick([p, ti, cb]() {
        if (cb) cb(ti->text());
        p->close();
    });
    row->add(ok, true);

    auto* cancel = new KitsuButton("Cancelar");
    cancel->onClick([p]() { p->close(); });
    row->add(cancel, true);

    p->add(row);
    p->updateLayout();
    return p;
}

// Helper interno para popups con icono
static KitsuPopup* icon_popup(const std::string& title,
                              const std::string& msg,
                              const std::string& icon_name) {
    auto* p = new KitsuPopup(460, 220, title);
    p->type(PopupType::MESSAGE)->modal(true);

    auto* row = new KitsuHBox();
    row->spacing(15)->padding(0);
    row->align(KitsuAlign::CENTER);
    row->justify(KitsuJustify::START);

    auto* icon = new KitsuIconView(icon_name, 48, IconSource::SYSTEM_THEME);
    row->add(icon, true);

    auto* label = new KitsuLabel(msg);
    label->align(TextAlign::LEFT);
    label->valign(TextVAlign::MIDDLE);
    row->add(label, true);

    p->add(row);
    p->addButton("OK", [p]() { p->close(); });
    p->updateLayout();
    return p;
}

KitsuPopup* warning(const std::string& t, const std::string& m) {
    return icon_popup(t, m, "dialog-warning");
}

KitsuPopup* error(const std::string& t, const std::string& m) {
    return icon_popup(t, m, "dialog-error");
}

KitsuPopup* info(const std::string& t, const std::string& m) {
    return icon_popup(t, m, "dialog-information");
}

KitsuPopup* success(const std::string& t, const std::string& m) {
    return icon_popup(t, m, "dialog-information");
}

} // namespace Popup

// ============================================================
// Modal activo
// ============================================================
KitsuPopup* activeModal() {
    for (auto* p : g_popups) {
        if (p && p->isOpen() && p->isModal()) return p;
    }
    return nullptr;
}

} // namespace KitsuGui
