#ifndef KITSUGUI_ICON_THEME_H
#define KITSUGUI_ICON_THEME_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace KitsuGui {

// ============================================================
// KitsuIconTheme: gestiona los temas de iconos XDG
// ============================================================
class KitsuIconTheme {
public:
    // ===== Singleton =====
    static KitsuIconTheme& instance();
    
    // ===== Cargar tema =====
    // Busca en: ~/.icons, ~/.local/share/icons, $PREFIX/share/icons,
    // /usr/share/icons, /usr/local/share/icons, data/icons
    bool load(const std::string& theme_name);
    
    // Tema actual
    const std::string& getCurrentTheme() const { return current_theme; }
    bool isLoaded() const { return loaded; }
    
    // ===== Buscar icono =====
    // Devuelve la ruta del archivo (PNG/SVG) o "" si no se encuentra.
    // size = tamaño nominal en píxeles (16, 22, 24, 32, 48, 64, 128, 256)
    std::string findIcon(const std::string& name, int size = 24) const;
    
    // ===== Listar temas disponibles =====
    std::vector<std::string> listAvailableThemes() const;
    
    // ===== Rutas de búsqueda =====
    // Añadir un directorio custom donde buscar temas (al principio)
    void addSearchPath(const std::string& path);
    
private:
    KitsuIconTheme();
    ~KitsuIconTheme();
    KitsuIconTheme(const KitsuIconTheme&) = delete;
    KitsuIconTheme& operator=(const KitsuIconTheme&) = delete;
    
    // ===== Estructuras internas =====
    struct ThemeInfo {
        std::string name;
        std::string comment;
        std::vector<std::string> inherits;   // temas padre
        std::string base_path;                // ruta absoluta
        
        struct DirInfo {
            int size = 0;
            std::string type = "Threshold";   // "Fixed", "Scalable", "Threshold"
            int scale = 1;
            int min_size = 0;
            int max_size = 0;
            int threshold = 2;
        };
        // subdirectory → info (ej: "16x16/actions" → DirInfo{16, ...})
        std::unordered_map<std::string, DirInfo> directories;
    };
    
    // Parsear index.theme
    bool parseIndexTheme(const std::string& path, ThemeInfo& out);
    
    // Buscar un icono en un tema específico (una fase)
    std::string lookupInTheme(const ThemeInfo& theme,
                              const std::string& name, int size) const;
    
    // Buscar recursivamente en la cadena de herencia
    std::string lookupWithInheritance(const std::string& theme_name,
                                      const std::string& name, int size) const;
    
    // Buscar archivo con extensiones (.png, .svg)
    std::string findIconFile(const std::string& dir,
                             const std::string& name) const;
    
    // ===== Estado =====
    bool loaded = false;
    std::string current_theme;
    std::vector<std::string> search_paths;
    std::unordered_map<std::string, ThemeInfo> themes;   // nombre → info
};

// ============================================================
// KitsuIconCache: cachea texturas para el renderer principal
// ============================================================
// - Para el renderer principal (ventana): cachea las texturas.
//   El llamador NO debe destruirlas.
// - Para otros renderers (popups): carga fresco sin cachear.
//   El llamador SÍ debe destruir la textura cuando termine.
// ============================================================
class KitsuIconCache {
public:
    static KitsuIconCache& instance();
    
    // ===== Devuelve la textura del icono =====
    // - Si el renderer es el principal: cachea y devuelve la textura cacheada
    //   (cached = true → NO destruir)
    // - Si el renderer NO es el principal (popup): carga fresco, SIN cachear
    //   (cached = false → el llamador DEBE destruir la textura)
    SDL_Texture* getTexture(SDL_Renderer* renderer,
                           const std::string& name,
                           int size,
                           bool& cached);
    
    // ===== Limpiar el cache del renderer principal =====
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
            return std::hash<std::string>()(k.name) ^ std::hash<int>()(k.size);
        }
    };
    
    // Cache solo para el renderer principal
    std::unordered_map<CacheKey, SDL_Texture*, CacheKeyHash> cache;
};

} // namespace KitsuGui

#endif
