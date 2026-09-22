#ifndef KITSUGUI_H
#define KITSUGUI_H

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <algorithm>

// ===== NÚCLEO =====
#include "kitsugui/utils.h"
#include "kitsugui/color.h"
#include "kitsugui/theme.h"
#include "kitsugui/fonts.h"
#include "kitsugui/config.h"
#include "kitsugui/renderer.h"
#include "kitsugui/widget.h"
#include "kitsugui/shapes.h"

// ===== CONTENEDORES =====
#include "kitsugui/box.h"
#include "kitsugui/panel.h"
#include "kitsugui/scroll_view.h"
#include "kitsugui/viewport.h"

// ===== WIDGETS =====
#include "kitsugui/button.h"
#include "kitsugui/label.h"
#include "kitsugui/check_box.h"
#include "kitsugui/switch.h"
#include "kitsugui/slider.h"
#include "kitsugui/progress_bar.h"
#include "kitsugui/radio.h"
#include "kitsugui/text_input.h"
#include "kitsugui/dropdown.h"
#include "kitsugui/icon.h"
#include "kitsugui/fps.h"

// ===== SISTEMAS =====
#include "kitsugui/icon_theme.h"
#include "kitsugui/context_menu.h"
#include "kitsugui/popup.h"

// ===== VENTANA =====
#include "kitsugui/window.h"

namespace KitsuGui {

// Función global del bucle principal
void run();

} // namespace KitsuGui

#endif
