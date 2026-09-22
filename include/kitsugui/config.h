#ifndef KITSUGUI_CONFIG_H
#define KITSUGUI_CONFIG_H

#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>

namespace KitsuGui {

// ============================================================
// KitsuConfig — Singleton de configuración
// ============================================================
// Uso:
//   auto& cfg = KitsuConfig::instance();
//   cfg.load();           // carga config.conf (crea si no existe)
//   cfg.applyTheme();     // aplica el tema configurado
//   cfg.watch();          // hot reload
// ============================================================
class KitsuConfig {
public:
    static KitsuConfig& instance();

    // ===== Cargar/guardar =====
    bool load();
    bool save();

    // ===== Ubicación =====
    const std::string& configDir()  const { return config_dir_; }
    const std::string& configPath() const { return config_path_; }

    // ===== Lectura =====
    std::string getString(const std::string& section,
                         const std::string& key,
                         const std::string& fallback = "") const;
    int  getInt(const std::string& section,
               const std::string& key,
               int fallback = 0) const;
    bool getBool(const std::string& section,
                const std::string& key,
                bool fallback = false) const;

    // ===== Escritura =====
    void setString(const std::string& section,
                  const std::string& key,
                  const std::string& value);
    void setInt(const std::string& section,
               const std::string& key, int value);
    void setBool(const std::string& section,
                const std::string& key, bool value);

    // ===== Atajos =====
    std::string themeName() const;
    void themeName(const std::string& name);

    std::string iconThemeName() const;
    void iconThemeName(const std::string& name);

    // ===== Aplicar =====
    void applyTheme();
    void applyIconTheme();

    // ===== Hot reload =====
    bool watch();
    void unwatch();
    bool isWatching() const { return watching_.load(); }

    void onReload(std::function<void()> cb) { on_reload_ = std::move(cb); }

private:
    KitsuConfig();
    ~KitsuConfig();
    KitsuConfig(const KitsuConfig&) = delete;
    KitsuConfig& operator=(const KitsuConfig&) = delete;

    std::string config_dir_;
    std::string config_path_;

    std::unordered_map<std::string,
                      std::unordered_map<std::string, std::string>> data_;

    std::thread watch_thread_;
    std::atomic<bool> watching_{false};
    std::atomic<bool> stop_requested_{false};
    std::function<void()> on_reload_;
    int inotify_fd_ = -1;

    void watchLoop();
    bool ensureConfigDir();
    void setDefaults();
};

} // namespace KitsuGui

#endif
