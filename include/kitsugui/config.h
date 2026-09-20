#ifndef KITSUGUI_CONFIG_H
#define KITSUGUI_CONFIG_H

#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>

namespace KitsuGui {

class KitsuConfig {
public:
    // ===== Singleton =====
    static KitsuConfig& instance();
    
    // ===== Cargar/guardar =====
    bool load();
    bool save();
    
    // ===== Directorio de configuración =====
    std::string getConfigDir() const { return config_dir; }
    std::string getConfigPath() const;
    
    // ===== Leer valores =====
    std::string getString(const std::string& section,
                         const std::string& key,
                         const std::string& default_value = "") const;
    int getInt(const std::string& section,
              const std::string& key,
              int default_value = 0) const;
    bool getBool(const std::string& section,
                const std::string& key,
                bool default_value = false) const;
    
    // ===== Escribir valores =====
    void setString(const std::string& section,
                  const std::string& key,
                  const std::string& value);
    void setInt(const std::string& section,
               const std::string& key,
               int value);
    void setBool(const std::string& section,
                const std::string& key,
                bool value);
    
    // ===== Atajos de apariencia =====
    std::string getThemeName() const;
    void setThemeName(const std::string& name);
    std::string getIconThemeName() const;
    void setIconThemeName(const std::string& name);
    
    // ===== Aplicar tema/iconos =====
    void applyTheme();
    void applyIconTheme();
    
    // ============================================================
    // HOT RELOAD
    // ============================================================
    
    // Iniciar el watcher. Cuando el archivo config.conf cambie,
    // se recarga automáticamente y se llama al callback.
    bool startWatching();
    void stopWatching();
    bool isWatching() const { return watching.load(); }
    
    // Callback que se llama después de recargar el tema.
    // Útil para que la app sepa que tiene que redibujar.
    void setOnReload(std::function<void()> cb) { on_reload = cb; }
    
private:
    KitsuConfig();
    ~KitsuConfig();
    KitsuConfig(const KitsuConfig&) = delete;
    KitsuConfig& operator=(const KitsuConfig&) = delete;
    
    std::string config_dir;
    std::string config_path;
    
    std::unordered_map<std::string,
                      std::unordered_map<std::string, std::string>> data;
    
    // ===== Watch thread =====
    std::thread watch_thread;
    std::atomic<bool> watching{false};
    std::atomic<bool> stop_requested{false};
    std::function<void()> on_reload;
    
    // File descriptor de inotify (-1 = no iniciado)
    int inotify_fd = -1;
    
    // Thread function
    void watchLoop();
    
    bool ensureConfigDir();
    void setDefaults();
    
    static std::string trim(const std::string& s);
};

} // namespace KitsuGui

#endif
