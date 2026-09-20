#ifndef KITSUGUI_RADIO_H
#define KITSUGUI_RADIO_H

#include "kitsugui/widget.h"
#include "kitsugui/box.h"
#include "kitsugui/theme.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <functional>
#include <vector>

namespace KitsuGui {

// Forward
class KitsuRadioGroup;

// ============================================================
// KitsuRadioButton
// ============================================================
class KitsuRadioButton : public KitsuWidget {
    friend class KitsuRadioGroup;
    
public:
    KitsuRadioButton(const std::string& text = "", bool checked = false);
    ~KitsuRadioButton() override;
    
    KitsuRadioButton& withFont(TTF_Font* font);
    KitsuRadioButton& withChecked(bool c);
    KitsuRadioButton& withCallback(std::function<void(bool)> cb);
    KitsuRadioButton& setBounds(int x, int y, int w, int h) {
        setBoundsInternal(x, y, w, h);
        return *this;
    }
    
    bool isChecked() const { return checked; }
    void setChecked(bool c);
    
    void render(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;
    
    static void setFont(TTF_Font* font) { g_font = font; }
    static TTF_Font* getFont() { return g_font; }
    
private:
    std::string text;
    bool checked = false;
    bool mouse_inside = false;
    bool mouse_down = false;
    bool enabled_ = true;
    TTF_Font* font = nullptr;
    std::function<void(bool)> callback;
    
    KitsuRadioGroup* group = nullptr;
    
    SDL_Texture* text_texture = nullptr;
    int tex_w = 0, tex_h = 0;
    std::string cached_text;
    TTF_Font* cached_font = nullptr;
    Uint8 cached_r = 0, cached_g = 0, cached_b = 0;
    
    static TTF_Font* g_font;
    
    TTF_Font* getActiveFont() const { return font ? font : g_font; }
    void updateTextTexture(SDL_Renderer* renderer);
    void destroyTextTexture();
    
    void activateFromGroup();
};

// ============================================================
// KitsuRadioGroup
// ============================================================
class KitsuRadioGroup : public KitsuBox {
public:
    KitsuRadioGroup();
    ~KitsuRadioGroup() override;
    
    void addRadio(KitsuRadioButton* radio, bool owns = true);
    void removeRadio(KitsuRadioButton* radio);
    
    KitsuRadioButton* getSelected() const { return selected; }
    
    void setOnChange(std::function<void(KitsuRadioButton*)> cb) { on_change = cb; }
    
    void notifySelected(KitsuRadioButton* radio);
    
    void tick() override;
    bool handleEvent(const SDL_Event& e) override;
    
private:
    std::vector<KitsuRadioButton*> radios;
    std::vector<KitsuRadioButton*> owned_radios;
    KitsuRadioButton* selected = nullptr;
    std::function<void(KitsuRadioButton*)> on_change;
};

} // namespace KitsuGui

#endif
