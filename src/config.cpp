#include "kitsugui/config.h"
#include "kitsugui/theme.h"
#include "kitsugui/icon_theme.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <chrono>
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

namespace KitsuGui {

// ============================================================
// HELPERS
// ============================================================
static std::string getEnvVar(const char* name) {
    const char* v = getenv(name);
    return v ? std::string(v) : "";
}

static bool dirExists(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
    return S_ISDIR(buffer.st_mode);
}

static bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Crear directorio recursivamente (mkdir -p)
static bool mkdirP(const std::string& path) {
    if (path.empty()) return false;
    
    std::string current;
    std::stringstream ss(path);
    std::string segment;
    bool first = true;
    
    while (std::getline(ss, segment, '/')) {
        if (first) {
            // La primera parte (vacía si empieza por /) es la raíz
            current = segment.empty() ? "/" : segment;
            first = false;
        } else {
            if (current.back() != '/') current += "/";
            current += segment;
        }
        
        if (current.empty() || current == "/") continue;
        
        if (!dirExists(current)) {
            if (mkdir(current.c_str(), 0755) != 0) {
                return false;
            }
        }
    }
    
    return dirExists(path);
}

// ============================================================
// SINGLETON
// ============================================================
KitsuConfig& KitsuConfig::instance() {
    static KitsuConfig inst;
    return inst;
}

KitsuConfig::KitsuConfig() {
    // Determinar el directorio de configuración
    std::string home = getEnvVar("HOME");
    std::string xdg = getEnvVar("XDG_CONFIG_HOME");
    
    if (!xdg.empty()) {
        config_dir = xdg + "/kitsugui";
    } else if (!home.empty()) {
        config_dir = home + "/.config/kitsugui";
    } else {
        // Fallback: directorio local
        config_dir = "./kitsugui_config";
    }
    
    config_path = config_dir + "/config.conf";
}

KitsuConfig::~KitsuConfig() = default;

std::string KitsuConfig::getConfigPath() const {
    return config_path;
}

std::string KitsuConfig::trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

// ============================================================
// CREAR DIRECTORIO
// ============================================================
bool KitsuConfig::ensureConfigDir() {
    if (dirExists(config_dir)) return true;
    return mkdirP(config_dir);
}

// ============================================================
// VALORES POR DEFECTO
// ============================================================
void KitsuConfig::setDefaults() {
    data.clear();
    
    data["appearance"]["theme"] = "KitsuMetro Light";
    data["appearance"]["icon_theme"] = "Papirus-Dark";
    
    data["window"]["width"] = "1000";
    data["window"]["height"] = "700";
    data["window"]["resizable"] = "true";
    
    data["behavior"]["always_render"] = "false";
    data["behavior"]["debug"] = "false";
}

// ============================================================
// CARGAR
// ============================================================
bool KitsuConfig::load() {
    // Si no existe el directorio, crearlo
    if (!ensureConfigDir()) {
        SDL_Log("KitsuConfig: no se pudo crear el directorio %s", config_dir.c_str());
        setDefaults();
        return false;
    }
    
    // Si no existe el archivo, crear con defaults
    if (!fileExists(config_path)) {
        SDL_Log("KitsuConfig: no existe %s, creando con defaults", config_path.c_str());
        setDefaults();
        save();
        return true;
    }
    
    // Leer el archivo
    std::ifstream file(config_path);
    if (!file.is_open()) {
        SDL_Log("KitsuConfig: no se pudo abrir %s", config_path.c_str());
        setDefaults();
        return false;
    }
    
    data.clear();
    
    std::string section;
    std::string line;
    
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#' || line[0] == ';') continue;
        
        // [Section]
        if (line[0] == '[' && line.back() == ']') {
            section = trim(line.substr(1, line.size() - 2));
            continue;
        }
        
        // Key = Value
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        
        if (!section.empty() && !key.empty()) {
            data[section][key] = value;
        }
    }
    
    // Aplicar defaults para claves que falten
    // (cargamos defaults primero en un mapa temporal)
    auto saved_data = data;
    setDefaults();
    
    for (auto& sec : saved_data) {
        for (auto& kv : sec.second) {
            data[sec.first][kv.first] = kv.second;
        }
    }
    
    SDL_Log("KitsuConfig: cargado desde %s", config_path.c_str());
    return true;
}

// ============================================================
// GUARDAR
// ============================================================
bool KitsuConfig::save() {
    if (!ensureConfigDir()) return false;
    
    std::ofstream file(config_path);
    if (!file.is_open()) {
        SDL_Log("KitsuConfig: no se pudo escribir %s", config_path.c_str());
        return false;
    }
    
    file << "# KitsuGui Configuration\n";
    file << "# Edita este archivo para personalizar KitsuGui\n\n";
    
    for (auto& sec : data) {
        file << "[" << sec.first << "]\n";
        for (auto& kv : sec.second) {
            file << kv.first << " = " << kv.second << "\n";
        }
        file << "\n";
    }
    
    SDL_Log("KitsuConfig: guardado en %s", config_path.c_str());
    return true;
}

// ============================================================
// GETTERS
// ============================================================
std::string KitsuConfig::getString(const std::string& section,
                                   const std::string& key,
                                   const std::string& default_value) const {
    auto sec_it = data.find(section);
    if (sec_it == data.end()) return default_value;
    
    auto key_it = sec_it->second.find(key);
    if (key_it == sec_it->second.end()) return default_value;
    
    return key_it->second;
}

int KitsuConfig::getInt(const std::string& section,
                       const std::string& key,
                       int default_value) const {
    std::string v = getString(section, key, "");
    if (v.empty()) return default_value;
    try {
        return std::stoi(v);
    } catch (...) {
        return default_value;
    }
}

bool KitsuConfig::getBool(const std::string& section,
                         const std::string& key,
                         bool default_value) const {
    std::string v = getString(section, key, "");
    if (v.empty()) return default_value;
    return (v == "true" || v == "1" || v == "yes" || v == "on");
}

// ============================================================
// SETTERS
// ============================================================
void KitsuConfig::setString(const std::string& section,
                           const std::string& key,
                           const std::string& value) {
    data[section][key] = value;
}

void KitsuConfig::setInt(const std::string& section,
                        const std::string& key,
                        int value) {
    data[section][key] = std::to_string(value);
}

void KitsuConfig::setBool(const std::string& section,
                         const std::string& key,
                         bool value) {
    data[section][key] = value ? "true" : "false";
}

// ============================================================
// ATAJOS DE APARIENCIA
// ============================================================
std::string KitsuConfig::getThemeName() const {
    return getString("appearance", "theme", "KitsuMetro Light");
}

void KitsuConfig::setThemeName(const std::string& name) {
    setString("appearance", "theme", name);
}

std::string KitsuConfig::getIconThemeName() const {
    return getString("appearance", "icon_theme", "Papirus-Dark");
}

void KitsuConfig::setIconThemeName(const std::string& name) {
    setString("appearance", "icon_theme", name);
}

// ============================================================
// APLICAR TEMA
// ============================================================
void KitsuConfig::applyTheme() {
    std::string theme_name = getThemeName();
    
    // Intentar cargar desde archivo primero
    // Buscar en varios sitios: config_dir/themes, ./data/themes, $PREFIX/share/kitsugui/themes
    std::vector<std::string> search_paths;
    
    search_paths.push_back(config_dir + "/themes");
    search_paths.push_back("data/themes");
    
    std::string prefix = getEnvVar("PREFIX");
    if (!prefix.empty()) {
        search_paths.push_back(prefix + "/share/kitsugui/themes");
    }
    search_paths.push_back("/usr/share/kitsugui/themes");
    search_paths.push_back("/usr/local/share/kitsugui/themes");
    
    // Nombre de archivo probable
    std::string file_name;
    // "KitsuMetro Light" → "kitsu-metro-light.conf"
    for (char c : theme_name) {
        if (c == ' ') file_name += '-';
        else file_name += tolower(c);
    }
    file_name += ".conf";
    
    for (const auto& base : search_paths) {
        std::string path = base + "/" + file_name;
        if (fileExists(path)) {
            if (KitsuTheme::loadFromFile(path)) {
                SDL_Log("KitsuConfig: tema '%s' cargado desde %s",
                        theme_name.c_str(), path.c_str());
                return;
            }
        }
    }
    
    // Fallback: usar los temas hardcodeados
    if (theme_name == "KitsuMetro Light" || theme_name == "KitsuMetroLight") {
        KitsuTheme::apply(KitsuTheme::KitsuMetroLight());
        SDL_Log("KitsuConfig: tema 'KitsuMetro Light' (built-in)");
    } else if (theme_name == "KitsuMetro Dark" || theme_name == "KitsuMetroDark") {
        KitsuTheme::apply(KitsuTheme::KitsuMetroDark());
        SDL_Log("KitsuConfig: tema 'KitsuMetro Dark' (built-in)");
    } else {
        // Intento fallback: si empieza por "dark" o contiene "dark"
        if (theme_name.find("Dark") != std::string::npos ||
            theme_name.find("dark") != std::string::npos) {
            KitsuTheme::apply(KitsuTheme::KitsuMetroDark());
            SDL_Log("KitsuConfig: tema desconocido '%s', usando Dark",
                    theme_name.c_str());
        } else {
            KitsuTheme::apply(KitsuTheme::KitsuMetroLight());
            SDL_Log("KitsuConfig: tema desconocido '%s', usando Light",
                    theme_name.c_str());
        }
    }
}

// ============================================================
// APLICAR ICON THEME
// ============================================================
void KitsuConfig::applyIconTheme() {
    std::string icon_theme_name = getIconThemeName();
    
    if (KitsuIconTheme::instance().load(icon_theme_name)) {
        SDL_Log("KitsuConfig: icon theme '%s' cargado", icon_theme_name.c_str());
        return;
    }
    
    // Fallback: probar con algunos comunes
    const char* fallbacks[] = {
        "Papirus-Dark", "Papirus", "breeze-dark", "breeze",
        "Adwaita", "hicolor", nullptr
    };
    
    for (int i = 0; fallbacks[i] != nullptr; i++) {
        if (KitsuIconTheme::instance().load(fallbacks[i])) {
            SDL_Log("KitsuConfig: icon theme '%s' (fallback)", fallbacks[i]);
            return;
        }
    }
    
    SDL_Log("KitsuConfig: no se pudo cargar ningún icon theme");
}

// ============================================================
// HOT RELOAD (inotify)
// ============================================================
bool KitsuConfig::startWatching() {
    if (watching.load()) {
        SDL_Log("KitsuConfig: ya está observando");
        return true;
    }
    
    // Crear el inotify fd
    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        SDL_Log("KitsuConfig: inotify_init falló: %s", strerror(errno));
        return false;
    }
    
    // Añadir el archivo config.conf al watch
    // Eventos: MODIFY (por si algún editor no dispara CLOSE_WRITE),
    //          CLOSE_WRITE (el estándar para "archivo guardado"),
    //          MOVED_TO (por si el editor hace "guardar como" y mueve),
    //          DELETE_SELF (por si se borra el archivo)
    int wd = inotify_add_watch(
        inotify_fd,
        config_path.c_str(),
        IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF
    );
    
    if (wd < 0) {
        SDL_Log("KitsuConfig: inotify_add_watch falló: %s", strerror(errno));
        close(inotify_fd);
        inotify_fd = -1;
        return false;
    }
    
    // Arrancar el thread
    stop_requested = false;
    watching = true;
    watch_thread = std::thread(&KitsuConfig::watchLoop, this);
    
    SDL_Log("KitsuConfig: observando cambios en %s", config_path.c_str());
    return true;
}

void KitsuConfig::stopWatching() {
    if (!watching.load()) return;
    
    stop_requested = true;
    
    // Despertar al thread con un evento dummy (cierre del fd)
    // El thread está en poll(), así que cerramos el fd para desbloquearlo
    // Pero eso puede causar problemas. Mejor usar poll con timeout.
    
    if (watch_thread.joinable()) {
        watch_thread.join();
    }
    
    watching = false;
    
    if (inotify_fd >= 0) {
        close(inotify_fd);
        inotify_fd = -1;
    }
    
    SDL_Log("KitsuConfig: dejando de observar");
}

void KitsuConfig::watchLoop() {
    constexpr size_t BUF_LEN = 4096;
    char buffer[BUF_LEN];
    
    // Debounce: cuando recibimos un evento, esperamos 200ms
    // antes de recargar. Esto agrupa múltiples eventos seguidos
    // (algunos editores disparan MODIFY + CLOSE_WRITE + MOVED_TO).
    constexpr int DEBOUNCE_MS = 200;
    auto last_event_time = std::chrono::steady_clock::now();
    bool pending_reload = false;
    
    while (!stop_requested.load()) {
        // Poll con timeout corto (para poder chequear stop_requested)
        struct pollfd pfd;
        pfd.fd = inotify_fd;
        pfd.events = POLLIN;
        
        int poll_result = poll(&pfd, 1, 50);   // 50ms timeout
        
        if (poll_result > 0 && (pfd.revents & POLLIN)) {
            // Leer eventos
            ssize_t len = read(inotify_fd, buffer, BUF_LEN);
            if (len > 0) {
                // Recorrer eventos
                ssize_t i = 0;
                while (i < len) {
                    struct inotify_event* event = (struct inotify_event*)&buffer[i];
                    
                    // Filtrar: nos interesan los cambios en el archivo
                    if (event->mask & (IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO)) {
                        pending_reload = true;
                        last_event_time = std::chrono::steady_clock::now();
                    }
                    
                    i += sizeof(struct inotify_event) + event->len;
                }
            }
        }
        
        // ¿Toca recargar?
        if (pending_reload) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_event_time
            ).count();
            
            if (elapsed >= DEBOUNCE_MS) {
                pending_reload = false;
                
                SDL_Log("KitsuConfig: cambio detectado, recargando...");
                
                // Recargar (pero sin tocar watching/thread)
                // load() lee el archivo y actualiza data
                // No podemos llamar a load() directamente porque
                // reiniciaría todo. Mejor releer el archivo:
                {
                    std::ifstream file(config_path);
                    if (file.is_open()) {
                        std::unordered_map<std::string,
                                          std::unordered_map<std::string, std::string>> new_data;
                        
                        std::string section;
                        std::string line;
                        
                        while (std::getline(file, line)) {
                            line = trim(line);
                            if (line.empty()) continue;
                            if (line[0] == '#' || line[0] == ';') continue;
                            
                            if (line[0] == '[' && line.back() == ']') {
                                section = trim(line.substr(1, line.size() - 2));
                                continue;
                            }
                            
                            size_t eq = line.find('=');
                            if (eq == std::string::npos) continue;
                            
                            std::string key = trim(line.substr(0, eq));
                            std::string value = trim(line.substr(eq + 1));
                            
                            if (!section.empty() && !key.empty()) {
                                new_data[section][key] = value;
                            }
                        }
                        
                        // Copiar defaults y luego sobrescribir
                        auto defaults = data;   // ya teníamos defaults
                        // Mejor: aplicar los defaults del sistema
                        setDefaults();
                        for (auto& sec : new_data) {
                            for (auto& kv : sec.second) {
                                data[sec.first][kv.first] = kv.second;
                            }
                        }
                        
                        SDL_Log("KitsuConfig: config recargada");
                    }
                }
                
                // Aplicar el nuevo tema
                applyTheme();
                
                // Notificar a la app
                if (on_reload) {
                    on_reload();
                }
            }
        }
    }
}

} // namespace KitsuGui
