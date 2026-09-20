#include "kitsugui/icon_theme.h"
#include "internal.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>

namespace KitsuGui {

// ============================================================
// HELPERS
// ============================================================

static std::string getHomeDir() {
    const char* home = getenv("HOME");
    return home ? std::string(home) : "";
}

static std::string getPREFIX() {
    const char* prefix = getenv("PREFIX");
    return prefix ? std::string(prefix) : "/usr";
}

static bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

static bool dirExists(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
    return S_ISDIR(buffer.st_mode);
}

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        std::string t = trim(item);
        if (!t.empty()) out.push_back(t);
    }
    return out;
}

// Comprueba si un comando está disponible en el PATH
static bool commandExists(const std::string& cmd) {
    std::string check = "command -v " + cmd + " > /dev/null 2>&1";
    return (system(check.c_str()) == 0);
}

// Convierte SVG a PNG temporal con rsvg-convert.
// Devuelve la ruta del PNG temporal, o "" si falla.
static std::string svgToPng(const std::string& svg_path, int size) {
    static int counter = 0;
    char tmp_path[256];
    snprintf(tmp_path, sizeof(tmp_path), "/tmp/kitsu_icon_%d.png", counter++);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "rsvg-convert -w %d -h %d -o %s \"%s\" 2>/dev/null",
             size, size, tmp_path, svg_path.c_str());
    
    int result = system(cmd);
    if (result != 0) return "";
    if (!fileExists(tmp_path)) return "";
    
    return std::string(tmp_path);
}

// ============================================================
// KitsuIconTheme - SINGLETON
// ============================================================
KitsuIconTheme& KitsuIconTheme::instance() {
    static KitsuIconTheme inst;
    return inst;
}

KitsuIconTheme::KitsuIconTheme() {
    std::string home = getHomeDir();
    std::string prefix = getPREFIX();
    
    if (!home.empty()) {
        search_paths.push_back(home + "/.icons");
        search_paths.push_back(home + "/.local/share/icons");
    }
    search_paths.push_back(prefix + "/share/icons");
    search_paths.push_back("/usr/share/icons");
    search_paths.push_back("/usr/local/share/icons");
    
    // Directorio local del proyecto
    search_paths.push_back("data/icons");
}

KitsuIconTheme::~KitsuIconTheme() = default;

// ============================================================
// PARSEAR index.theme
// ============================================================
bool KitsuIconTheme::parseIndexTheme(const std::string& path, ThemeInfo& out) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    std::string section;
    std::string line;
    out.base_path = path.substr(0, path.find_last_of('/'));
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        
        if (section == "Icon Theme") {
            if (key == "Name") out.name = value;
            else if (key == "Comment") out.comment = value;
            else if (key == "Inherits") {
                out.inherits = split(value, ',');
                if (std::find(out.inherits.begin(), out.inherits.end(), "hicolor")
                    == out.inherits.end()) {
                    out.inherits.push_back("hicolor");
                }
            }
        }
        else {
            auto& dir = out.directories[section];
            if (key == "Size") dir.size = std::stoi(value);
            else if (key == "Type") dir.type = value;
            else if (key == "Scale") dir.scale = std::stoi(value);
            else if (key == "MinSize") dir.min_size = std::stoi(value);
            else if (key == "MaxSize") dir.max_size = std::stoi(value);
            else if (key == "Threshold") dir.threshold = std::stoi(value);
        }
    }
    
    return true;
}

// ============================================================
// CARGAR TEMA
// ============================================================
bool KitsuIconTheme::load(const std::string& theme_name) {
    themes.clear();
    
    // Buscar el tema en las rutas
    std::string found_path;
    for (const auto& base : search_paths) {
        std::string path = base + "/" + theme_name + "/index.theme";
        if (fileExists(path)) {
            found_path = path;
            break;
        }
    }
    
    if (found_path.empty()) {
        SDL_Log("KitsuIconTheme: tema '%s' no encontrado", theme_name.c_str());
        return false;
    }
    
    ThemeInfo main_theme;
    if (!parseIndexTheme(found_path, main_theme)) {
        SDL_Log("KitsuIconTheme: error parseando %s", found_path.c_str());
        return false;
    }
    
    main_theme.name = theme_name;
    themes[theme_name] = main_theme;
    
    // Cargar herencia recursivamente
    std::vector<std::string> to_load = main_theme.inherits;
    std::vector<std::string> loaded_names = {theme_name};
    
    while (!to_load.empty()) {
        std::string parent = to_load.front();
        to_load.erase(to_load.begin());
        
        if (std::find(loaded_names.begin(), loaded_names.end(), parent)
            != loaded_names.end()) {
            continue;
        }
        loaded_names.push_back(parent);
        
        std::string parent_path;
        for (const auto& base : search_paths) {
            std::string path = base + "/" + parent + "/index.theme";
            if (fileExists(path)) {
                parent_path = path;
                break;
            }
        }
        
        if (parent_path.empty()) continue;
        
        ThemeInfo parent_theme;
        if (parseIndexTheme(parent_path, parent_theme)) {
            parent_theme.name = parent;
            themes[parent] = parent_theme;
            
            for (const auto& p : parent_theme.inherits) {
                if (std::find(loaded_names.begin(), loaded_names.end(), p)
                    == loaded_names.end()) {
                    to_load.push_back(p);
                }
            }
        }
    }
    
    current_theme = theme_name;
    loaded = true;
    
    SDL_Log("KitsuIconTheme: '%s' cargado (%zu temas en la cadena)",
            theme_name.c_str(), themes.size());
    return true;
}

// ============================================================
// BUSCAR ARCHIVO DE ICONO
// ============================================================
std::string KitsuIconTheme::findIconFile(const std::string& dir,
                                         const std::string& name) const {
    const char* exts[] = { ".png", ".svg" };
    
    for (const char* ext : exts) {
        std::string path = dir + "/" + name + ext;
        if (fileExists(path)) return path;
    }
    return "";
}

// ============================================================
// BUSCAR EN UN TEMA (una fase)
// ============================================================
std::string KitsuIconTheme::lookupInTheme(const ThemeInfo& theme,
                                          const std::string& name,
                                          int size) const {
    std::string best_match;
    int best_distance = 999999;
    
    // ===== FASE 1: buscar en directorios que coincidan con el tamaño =====
    for (const auto& kv : theme.directories) {
        const std::string& subdir = kv.first;
        const ThemeInfo::DirInfo& info = kv.second;
        
        bool matches = false;
        if (info.type == "Fixed") {
            matches = (info.size == size);
        } else if (info.type == "Scalable") {
            matches = (size >= info.min_size && size <= info.max_size);
        } else {
            matches = (size >= info.size - info.threshold &&
                       size <= info.size + info.threshold);
        }
        
        if (!matches) continue;
        
        std::string dir = theme.base_path + "/" + subdir;
        std::string path = findIconFile(dir, name);
        if (!path.empty()) return path;
        
        // Subcategoría (ej: "actions/edit-copy")
        if (name.find('/') != std::string::npos) {
            std::string sub = name.substr(0, name.find('/'));
            std::string icon = name.substr(name.find('/') + 1);
            std::string subdir_path = dir + "/" + sub;
            if (dirExists(subdir_path)) {
                path = findIconFile(subdir_path, icon);
                if (!path.empty()) return path;
            }
        }
    }
    
    // ===== FASE 2: cualquier tamaño, elegir el más cercano =====
    for (const auto& kv : theme.directories) {
        const std::string& subdir = kv.first;
        std::string dir = theme.base_path + "/" + subdir;
        
        std::string path = findIconFile(dir, name);
        if (!path.empty()) {
            int dist = abs(kv.second.size - size);
            if (dist < best_distance) {
                best_distance = dist;
                best_match = path;
            }
        }
        
        if (name.find('/') != std::string::npos) {
            std::string sub = name.substr(0, name.find('/'));
            std::string icon = name.substr(name.find('/') + 1);
            std::string subdir_path = dir + "/" + sub;
            if (dirExists(subdir_path)) {
                std::string p = findIconFile(subdir_path, icon);
                if (!p.empty()) {
                    int dist = abs(kv.second.size - size);
                    if (dist < best_distance) {
                        best_distance = dist;
                        best_match = p;
                    }
                }
            }
        }
    }
    
    return best_match;
}

// ============================================================
// BUSCAR CON HERENCIA
// ============================================================
std::string KitsuIconTheme::lookupWithInheritance(const std::string& theme_name,
                                                   const std::string& name,
                                                   int size) const {
    auto it = themes.find(theme_name);
    if (it == themes.end()) return "";
    
    // 1. Buscar en el tema actual
    std::string result = lookupInTheme(it->second, name, size);
    if (!result.empty()) return result;
    
    // 2. Buscar recursivamente en los padres
    for (const auto& parent : it->second.inherits) {
        result = lookupWithInheritance(parent, name, size);
        if (!result.empty()) return result;
    }
    
    return "";
}

// ============================================================
// API PÚBLICA
// ============================================================
std::string KitsuIconTheme::findIcon(const std::string& name, int size) const {
    if (!loaded) return "";
    return lookupWithInheritance(current_theme, name, size);
}

std::vector<std::string> KitsuIconTheme::listAvailableThemes() const {
    std::vector<std::string> result;
    
    for (const auto& base : search_paths) {
        DIR* dir = opendir(base.c_str());
        if (!dir) continue;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            
            std::string theme_path = base + "/" + entry->d_name;
            std::string index_path = theme_path + "/index.theme";
            
            if (fileExists(index_path)) {
                std::string theme_name = entry->d_name;
                if (std::find(result.begin(), result.end(), theme_name)
                    == result.end()) {
                    result.push_back(theme_name);
                }
            }
        }
        closedir(dir);
    }
    
    return result;
}

void KitsuIconTheme::addSearchPath(const std::string& path) {
    search_paths.insert(search_paths.begin(), path);
}

// ============================================================
// CACHE DE TEXTURAS
// ============================================================
// Estrategia:
// - Si el renderer es el PRINCIPAL (g_window->getSDLRenderer()):
//   se cachea la textura. El llamador NO debe destruirla.
// - Si el renderer NO es el principal (ej: popup):
//   se carga fresco SIN cachear. El llamador SÍ debe destruirla.
// El parámetro 'cached' indica al llamador cuál es el caso.

KitsuIconCache& KitsuIconCache::instance() {
    static KitsuIconCache inst;
    return inst;
}

KitsuIconCache::~KitsuIconCache() {
    for (auto& kv : cache) {
        if (kv.second) SDL_DestroyTexture(kv.second);
    }
    cache.clear();
}

SDL_Texture* KitsuIconCache::getTexture(SDL_Renderer* renderer,
                                        const std::string& name,
                                        int size,
                                        bool& cached) {
    cached = false;
    if (!renderer) return nullptr;
    
    // ===== ¿Es el renderer principal? =====
    bool is_main_renderer = false;
    if (g_window) {
        is_main_renderer = (renderer == g_window->getSDLRenderer());
    }
    
    CacheKey key{name, size};
    
    // ===== Buscar en cache (solo si es el principal) =====
    if (is_main_renderer) {
        auto it = cache.find(key);
        if (it != cache.end()) {
            cached = true;
            return it->second;
        }
    }
    
    // ===== Buscar el archivo =====
    std::string path = KitsuIconTheme::instance().findIcon(name, size);
    if (path.empty()) {
        // No encontrado: cachear nullptr solo si es el principal
        if (is_main_renderer) {
            cache[key] = nullptr;
        }
        return nullptr;
    }
    
    // ===== Si es SVG, convertir a PNG temporal =====
    std::string load_path = path;
    std::string tmp_png;
    
    if (path.size() > 4 && path.substr(path.size() - 4) == ".svg") {
        if (commandExists("rsvg-convert")) {
            tmp_png = svgToPng(path, size);
            if (!tmp_png.empty()) {
                load_path = tmp_png;
            }
        }
    }
    
    // ===== Cargar con SDL_image =====
    SDL_Surface* surface = IMG_Load(load_path.c_str());
    
    // Limpiar el PNG temporal
    if (!tmp_png.empty()) {
        remove(tmp_png.c_str());
    }
    
    if (!surface) {
        SDL_Log("KitsuIconCache: error cargando '%s': %s",
                path.c_str(), IMG_GetError());
        if (is_main_renderer) {
            cache[key] = nullptr;
        }
        return nullptr;
    }
    
    // ===== Escalar si el tamaño no coincide =====
    if (surface->w != size || surface->h != size) {
        SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(
            0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
        if (scaled) {
            SDL_BlitScaled(surface, nullptr, scaled, nullptr);
            SDL_FreeSurface(surface);
            surface = scaled;
        }
    }
    
    // ===== Crear textura =====
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        SDL_Log("KitsuIconCache: error creando textura: %s", SDL_GetError());
        return nullptr;
    }
    
    // ===== Cachear solo si es el renderer principal =====
    if (is_main_renderer) {
        cache[key] = texture;
        cached = true;
    }
    // Si no es el principal, cached = false → el llamador la destruye
    
    return texture;
}

void KitsuIconCache::clear() {
    for (auto& kv : cache) {
        if (kv.second) SDL_DestroyTexture(kv.second);
    }
    cache.clear();
}

} // namespace KitsuGui
