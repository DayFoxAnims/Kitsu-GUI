#include "kitsugui/icon_theme.h"
#include "kitsugui/utils.h"
#include "internal.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

namespace KitsuGui {

// ============================================================
// Helpers
// ============================================================
namespace {

std::string env(const char* name) {
    const char* v = getenv(name);
    return v ? std::string(v) : "";
}

bool fileExists(const std::string& path) {
    struct stat b;
    return (stat(path.c_str(), &b) == 0);
}

bool dirExists(const std::string& path) {
    struct stat b;
    if (stat(path.c_str(), &b) != 0) return false;
    return S_ISDIR(b.st_mode);
}

bool commandExists(const std::string& cmd) {
    std::string check = "command -v " + cmd + " > /dev/null 2>&1";
    return (system(check.c_str()) == 0);
}

// Convierte SVG a PNG temporal con rsvg-convert.
// Devuelve la ruta del PNG temporal, o "" si falla.
std::string svgToPng(const std::string& svg_path, int size) {
    static int counter = 0;
    char tmp_path[256];
    snprintf(tmp_path, sizeof(tmp_path),
             "/tmp/kitsu_icon_%d_%d.png", (int)getpid(), counter++);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "rsvg-convert -w %d -h %d -o \"%s\" \"%s\" 2>/dev/null",
             size, size, tmp_path, svg_path.c_str());

    int result = system(cmd);
    if (result != 0) return "";
    if (!fileExists(tmp_path)) return "";

    return std::string(tmp_path);
}

} // namespace

// ============================================================
// Singleton
// ============================================================
KitsuIconTheme& KitsuIconTheme::instance() {
    static KitsuIconTheme inst;
    return inst;
}

// ============================================================
// Constructor / destructor
// ============================================================
KitsuIconTheme::KitsuIconTheme() {
    std::string home   = env("HOME");
    std::string prefix = env("PREFIX");
    if (prefix.empty()) prefix = "/usr";

    // Rutas locales primero (para desarrollo)
    search_paths_.push_back("data/icons");
    search_paths_.push_back("../data/icons");
    search_paths_.push_back("../../data/icons");

    // Rutas del usuario
    if (!home.empty()) {
        search_paths_.push_back(home + "/.icons");
        search_paths_.push_back(home + "/.local/share/icons");
    }

    // Rutas del sistema
    search_paths_.push_back(prefix + "/share/icons");
    search_paths_.push_back("/usr/share/icons");
    search_paths_.push_back("/usr/local/share/icons");
}

KitsuIconTheme::~KitsuIconTheme() = default;

// ============================================================
// Parsear index.theme
// ============================================================
bool KitsuIconTheme::parseIndexTheme(const std::string& path,
                                     ThemeInfo& out) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string section;
    std::string line;
    out.base_path = path.substr(0, path.find_last_of('/'));

    while (std::getline(file, line)) {
        line = Utils::trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = Utils::trim(line.substr(0, eq));
        std::string value = Utils::trim(line.substr(eq + 1));

        if (section == "Icon Theme") {
            if (key == "Name")         out.name = value;
            else if (key == "Comment") out.comment = value;
            else if (key == "Inherits") {
                out.inherits = Utils::split(value, ',');
                // Asegurar hicolor al final
                if (std::find(out.inherits.begin(), out.inherits.end(),
                              "hicolor") == out.inherits.end()) {
                    out.inherits.push_back("hicolor");
                }
            }
        } else {
            auto& dir = out.directories[section];
            if (key == "Size")            dir.size = Utils::safeStoi(value, 0);
            else if (key == "Type")       dir.type = value;
            else if (key == "Scale")      dir.scale = Utils::safeStoi(value, 1);
            else if (key == "MinSize")    dir.min_size = Utils::safeStoi(value, 0);
            else if (key == "MaxSize")    dir.max_size = Utils::safeStoi(value, 0);
            else if (key == "Threshold")  dir.threshold = Utils::safeStoi(value, 2);
        }
    }

    return true;
}

// ============================================================
// Cargar tema
// ============================================================
bool KitsuIconTheme::load(const std::string& theme_name) {
    themes_.clear();

    // Buscar index.theme
    std::string found_path;
    for (const auto& base : search_paths_) {
        std::string p = base + "/" + theme_name + "/index.theme";
        if (fileExists(p)) {
            found_path = p;
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
    themes_[theme_name] = main_theme;

    // Cargar herencia recursivamente
    std::vector<std::string> to_load = main_theme.inherits;
    std::vector<std::string> loaded = {theme_name};

    while (!to_load.empty()) {
        std::string parent = to_load.front();
        to_load.erase(to_load.begin());

        if (std::find(loaded.begin(), loaded.end(), parent) != loaded.end()) {
            continue;
        }
        loaded.push_back(parent);

        std::string parent_path;
        for (const auto& base : search_paths_) {
            std::string p = base + "/" + parent + "/index.theme";
            if (fileExists(p)) {
                parent_path = p;
                break;
            }
        }
        if (parent_path.empty()) continue;

        ThemeInfo pt;
        if (parseIndexTheme(parent_path, pt)) {
            pt.name = parent;
            themes_[parent] = pt;

            for (const auto& p : pt.inherits) {
                if (std::find(loaded.begin(), loaded.end(), p) == loaded.end()) {
                    to_load.push_back(p);
                }
            }
        }
    }

    current_theme_ = theme_name;
    loaded_ = true;

    SDL_Log("KitsuIconTheme: '%s' cargado (%zu temas en la cadena)",
            theme_name.c_str(), themes_.size());
    return true;
}

// ============================================================
// Buscar archivo de icono
// ============================================================
std::string KitsuIconTheme::findIconFile(const std::string& dir,
                                         const std::string& name) const {
    const char* exts[] = { ".png", ".svg", nullptr };
    for (int i = 0; exts[i]; i++) {
        std::string p = dir + "/" + name + exts[i];
        if (fileExists(p)) return p;
    }
    return "";
}

// ============================================================
// Buscar en un tema (una fase)
// ============================================================
std::string KitsuIconTheme::lookupInTheme(const ThemeInfo& theme,
                                          const std::string& name,
                                          int size) const {
    std::string best_match;
    int best_distance = 999999;

    // FASE 1: directorios que coinciden con el tamaño
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

    // FASE 2: cualquier tamaño, elegir el más cercano
    for (const auto& kv : theme.directories) {
        const std::string& subdir = kv.first;
        std::string dir = theme.base_path + "/" + subdir;

        std::string path = findIconFile(dir, name);
        if (!path.empty()) {
            int dist = std::abs(kv.second.size - size);
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
                    int dist = std::abs(kv.second.size - size);
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
// Buscar con herencia
// ============================================================
std::string KitsuIconTheme::lookupWithInheritance(
    const std::string& theme_name,
    const std::string& name,
    int size) const {

    auto it = themes_.find(theme_name);
    if (it == themes_.end()) return "";

    // 1. Buscar en el tema actual
    std::string result = lookupInTheme(it->second, name, size);
    if (!result.empty()) return result;

    // 2. Buscar en los padres
    for (const auto& parent : it->second.inherits) {
        result = lookupWithInheritance(parent, name, size);
        if (!result.empty()) return result;
    }

    return "";
}

// ============================================================
// API pública
// ============================================================
std::string KitsuIconTheme::icon(const std::string& name, int size) const {
    if (!loaded_) return "";
    return lookupWithInheritance(current_theme_, name, size);
}

std::vector<std::string> KitsuIconTheme::availableThemes() const {
    std::vector<std::string> result;

    for (const auto& base : search_paths_) {
        DIR* dir = opendir(base.c_str());
        if (!dir) continue;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] == '.') continue;

            std::string theme_path = base + "/" + entry->d_name;
            std::string index_path = theme_path + "/index.theme";

            if (fileExists(index_path)) {
                std::string name = entry->d_name;
                if (std::find(result.begin(), result.end(), name)
                    == result.end()) {
                    result.push_back(name);
                }
            }
        }
        closedir(dir);
    }

    return result;
}

void KitsuIconTheme::searchPath(const std::string& path) {
    search_paths_.insert(search_paths_.begin(), path);
}

// ============================================================
// KitsuIconCache
// ============================================================
KitsuIconCache& KitsuIconCache::instance() {
    static KitsuIconCache inst;
    return inst;
}

KitsuIconCache::~KitsuIconCache() {
    for (auto& kv : cache_) {
        if (kv.second) SDL_DestroyTexture(kv.second);
    }
    cache_.clear();
}

SDL_Texture* KitsuIconCache::getTexture(SDL_Renderer* renderer,
                                        const std::string& name,
                                        int size,
                                        bool& cached) {
    cached = false;
    if (!renderer) return nullptr;

    // ¿Es el renderer principal?
    bool is_main = false;
    if (g_window) {
        is_main = (renderer == g_window->sdlRenderer());
    }

    CacheKey key{name, size};

    // Buscar en cache (solo renderer principal)
    if (is_main) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            cached = true;
            return it->second;
        }
    }

    // Buscar archivo
    std::string path = KitsuIconTheme::instance().icon(name, size);
    if (path.empty()) {
        if (is_main) cache_[key] = nullptr;
        return nullptr;
    }

    // SVG → PNG temporal
    std::string load_path = path;
    std::string tmp_png;
    if (Utils::endsWith(path, ".svg")) {
        if (commandExists("rsvg-convert")) {
            tmp_png = svgToPng(path, size);
            if (!tmp_png.empty()) load_path = tmp_png;
        }
    }

    // Cargar con SDL_image
    SDL_Surface* surface = IMG_Load(load_path.c_str());

    // Limpiar PNG temporal
    if (!tmp_png.empty()) {
        remove(tmp_png.c_str());
    }

    if (!surface) {
        SDL_Log("KitsuIconCache: error cargando '%s': %s",
                path.c_str(), IMG_GetError());
        if (is_main) cache_[key] = nullptr;
        return nullptr;
    }

    // Escalar si el tamaño no coincide
    if (surface->w != size || surface->h != size) {
        SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(
            0, size, size, 32, SDL_PIXELFORMAT_RGBA32);
        if (scaled) {
            SDL_BlitScaled(surface, nullptr, scaled, nullptr);
            SDL_FreeSurface(surface);
            surface = scaled;
        }
    }

    // Crear textura
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        SDL_Log("KitsuIconCache: error creando textura: %s", SDL_GetError());
        return nullptr;
    }

    // Cachear solo si es el renderer principal
    if (is_main) {
        cache_[key] = texture;
        cached = true;
    }

    return texture;
}

void KitsuIconCache::clear() {
    for (auto& kv : cache_) {
        if (kv.second) SDL_DestroyTexture(kv.second);
    }
    cache_.clear();
}

} // namespace KitsuGui
