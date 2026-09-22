#ifndef KITSUGUI_FONTS_H
#define KITSUGUI_FONTS_H

#include <string>
#include <SDL2/SDL_ttf.h>

namespace KitsuGui {

// ============================================================
// KitsuFonts — Gestor global de fuentes
// ============================================================
// Carga automáticamente las fuentes básicas al iniciar la app.
// El usuario NO necesita gestionar TTF_Font* manualmente.
//
// Uso típico:
//   KitsuFonts::init();                       // una vez al arrancar
//   auto* f = KitsuFonts::normal();           // la fuente activa
//   auto* t = KitsuFonts::title();            // título
//
// El usuario puede sobreescribir cualquiera:
//   KitsuFonts::load("normal", "/path/to/font.ttf", 16);
// ============================================================
class KitsuFonts {
public:
    // ===== Ciclo de vida =====
    // Carga las fuentes por defecto desde el sistema o desde data/fonts.
    // Es idempotente: llamarla varias veces no recarga.
    static void init();
    static void shutdown();

    // ===== Fuentes por rol =====
    static TTF_Font* small();
    static TTF_Font* normal();
    static TTF_Font* large();
    static TTF_Font* title();
    static TTF_Font* mono();

    // ===== Carga manual =====
    // role: "small", "normal", "large", "title", "mono"
    static bool load(const std::string& role,
                     const std::string& path,
                     int size);

    // ===== Configuración global =====
    // Cambia el tamaño base. Las demás escalan proporcionalmente.
    static void setBaseSize(int size);
    static int  getBaseSize();

    // ===== Búsqueda automática =====
    // Añade un directorio donde buscar fuentes TTF (al principio).
    static void addSearchPath(const std::string& path);

private:
    KitsuFonts() = delete;
    ~KitsuFonts() = delete;
};

} // namespace KitsuGui

#endif
