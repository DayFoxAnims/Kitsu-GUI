#include "kitsugui/fonts.h"
#include "kitsugui/utils.h"
#include <SDL2/SDL.h>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <sys/stat.h>

namespace KitsuGui {

namespace {

// ============================================================
// Estado interno
// ============================================================
struct FontEntry {
    TTF_Font* font = nullptr;
    std::string path;
    int size = 0;
};

std::unordered_map<std::string, FontEntry> g_fonts;
std::vector<std::string> g_search_paths;
bool g_initialized = false;
int  g_base_size = 15;

// ============================================================
// Helpers
// ============================================================
bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

std::string getEnv(const char* name) {
    const char* v = getenv(name);
    return v ? std::string(v) : "";
}

// Directorios donde buscar fuentes TTF
void buildSearchPaths() {
    if (!g_search_paths.empty()) return;

    std::string home   = getEnv("HOME");
    std::string prefix = getEnv("PREFIX");
    if (prefix.empty()) prefix = "/usr";

    // Rutas locales del proyecto primero
    g_search_paths.push_back("data/fonts");
    g_search_paths.push_back("../data/fonts");
    g_search_paths.push_back("../../data/fonts");

    // Rutas del sistema
    if (!home.empty()) {
        g_search_paths.push_back(home + "/.local/share/fonts");
        g_search_paths.push_back(home + "/.fonts");
    }
    g_search_paths.push_back(prefix + "/share/fonts");
    g_search_paths.push_back("/usr/share/fonts");
    g_search_paths.push_back("/usr/local/share/fonts");
}

// Nombres de fuentes candidatas por rol (en orden de preferencia)
const std::vector<std::string>& candidateFiles(const std::string& role) {
    static const std::vector<std::string> sans = {
        "DejaVuSans.ttf",
        "NotoSans-Regular.ttf",
        "LiberationSans-Regular.ttf",
        "Roboto-Regular.ttf",
        "OpenSans-Regular.ttf",
        "Arial.ttf"
    };
    static const std::vector<std::string> mono = {
        "DejaVuSansMono.ttf",
        "NotoSansMono-Regular.ttf",
        "LiberationMono-Regular.ttf",
        "RobotoMono-Regular.ttf",
        "CourierNew.ttf"
    };

    if (role == "mono") return mono;
    return sans;
}

// Recorre los search_paths recursivamente buscando un archivo
// (búsqueda superficial: 2 niveles de profundidad)
std::string findFontFile(const std::string& filename) {
    for (const auto& base : g_search_paths) {
        std::string direct = base + "/" + filename;
        if (fileExists(direct)) return direct;

        // Subdirectorios comunes
        const char* subs[] = {
            "truetype", "truetype/dejavu", "truetype/noto",
            "truetype/liberation", "TTF", "ttf", nullptr
        };
        for (int i = 0; subs[i] != nullptr; i++) {
            std::string p = base + "/" + subs[i] + "/" + filename;
            if (fileExists(p)) return p;
        }
    }
    return "";
}

// Carga una fuente por rol, buscando automáticamente.
TTF_Font* loadRole(const std::string& role, int size) {
    const auto& candidates = candidateFiles(role);

    for (const auto& file : candidates) {
        std::string path = findFontFile(file);
        if (!path.empty()) {
            TTF_Font* f = TTF_OpenFont(path.c_str(), size);
            if (f) {
                SDL_Log("KitsuFonts: '%s' → %s (%dpx)",
                        role.c_str(), path.c_str(), size);
                return f;
            }
        }
    }

    // Fallback: usar la fuente normal para todos los roles
    SDL_Log("KitsuFonts: no se encontró fuente para '%s', "
            "usando 'normal' como fallback", role.c_str());
    return nullptr;
}

// ============================================================
// Inicialización
// ============================================================
void ensureInitialized() {
    if (g_initialized) return;
    g_initialized = true;

    if (!TTF_WasInit()) {
        if (TTF_Init() < 0) {
            SDL_Log("KitsuFonts: TTF_Init falló: %s", TTF_GetError());
            return;
        }
    }

    buildSearchPaths();

    // Cargar fuentes por rol
    // Los tamaños son proporcionales al base
    struct RoleSpec { const char* role; float scale; };
    const RoleSpec specs[] = {
        { "small",  0.85f },
        { "normal", 1.00f },
        { "large",  1.20f },
        { "title",  1.45f },
        { "mono",   1.00f },
    };

    for (const auto& spec : specs) {
        int size = (int)(g_base_size * spec.scale);
        if (size < 8) size = 8;

        TTF_Font* f = loadRole(spec.role, size);
        g_fonts[spec.role] = { f, "", size };
    }

    // Si normal falló, la app no tiene fuentes → error crítico
    if (!g_fonts["normal"].font) {
        SDL_Log("KitsuFonts: ADVERTENCIA — no hay fuente 'normal'. "
                "El texto no se renderizará.");
    }

    // Heredar fallback: si algún rol no cargó, usa normal
    for (auto& kv : g_fonts) {
        if (!kv.second.font && kv.first != "normal") {
            kv.second.font = g_fonts["normal"].font;
        }
    }
}

} // namespace

// ============================================================
// API pública
// ============================================================
void KitsuFonts::init() {
    ensureInitialized();
}

void KitsuFonts::shutdown() {
    for (auto& kv : g_fonts) {
        // Solo destruir si NO es el fallback compartido
        if (kv.second.font && kv.second.font != g_fonts["normal"].font) {
            TTF_CloseFont(kv.second.font);
        }
    }
    // Destruir normal al final
    if (g_fonts["normal"].font) {
        TTF_CloseFont(g_fonts["normal"].font);
    }
    g_fonts.clear();
    g_initialized = false;
}

TTF_Font* KitsuFonts::small()  { ensureInitialized(); return g_fonts["small"].font; }
TTF_Font* KitsuFonts::normal() { ensureInitialized(); return g_fonts["normal"].font; }
TTF_Font* KitsuFonts::large()  { ensureInitialized(); return g_fonts["large"].font; }
TTF_Font* KitsuFonts::title()  { ensureInitialized(); return g_fonts["title"].font; }
TTF_Font* KitsuFonts::mono()   { ensureInitialized(); return g_fonts["mono"].font; }

bool KitsuFonts::load(const std::string& role,
                      const std::string& path,
                      int size) {
    ensureInitialized();

    if (!fileExists(path)) {
        SDL_Log("KitsuFonts: archivo no existe: %s", path.c_str());
        return false;
    }

    TTF_Font* f = TTF_OpenFont(path.c_str(), size);
    if (!f) {
        SDL_Log("KitsuFonts: error abriendo %s: %s",
                path.c_str(), TTF_GetError());
        return false;
    }

    // Cerrar la anterior si existía y no era compartida
    auto it = g_fonts.find(role);
    if (it != g_fonts.end() && it->second.font &&
        it->second.font != g_fonts["normal"].font) {
        TTF_CloseFont(it->second.font);
    }

    g_fonts[role] = { f, path, size };
    return true;
}

void KitsuFonts::setBaseSize(int size) {
    if (size < 8) size = 8;
    if (size == g_base_size) return;
    g_base_size = size;

    // Recargar todo
    if (g_initialized) {
        // Guardar paths custom para no perderlos
        auto saved = g_fonts;
        g_fonts.clear();

        shutdown();
        g_base_size = size;
        ensureInitialized();
    }
}

int KitsuFonts::getBaseSize() {
    return g_base_size;
}

void KitsuFonts::addSearchPath(const std::string& path) {
    buildSearchPaths();
    g_search_paths.insert(g_search_paths.begin(), path);
}

} // namespace KitsuGui
