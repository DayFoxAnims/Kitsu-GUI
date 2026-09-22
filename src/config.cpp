#include "kitsugui/config.h"
#include "kitsugui/theme.h"
#include "kitsugui/icon_theme.h"
#include "kitsugui/utils.h"
#include <SDL2/SDL.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>

namespace KitsuGui {

// ============================================================
// Helpers
// ============================================================
namespace {

std::string env(const char* name) {
    const char* v = getenv(name);
    return v ? std::string(v) : "";
}

bool dirExists(const std::string& path) {
    struct stat b;
    if (stat(path.c_str(), &b) != 0) return false;
    return S_ISDIR(b.st_mode);
}

bool fileExists(const std::string& path) {
    struct stat b;
    return (stat(path.c_str(), &b) == 0);
}

bool mkdirP(const std::string& path) {
    if (path.empty()) return false;

    std::string current;
    std::stringstream ss(path);
    std::string segment;
    bool first = true;

    while (std::getline(ss, segment, '/')) {
        if (first) {
            current = segment.empty() ? "/" : segment;
            first = false;
        } else {
            if (current.back() != '/') current += "/";
            current += segment;
        }
        if (current.empty() || current == "/") continue;
        if (!dirExists(current)) {
            if (mkdir(current.c_str(), 0755) != 0) return false;
        }
    }
    return dirExists(path);
}

} // namespace

// ============================================================
// Singleton
// ============================================================
KitsuConfig& KitsuConfig::instance() {
    static KitsuConfig inst;
    return inst;
}

KitsuConfig::KitsuConfig() {
    std::string home = env("HOME");
    std::string xdg  = env("XDG_CONFIG_HOME");

    if (!xdg.empty()) {
        config_dir_ = xdg + "/kitsugui";
    } else if (!home.empty()) {
        config_dir_ = home + "/.config/kitsugui";
    } else {
        config_dir_ = "./kitsugui_config";
    }
    config_path_ = config_dir_ + "/config.conf";
}

KitsuConfig::~KitsuConfig() {
    unwatch();   // ← arregla bug #14
}

// ============================================================
// Directorio
// ============================================================
bool KitsuConfig::ensureConfigDir() {
    if (dirExists(config_dir_)) return true;
    return mkdirP(config_dir_);
}

// ============================================================
// Defaults
// ============================================================
void KitsuConfig::setDefaults() {
    data_.clear();
    data_["appearance"]["theme"]      = "KitsuMetro Light";
    data_["appearance"]["icon_theme"] = "Papirus-Dark";
    data_["window"]["width"]          = "1000";
    data_["window"]["height"]         = "700";
    data_["window"]["resizable"]      = "true";
    data_["behavior"]["always_render"] = "false";
    data_["behavior"]["debug"]        = "false";
}

// ============================================================
// Cargar
// ============================================================
bool KitsuConfig::load() {
    if (!ensureConfigDir()) {
        SDL_Log("KitsuConfig: no se pudo crear %s", config_dir_.c_str());
        setDefaults();
        return false;
    }

    if (!fileExists(config_path_)) {
        SDL_Log("KitsuConfig: creando %s con defaults", config_path_.c_str());
        setDefaults();
        save();
        return true;
    }

    std::ifstream file(config_path_);
    if (!file.is_open()) {
        SDL_Log("KitsuConfig: no se pudo abrir %s", config_path_.c_str());
        setDefaults();
        return false;
    }

    data_.clear();

    std::string section;
    std::string line;
    while (std::getline(file, line)) {
        line = Utils::trim(line);
        if (line.empty()) continue;
        if (line[0] == '#' || line[0] == ';') continue;

        if (line[0] == '[' && line.back() == ']') {
            section = Utils::trim(line.substr(1, line.size() - 2));
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = Utils::trim(line.substr(0, eq));
        std::string value = Utils::trim(line.substr(eq + 1));

        if (!section.empty() && !key.empty()) {
            data_[section][key] = value;
        }
    }

    SDL_Log("KitsuConfig: cargado desde %s", config_path_.c_str());
    return true;
}

// ============================================================
// Guardar
// ============================================================
bool KitsuConfig::save() {
    if (!ensureConfigDir()) return false;

    std::ofstream file(config_path_);
    if (!file.is_open()) return false;

    file << "# KitsuGui Configuration\n\n";
    for (auto& sec : data_) {
        file << "[" << sec.first << "]\n";
        for (auto& kv : sec.second) {
            file << kv.first << " = " << kv.second << "\n";
        }
        file << "\n";
    }
    return true;
}

// ============================================================
// Getters / setters
// ============================================================
std::string KitsuConfig::getString(const std::string& section,
                                   const std::string& key,
                                   const std::string& fallback) const {
    auto sit = data_.find(section);
    if (sit == data_.end()) return fallback;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return fallback;
    return kit->second;
}

int KitsuConfig::getInt(const std::string& section,
                       const std::string& key,
                       int fallback) const {
    std::string v = getString(section, key, "");
    if (v.empty()) return fallback;
    return Utils::safeStoi(v, fallback);
}

bool KitsuConfig::getBool(const std::string& section,
                         const std::string& key,
                         bool fallback) const {
    std::string v = getString(section, key, "");
    if (v.empty()) return fallback;
    return Utils::safeStob(v, fallback);
}

void KitsuConfig::setString(const std::string& section,
                           const std::string& key,
                           const std::string& value) {
    data_[section][key] = value;
}

void KitsuConfig::setInt(const std::string& section,
                        const std::string& key, int value) {
    data_[section][key] = std::to_string(value);
}

void KitsuConfig::setBool(const std::string& section,
                         const std::string& key, bool value) {
    data_[section][key] = value ? "true" : "false";
}

// ============================================================
// Atajos
// ============================================================
std::string KitsuConfig::themeName() const {
    return getString("appearance", "theme", "KitsuMetro Light");
}

void KitsuConfig::themeName(const std::string& name) {
    setString("appearance", "theme", name);
}

std::string KitsuConfig::iconThemeName() const {
    return getString("appearance", "icon_theme", "Papirus-Dark");
}

void KitsuConfig::iconThemeName(const std::string& name) {
    setString("appearance", "icon_theme", name);
}

// ============================================================
// applyTheme
// ============================================================
void KitsuConfig::applyTheme() {
    std::string name = themeName();
    std::string file_name = Utils::toLower(name);
    for (char& c : file_name) {
        if (c == ' ') c = '-';   // ← fix tolower con cast ya hecho en Utils
    }
    file_name += ".conf";

    std::vector<std::string> paths = {
        config_dir_ + "/themes/" + file_name,
        "data/themes/" + file_name,
        "../data/themes/" + file_name,
    };

    std::string prefix = env("PREFIX");
    if (!prefix.empty()) {
        paths.push_back(prefix + "/share/kitsugui/themes/" + file_name);
    }
    paths.push_back("/usr/share/kitsugui/themes/" + file_name);
    paths.push_back("/usr/local/share/kitsugui/themes/" + file_name);

    for (const auto& p : paths) {
        if (fileExists(p)) {
            if (KitsuTheme::loadFromFile(p)) {
                SDL_Log("KitsuConfig: tema '%s' desde %s",
                        name.c_str(), p.c_str());
                return;
            }
        }
    }

    // Fallback a los built-in
    if (name == "KitsuMetro Light" || name == "KitsuMetroLight") {
        KitsuTheme::set(KitsuTheme::KitsuMetroLight());
        SDL_Log("KitsuConfig: tema '%s' (built-in)", name.c_str());
    } else if (name == "KitsuMetro Dark" || name == "KitsuMetroDark") {
        KitsuTheme::set(KitsuTheme::KitsuMetroDark());
        SDL_Log("KitsuConfig: tema '%s' (built-in)", name.c_str());
    } else {
        bool dark = Utils::toLower(name).find("dark") != std::string::npos;
        KitsuTheme::set(dark ? KitsuTheme::KitsuMetroDark()
                             : KitsuTheme::KitsuMetroLight());
        SDL_Log("KitsuConfig: tema desconocido '%s', usando %s",
                name.c_str(), dark ? "Dark" : "Light");
    }
}

// ============================================================
// applyIconTheme
// ============================================================
void KitsuConfig::applyIconTheme() {
    std::string name = iconThemeName();
    if (KitsuIconTheme::instance().load(name)) {
        SDL_Log("KitsuConfig: icon theme '%s'", name.c_str());
        return;
    }

    const char* fallbacks[] = {
        "Papirus-Dark", "Papirus", "breeze-dark", "breeze",
        "Adwaita", "hicolor", nullptr
    };
    for (int i = 0; fallbacks[i]; i++) {
        if (KitsuIconTheme::instance().load(fallbacks[i])) {
            SDL_Log("KitsuConfig: icon theme fallback '%s'", fallbacks[i]);
            return;
        }
    }
    SDL_Log("KitsuConfig: no se pudo cargar ningún icon theme");
}

// ============================================================
// Hot reload
// ============================================================
bool KitsuConfig::watch() {
    if (watching_.load()) return true;

    inotify_fd_ = inotify_init();
    if (inotify_fd_ < 0) {
        SDL_Log("KitsuConfig: inotify_init falló: %s", strerror(errno));
        return false;
    }

    int wd = inotify_add_watch(
        inotify_fd_,
        config_path_.c_str(),
        IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO | IN_DELETE_SELF
    );

    if (wd < 0) {
        SDL_Log("KitsuConfig: inotify_add_watch falló: %s", strerror(errno));
        close(inotify_fd_);
        inotify_fd_ = -1;
        return false;
    }

    stop_requested_ = false;
    watching_ = true;
    watch_thread_ = std::thread(&KitsuConfig::watchLoop, this);
    SDL_Log("KitsuConfig: observando %s", config_path_.c_str());
    return true;
}

void KitsuConfig::unwatch() {
    if (!watching_.load()) return;
    stop_requested_ = true;
    if (watch_thread_.joinable()) watch_thread_.join();
    watching_ = false;
    if (inotify_fd_ >= 0) {
        close(inotify_fd_);
        inotify_fd_ = -1;
    }
}

void KitsuConfig::watchLoop() {
    constexpr size_t BUF_LEN = 4096;
    char buffer[BUF_LEN];
    constexpr int DEBOUNCE_MS = 200;

    auto last_event = std::chrono::steady_clock::now();
    bool pending = false;

    while (!stop_requested_.load()) {
        struct pollfd pfd;
        pfd.fd = inotify_fd_;
        pfd.events = POLLIN;

        int result = poll(&pfd, 1, 50);

        if (result > 0 && (pfd.revents & POLLIN)) {
            ssize_t len = read(inotify_fd_, buffer, BUF_LEN);
            if (len > 0) {
                ssize_t i = 0;
                while (i < len) {
                    auto* ev = (struct inotify_event*)&buffer[i];
                    if (ev->mask & (IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO)) {
                        pending = true;
                        last_event = std::chrono::steady_clock::now();
                    }
                    i += sizeof(struct inotify_event) + ev->len;
                }
            }
        }

        if (pending) {
            auto now = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_event).count();

            if (ms >= DEBOUNCE_MS) {
                pending = false;
                SDL_Log("KitsuConfig: cambio detectado, recargando...");
                load();
                applyTheme();
                if (on_reload_) on_reload_();
            }
        }
    }
}

} // namespace KitsuGui
