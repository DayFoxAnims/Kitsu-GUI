#ifndef KITSUGUI_THEME_H
#define KITSUGUI_THEME_H

#include "kitsugui/color.h"
#include <string>
#include <unordered_map>

namespace KitsuGui {

// ============================================================
// KitsuTheme
// ============================================================
// Sistema de temas con tokens semánticos.
//
// Temas built-in:
//   KitsuTheme::KitsuMetroLight()
//   KitsuTheme::KitsuMetroDark()
//
// Tema activo:
//   KitsuTheme::active()            // getter
//   KitsuTheme::set(theme)          // setter global
//
// Cargar desde archivo:
//   KitsuTheme::loadFromFile("data/themes/kitsu-metro-light.conf");
// ============================================================
struct KitsuTheme {
    // ===== Identidad =====
    std::string name = "KitsuMetro Light";
    std::string author = "KitsuGui";
    bool is_dark = false;

    // ===== Paleta base =====
    std::unordered_map<std::string, Color> palette;

    // ===== Tokens semánticos — Colores =====
    Color bg_primary;
    Color bg_secondary;
    Color bg_tertiary;
    Color bg_disabled;

    Color text_primary;
    Color text_secondary;
    Color text_disabled;
    Color text_on_accent;

    Color accent;
    Color accent_hover;
    Color accent_pressed;
    Color accent_disabled;

    Color border;
    Color border_focus;
    Color border_disabled;

    Color success;
    Color warning;
    Color error;
    Color info;

    Color scrollbar_track;
    Color scrollbar_thumb;
    Color scrollbar_thumb_hover;
    Color selection;
    Color shadow;

    // ===== Radios base =====
    float radius_small = 0.0f;
    float radius_medium = 0.0f;
    float radius_large = 0.0f;
    float radius_pill = 999.0f;

    // ===== Espaciados =====
    int spacing_small = 4;
    int spacing_medium = 8;
    int spacing_large = 16;
    int spacing_xlarge = 24;

    // ===== Grosor de bordes =====
    int border_thin = 1;
    int border_normal = 2;
    int border_thick = 3;

    // ===== Radios por widget =====
    float radius_checkbox = 0.0f;
    float radius_switch = 0.0f;
    float radius_switch_knob = 0.0f;
    float radius_slider_track = 0.0f;
    float radius_slider_knob = 0.0f;
    float radius_radio = 0.0f;
    float radius_button = 0.0f;
    float radius_panel = 0.0f;
    float radius_text_input = 0.0f;
    float radius_viewport = 0.0f;
    float radius_progress_bar = 0.0f;

    // ===== Grosor de borde por widget =====
    int border_thickness_button = 2;
    int border_thickness_checkbox = 2;
    int border_thickness_switch = 2;
    int border_thickness_text_input = 2;
    int border_thickness_panel = 2;
    int border_thickness_viewport = 2;

    // ===== Dimensiones por widget =====
    int checkbox_size = 18;
    int switch_width = 44;
    int switch_height = 22;
    int switch_knob_pad = 3;
    int slider_track_thickness = 6;
    int slider_knob_size = 16;
    int radio_size = 18;
    int text_input_height = 32;
    int button_height = 42;

    // ===== Tipografía =====
    std::string font_family = "DejaVuSans";
    std::string font_mono = "DejaVuSansMono";
    int font_size_small = 13;
    int font_size_normal = 15;
    int font_size_large = 18;
    int font_size_title = 22;
    int font_size_huge = 28;

    // ===== Factories built-in =====
    static KitsuTheme KitsuMetroLight();
    static KitsuTheme KitsuMetroDark();

    // ===== Tema activo (singleton) =====
    static KitsuTheme& active();
    static void set(const KitsuTheme& theme);

    // ===== Carga/guardado =====
    static bool loadFromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;

    // ===== Setters de tokens =====
    bool setToken(const std::string& key, const std::string& value);
    bool setPaletteColor(const std::string& name, const std::string& hex);
    Color resolveColor(const std::string& value) const;
};

} // namespace KitsuGui

#endif
