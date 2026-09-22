#ifndef KITSUGUI_ICON_THEME_H
#define KITSUGUI_ICON_THEME_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace KitsuGui {

// ============================================================
// KitsuIconTheme — gestión de temas de iconos XDG
// ============================================================
// Uso:
//   KitsuIconTheme::instance().load("Papirus-Dark");
//   std::string path = KitsuIconTheme::instance().icon("document-save", 24);
//
// Busca en (en orden):
//   ~/.icons, ~/.local/share/icons, $PREFIX/share/icons,
//   /usr/share/icons, /usr/local/share/icons, data/icons
//
// Soporta herencia de temas (Inherits= en index.theme) y
// varios tipos de directorio (Fixed, Scalable, Threshold).
// ============================================================
class KitsuIconTheme {
public:
    static KitsuIconTheme& instance();

    // ===== Cargar tema =====
    bool load(const std::string& theme_name);

    // ===== Estado =====
    const std::string& currentTheme() const { return current_theme_; }
    bool isLoaded() const { return loaded_; }

    // ===== Buscar icono =====
    // Devuelve la ruta del archivo (PNG/SVG) o "" si no se encuentra.
    // size = tamaño nominal en píxeles (16, 22, 24, 32, 48, 64, 128, 256).
    std::string icon(const std::string& name, int size = 24) const;

    // ===== Listar temas =====
    std::vector<std::string> availableThemes() const;

    // ===== Rutas de búsqueda =====
    void searchPath(const std::string& path);

private:
    KitsuIconTheme();
    ~KitsuIconTheme();
    KitsuIconTheme(const KitsuIconTheme&) = delete;
    KitsuIconTheme& operator=(const KitsuIconTheme&) = delete;

    // ===== Estructuras internas =====
    struct ThemeInfo {
        std::string name;
        std::string comment;
        std::vector<std::string> inherits;
        std::string base_path;

        struct DirInfo {
            int size = 0;
            std::string type = "Threshold";
            int scale = 1;
            int min_size = 0;
            int max_size = 0;
            int threshold = 2;
        };
        std::unordered_map<std::string, DirInfo> directories;
    };

    bool parseIndexTheme(const std::string& path, ThemeInfo& out);

    std::string lookupInTheme(const ThemeInfo& theme,
                              const std::string& name, int size) const;

    std::string lookupWithInheritance(const std::string& theme_name,
                                      const std::string& name,
                                      int size) const;

    std::string findIconFile(const std::string& dir,
                             const std::string& name) const;

    // ===== Estado =====
    bool loaded_ = false;
    std::string current_theme_;
    std::vector<std::string> search_paths_;
    std::unordered_map<std::string, ThemeInfo> themes_;
};

// ============================================================
// KitsuIconCache — cache de texturas para el renderer principal
// ============================================================
// Estrategia:
//   - Renderer principal (ventana principal): se cachea.
//     El llamador NO debe destruir la textura.
//   - Otros renderers (popups): se carga fresco SIN cachear.
//     El llamador SÍ debe destruir la textura.
//
// El parámetro 'cached' indica cuál es el caso.
// ============================================================
class KitsuIconCache {
public:
    static KitsuIconCache& instance();

    SDL_Texture* getTexture(SDL_Renderer* renderer,
                           const std::string& name,
                           int size,
                           bool& cached);

    void clear();
    ~KitsuIconCache();

private:
    KitsuIconCache() = default;
    KitsuIconCache(const KitsuIconCache&) = delete;
    KitsuIconCache& operator=(const KitsuIconCache&) = delete;

    struct CacheKey {
        std::string name;
        int size;
        bool operator==(const CacheKey& o) const {
            return name == o.name && size == o.size;
        }
    };

    struct CacheKeyHash {
        size_t operator()(const CacheKey& k) const {
            return std::hash<std::string>()(k.name) ^
                   (std::hash<int>()(k.size) << 1);
        }
    };

    std::unordered_map<CacheKey, SDL_Texture*, CacheKeyHash> cache_;
};

} // namespace KitsuGui

#endif
