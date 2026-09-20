#include "kitsugui/theme.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

namespace KitsuGui {

static KitsuTheme s_current_theme = KitsuTheme::KitsuMetroLight();

// ===== HELPER: parsear hex "#RGB", "#RRGGBB", "#RRGGBBAA" =====
static Color parseHex(const std::string& str) {
    std::string s = str;
    if (!s.empty() && s[0] == '#') s = s.substr(1);
    
    auto hexToInt = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    };
    
    // Formato #RRGGBB (6 dígitos)
    if (s.size() == 6) {
        int r = hexToInt(s[0]) * 16 + hexToInt(s[1]);
        int g = hexToInt(s[2]) * 16 + hexToInt(s[3]);
        int b = hexToInt(s[4]) * 16 + hexToInt(s[5]);
        return Color(r, g, b, 255);
    }
    // Formato #RRGGBBAA (8 dígitos)
    if (s.size() == 8) {
        int r = hexToInt(s[0]) * 16 + hexToInt(s[1]);
        int g = hexToInt(s[2]) * 16 + hexToInt(s[3]);
        int b = hexToInt(s[4]) * 16 + hexToInt(s[5]);
        int a = hexToInt(s[6]) * 16 + hexToInt(s[7]);
        return Color(r, g, b, a);
    }
    // Formato #RGB (3 dígitos, CSS shorthand)
    if (s.size() == 3) {
        int r = hexToInt(s[0]) * 17;   // 0xF -> 0xFF
        int g = hexToInt(s[1]) * 17;
        int b = hexToInt(s[2]) * 17;
        return Color(r, g, b, 255);
    }
    return Color(0, 0, 0, 255);
}

// ===== HELPER: trim =====
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// ============================================================
// KITSUMETRO LIGHT
// ============================================================
KitsuTheme KitsuTheme::KitsuMetroLight() {
    KitsuTheme t;
    t.name = "KitsuMetro Light";
    t.author = "KitsuGui";
    t.is_dark = false;
    
    // ===== Paleta base =====
    t.palette["orange"]        = Color(255, 136, 0);
    t.palette["orange_dark"]   = Color(220, 100, 0);
    t.palette["orange_light"]  = Color(255, 170, 68);
    t.palette["white_cold"]    = Color(248, 249, 250);
    t.palette["white"]         = Color(255, 255, 255);
    t.palette["gray_50"]       = Color(245, 245, 247);
    t.palette["gray_100"]      = Color(230, 230, 233);
    t.palette["gray_200"]      = Color(200, 200, 205);
    t.palette["gray_400"]      = Color(150, 150, 160);
    t.palette["gray_600"]      = Color(120, 120, 130);
    t.palette["gray_900"]      = Color(40, 40, 45);
    t.palette["black"]         = Color(0, 0, 0);
    t.palette["red"]           = Color(220, 60, 60);
    t.palette["green"]         = Color(76, 175, 80);
    t.palette["blue"]          = Color(33, 150, 243);
    t.palette["yellow"]        = Color(255, 220, 60);
    
    // ===== Fondos =====
    t.bg_primary   = t.palette["white_cold"];
    t.bg_secondary = t.palette["white"];
    t.bg_tertiary  = t.palette["gray_50"];
    t.bg_disabled  = Color(240, 240, 242);
    
    // ===== Texto =====
    t.text_primary   = t.palette["gray_900"];
    t.text_secondary = t.palette["gray_600"];
    t.text_disabled  = Color(160, 160, 165);
    t.text_on_accent = t.palette["white"];
    
    // ===== Acento =====
    t.accent          = t.palette["orange"];
    t.accent_hover    = t.palette["orange_dark"];
    t.accent_pressed  = t.palette["orange"];
    t.accent_disabled = Color(200, 200, 205);
    
    // ===== Bordes =====
    t.border          = t.palette["gray_200"];
    t.border_focus    = t.palette["orange"];
    t.border_disabled = Color(200, 200, 205);
    
    // ===== Estados =====
    t.success = t.palette["green"];
    t.warning = t.palette["yellow"];
    t.error   = t.palette["red"];
    t.info    = t.palette["blue"];
    
    // ===== Extras =====
    t.scrollbar_track       = Color(200, 200, 205, 80);
    t.scrollbar_thumb       = Color(140, 140, 150, 180);
    t.scrollbar_thumb_hover = Color(255, 136, 0, 220);
    t.selection             = Color(100, 150, 255, 100);
    t.shadow                = Color(0, 0, 0, 60);
    
    // ===== Estilos KitsuMetro (cuadrado puro) =====
    // Todos los radios a 0 para estilo Metro
    t.radius_small  = 0.0f;
    t.radius_medium = 0.0f;
    t.radius_large  = 0.0f;
    
    t.radius_checkbox       = 0.0f;
    t.radius_switch         = 0.0f;
    t.radius_switch_knob    = 0.0f;
    t.radius_slider_track   = 0.0f;
    t.radius_slider_knob    = 0.0f;
    t.radius_radio          = 0.0f;
    t.radius_button         = 0.0f;
    t.radius_panel          = 0.0f;
    t.radius_text_input     = 0.0f;
    t.radius_viewport       = 0.0f;
    t.radius_progress_bar   = 0.0f;
    
    // Grosor de bordes
    t.border_thin   = 1;
    t.border_normal = 2;
    t.border_thick  = 3;
    
    t.border_thickness_button     = 2;
    t.border_thickness_checkbox   = 2;
    t.border_thickness_switch     = 2;
    t.border_thickness_text_input = 2;
    t.border_thickness_panel      = 3;
    t.border_thickness_viewport   = 3;
    
    // Dimensiones
    t.checkbox_size         = 18;
    t.switch_width          = 44;
    t.switch_height         = 22;
    t.switch_knob_pad       = 3;
    t.slider_track_thickness = 6;
    t.slider_knob_size      = 16;
    t.radio_size            = 18;
    t.text_input_height     = 32;
    t.button_height         = 42;
    
    // Tipografía
    t.font_size_small  = 13;
    t.font_size_normal = 15;
    t.font_size_large  = 18;
    t.font_size_title  = 22;
    t.font_size_huge   = 28;
    
    return t;
}

// ============================================================
// KITSUMETRO DARK
// ============================================================
KitsuTheme KitsuTheme::KitsuMetroDark() {
    KitsuTheme t;
    t.name = "KitsuMetro Dark";
    t.author = "KitsuGui";
    t.is_dark = true;
    
    // ===== Paleta base =====
    t.palette["orange"]       = Color(255, 136, 0);
    t.palette["orange_dark"]  = Color(220, 100, 0);
    t.palette["orange_light"] = Color(255, 170, 68);
    t.palette["dark_900"]     = Color(26, 24, 22);
    t.palette["dark_800"]     = Color(35, 32, 30);
    t.palette["dark_700"]     = Color(45, 42, 40);
    t.palette["dark_600"]     = Color(60, 58, 55);
    t.palette["dark_500"]     = Color(80, 78, 75);
    t.palette["gray_100"]     = Color(200, 200, 205);
    t.palette["gray_300"]     = Color(140, 140, 150);
    t.palette["white"]        = Color(255, 255, 255);
    t.palette["red"]          = Color(240, 80, 80);
    t.palette["green"]        = Color(100, 200, 100);
    t.palette["blue"]         = Color(60, 170, 255);
    t.palette["yellow"]       = Color(255, 220, 60);
    
    // ===== Fondos =====
    t.bg_primary   = t.palette["dark_700"];   // Fondo general
    t.bg_secondary = t.palette["dark_800"];   // Fondo de panels
    t.bg_tertiary  = t.palette["dark_600"];   // Fondo de widgets
    t.bg_disabled  = Color(30, 28, 26);
    
    // ===== Texto =====
    t.text_primary   = Color(240, 240, 245);
    t.text_secondary = t.palette["gray_300"];
    t.text_disabled  = Color(100, 100, 110);
    t.text_on_accent = t.palette["white"];
    
    // ===== Acento =====
    t.accent          = t.palette["orange"];
    t.accent_hover    = t.palette["orange_light"];
    t.accent_pressed  = t.palette["orange_dark"];
    t.accent_disabled = Color(100, 90, 80);
    
    // ===== Bordes =====
    t.border          = t.palette["dark_500"];
    t.border_focus    = t.palette["orange"];
    t.border_disabled = Color(80, 80, 85);
    
    // ===== Estados =====
    t.success = t.palette["green"];
    t.warning = t.palette["yellow"];
    t.error   = t.palette["red"];
    t.info    = t.palette["blue"];
    
    // ===== Extras =====
    t.scrollbar_track       = Color(100, 100, 105, 80);
    t.scrollbar_thumb       = Color(160, 160, 170, 180);
    t.scrollbar_thumb_hover = Color(255, 180, 100, 220);
    t.selection             = Color(255, 180, 100, 100);
    t.shadow                = Color(0, 0, 0, 120);
    
    // ===== Estilos KitsuMetro (cuadrado puro) =====
    t.radius_small  = 0.0f;
    t.radius_medium = 0.0f;
    t.radius_large  = 0.0f;
    
    t.radius_checkbox       = 0.0f;
    t.radius_switch         = 0.0f;
    t.radius_switch_knob    = 0.0f;
    t.radius_slider_track   = 0.0f;
    t.radius_slider_knob    = 0.0f;
    t.radius_radio          = 0.0f;
    t.radius_button         = 0.0f;
    t.radius_panel          = 0.0f;
    t.radius_text_input     = 0.0f;
    t.radius_viewport       = 0.0f;
    t.radius_progress_bar   = 0.0f;
    
    t.border_thin   = 1;
    t.border_normal = 2;
    t.border_thick  = 3;
    
    t.border_thickness_button     = 2;
    t.border_thickness_checkbox   = 2;
    t.border_thickness_switch     = 2;
    t.border_thickness_text_input = 2;
    t.border_thickness_panel      = 3;
    t.border_thickness_viewport   = 3;
    
    t.checkbox_size         = 18;
    t.switch_width          = 44;
    t.switch_height         = 22;
    t.switch_knob_pad       = 3;
    t.slider_track_thickness = 6;
    t.slider_knob_size      = 16;
    t.radio_size            = 18;
    t.text_input_height     = 32;
    t.button_height         = 42;
    
    t.font_size_small  = 13;
    t.font_size_normal = 15;
    t.font_size_large  = 18;
    t.font_size_title  = 22;
    t.font_size_huge   = 28;
    
    return t;
}

// ============================================================
// APLICAR / ACCEDER
// ============================================================
void KitsuTheme::apply(const KitsuTheme& theme) {
    s_current_theme = theme;
}

KitsuTheme& KitsuTheme::current() {
    return s_current_theme;
}

// ============================================================
// RESOLVER COLOR
// ============================================================
Color KitsuTheme::resolveColor(const std::string& value) const {
    // @palette:key
    if (value.size() > 9 && value.substr(0, 9) == "@palette:") {
        std::string key = value.substr(9);
        auto it = palette.find(key);
        if (it != palette.end()) return it->second;
        return Color(255, 0, 255);   // magenta = error
    }
    // #hex
    if (!value.empty() && value[0] == '#') {
        return parseHex(value);
    }
    return Color(255, 0, 255);
}

// ============================================================
// SET TOKEN
// ============================================================
bool KitsuTheme::setToken(const std::string& key, const std::string& value) {
    Color c = resolveColor(value);
    
    // ===== Colores =====
    if (key == "bg_primary")       { bg_primary = c; return true; }
    if (key == "bg_secondary")     { bg_secondary = c; return true; }
    if (key == "bg_tertiary")      { bg_tertiary = c; return true; }
    if (key == "bg_disabled")      { bg_disabled = c; return true; }
    if (key == "text_primary")     { text_primary = c; return true; }
    if (key == "text_secondary")   { text_secondary = c; return true; }
    if (key == "text_disabled")    { text_disabled = c; return true; }
    if (key == "text_on_accent")   { text_on_accent = c; return true; }
    if (key == "accent")           { accent = c; return true; }
    if (key == "accent_hover")     { accent_hover = c; return true; }
    if (key == "accent_pressed")   { accent_pressed = c; return true; }
    if (key == "accent_disabled")  { accent_disabled = c; return true; }
    if (key == "border")           { border = c; return true; }
    if (key == "border_focus")     { border_focus = c; return true; }
    if (key == "border_disabled")  { border_disabled = c; return true; }
    if (key == "success")          { success = c; return true; }
    if (key == "warning")          { warning = c; return true; }
    if (key == "error")            { error = c; return true; }
    if (key == "info")             { info = c; return true; }
    if (key == "scrollbar_track")  { scrollbar_track = c; return true; }
    if (key == "scrollbar_thumb")  { scrollbar_thumb = c; return true; }
    if (key == "scrollbar_thumb_hover") { scrollbar_thumb_hover = c; return true; }
    if (key == "selection")        { selection = c; return true; }
    if (key == "shadow")           { shadow = c; return true; }
    
    // ===== Radios base =====
    if (key == "radius_small")  { radius_small = std::stof(value); return true; }
    if (key == "radius_medium") { radius_medium = std::stof(value); return true; }
    if (key == "radius_large")  { radius_large = std::stof(value); return true; }
    if (key == "radius_pill")   { radius_pill = std::stof(value); return true; }
    
    // ===== Radios por widget =====
    if (key == "radius_checkbox")     { radius_checkbox = std::stof(value); return true; }
    if (key == "radius_switch")       { radius_switch = std::stof(value); return true; }
    if (key == "radius_switch_knob")  { radius_switch_knob = std::stof(value); return true; }
    if (key == "radius_slider_track") { radius_slider_track = std::stof(value); return true; }
    if (key == "radius_slider_knob")  { radius_slider_knob = std::stof(value); return true; }
    if (key == "radius_radio")        { radius_radio = std::stof(value); return true; }
    if (key == "radius_button")       { radius_button = std::stof(value); return true; }
    if (key == "radius_panel")        { radius_panel = std::stof(value); return true; }
    if (key == "radius_text_input")   { radius_text_input = std::stof(value); return true; }
    if (key == "radius_viewport")     { radius_viewport = std::stof(value); return true; }
    if (key == "radius_progress_bar") { radius_progress_bar = std::stof(value); return true; }
    
    // ===== Espaciados =====
    if (key == "spacing_small")  { spacing_small = std::stoi(value); return true; }
    if (key == "spacing_medium") { spacing_medium = std::stoi(value); return true; }
    if (key == "spacing_large")  { spacing_large = std::stoi(value); return true; }
    if (key == "spacing_xlarge") { spacing_xlarge = std::stoi(value); return true; }
    
    // ===== Grosor de bordes =====
    if (key == "border_thin")   { border_thin = std::stoi(value); return true; }
    if (key == "border_normal") { border_normal = std::stoi(value); return true; }
    if (key == "border_thick")  { border_thick = std::stoi(value); return true; }
    
    if (key == "border_thickness_button")     { border_thickness_button = std::stoi(value); return true; }
    if (key == "border_thickness_checkbox")   { border_thickness_checkbox = std::stoi(value); return true; }
    if (key == "border_thickness_switch")     { border_thickness_switch = std::stoi(value); return true; }
    if (key == "border_thickness_text_input") { border_thickness_text_input = std::stoi(value); return true; }
    if (key == "border_thickness_panel")      { border_thickness_panel = std::stoi(value); return true; }
    if (key == "border_thickness_viewport")   { border_thickness_viewport = std::stoi(value); return true; }
    
    // ===== Dimensiones =====
    if (key == "checkbox_size")          { checkbox_size = std::stoi(value); return true; }
    if (key == "switch_width")           { switch_width = std::stoi(value); return true; }
    if (key == "switch_height")          { switch_height = std::stoi(value); return true; }
    if (key == "switch_knob_pad")        { switch_knob_pad = std::stoi(value); return true; }
    if (key == "slider_track_thickness") { slider_track_thickness = std::stoi(value); return true; }
    if (key == "slider_knob_size")       { slider_knob_size = std::stoi(value); return true; }
    if (key == "radio_size")             { radio_size = std::stoi(value); return true; }
    if (key == "text_input_height")      { text_input_height = std::stoi(value); return true; }
    if (key == "button_height")          { button_height = std::stoi(value); return true; }
    
    // ===== Tipografía =====
    if (key == "font_size_small")  { font_size_small = std::stoi(value); return true; }
    if (key == "font_size_normal") { font_size_normal = std::stoi(value); return true; }
    if (key == "font_size_large")  { font_size_large = std::stoi(value); return true; }
    if (key == "font_size_title")  { font_size_title = std::stoi(value); return true; }
    if (key == "font_size_huge")   { font_size_huge = std::stoi(value); return true; }
    
    return false;
}

bool KitsuTheme::setPaletteColor(const std::string& name, const std::string& hex) {
    palette[name] = parseHex(hex);
    return true;
}

// ============================================================
// CARGAR DESDE ARCHIVO
// ============================================================
bool KitsuTheme::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    KitsuTheme t;
    std::string section;
    std::string line;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        
        if (section == "metadata") {
            if (key == "name") t.name = value;
            else if (key == "author") t.author = value;
            else if (key == "is_dark") t.is_dark = (value == "true");
        }
        else if (section == "palette") {
            t.setPaletteColor(key, value);
        }
        else if (section == "tokens" || section == "style" || section == "sizes") {
            t.setToken(key, value);
        }
    }
    
    s_current_theme = t;
    return true;
}

// ============================================================
// GUARDAR A ARCHIVO
// ============================================================
bool KitsuTheme::saveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    file << "# KitsuGui Theme File\n\n";
    
    file << "[metadata]\n";
    file << "name = " << name << "\n";
    file << "author = " << author << "\n";
    file << "is_dark = " << (is_dark ? "true" : "false") << "\n\n";
    
    file << "[palette]\n";
    for (const auto& kv : palette) {
        char buf[32];
        snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X",
                 kv.second.r, kv.second.g, kv.second.b, kv.second.a);
        file << kv.first << " = " << buf << "\n";
    }
    file << "\n";
    
    auto writeColor = [&](const std::string& k, const Color& c) {
        char buf[32];
        snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", c.r, c.g, c.b, c.a);
        file << k << " = " << buf << "\n";
    };
    
    file << "[tokens]\n";
    writeColor("bg_primary", bg_primary);
    writeColor("bg_secondary", bg_secondary);
    writeColor("bg_tertiary", bg_tertiary);
    writeColor("bg_disabled", bg_disabled);
    writeColor("text_primary", text_primary);
    writeColor("text_secondary", text_secondary);
    writeColor("text_disabled", text_disabled);
    writeColor("text_on_accent", text_on_accent);
    writeColor("accent", accent);
    writeColor("accent_hover", accent_hover);
    writeColor("accent_pressed", accent_pressed);
    writeColor("accent_disabled", accent_disabled);
    writeColor("border", border);
    writeColor("border_focus", border_focus);
    writeColor("border_disabled", border_disabled);
    writeColor("success", success);
    writeColor("warning", warning);
    writeColor("error", error);
    writeColor("info", info);
    writeColor("scrollbar_track", scrollbar_track);
    writeColor("scrollbar_thumb", scrollbar_thumb);
    writeColor("scrollbar_thumb_hover", scrollbar_thumb_hover);
    writeColor("selection", selection);
    writeColor("shadow", shadow);
    file << "\n";
    
    file << "[style]\n";
    file << "radius_small = " << radius_small << "\n";
    file << "radius_medium = " << radius_medium << "\n";
    file << "radius_large = " << radius_large << "\n";
    file << "radius_checkbox = " << radius_checkbox << "\n";
    file << "radius_switch = " << radius_switch << "\n";
    file << "radius_switch_knob = " << radius_switch_knob << "\n";
    file << "radius_slider_track = " << radius_slider_track << "\n";
    file << "radius_slider_knob = " << radius_slider_knob << "\n";
    file << "radius_radio = " << radius_radio << "\n";
    file << "radius_button = " << radius_button << "\n";
    file << "radius_panel = " << radius_panel << "\n";
    file << "radius_text_input = " << radius_text_input << "\n";
    file << "radius_viewport = " << radius_viewport << "\n";
    file << "radius_progress_bar = " << radius_progress_bar << "\n";
    file << "spacing_small = " << spacing_small << "\n";
    file << "spacing_medium = " << spacing_medium << "\n";
    file << "spacing_large = " << spacing_large << "\n";
    file << "spacing_xlarge = " << spacing_xlarge << "\n";
    file << "border_thin = " << border_thin << "\n";
    file << "border_normal = " << border_normal << "\n";
    file << "border_thick = " << border_thick << "\n";
    file << "border_thickness_button = " << border_thickness_button << "\n";
    file << "border_thickness_checkbox = " << border_thickness_checkbox << "\n";
    file << "border_thickness_switch = " << border_thickness_switch << "\n";
    file << "border_thickness_text_input = " << border_thickness_text_input << "\n";
    file << "border_thickness_panel = " << border_thickness_panel << "\n";
    file << "border_thickness_viewport = " << border_thickness_viewport << "\n";
    file << "\n";
    
    file << "[sizes]\n";
    file << "checkbox_size = " << checkbox_size << "\n";
    file << "switch_width = " << switch_width << "\n";
    file << "switch_height = " << switch_height << "\n";
    file << "switch_knob_pad = " << switch_knob_pad << "\n";
    file << "slider_track_thickness = " << slider_track_thickness << "\n";
    file << "slider_knob_size = " << slider_knob_size << "\n";
    file << "radio_size = " << radio_size << "\n";
    file << "text_input_height = " << text_input_height << "\n";
    file << "button_height = " << button_height << "\n";
    file << "\n";
    
    file << "[typography]\n";
    file << "font_size_small = " << font_size_small << "\n";
    file << "font_size_normal = " << font_size_normal << "\n";
    file << "font_size_large = " << font_size_large << "\n";
    file << "font_size_title = " << font_size_title << "\n";
    file << "font_size_huge = " << font_size_huge << "\n";
    
    return true;
}

} // namespace KitsuGui
