#include <kitsugui.h>
using namespace KitsuGui;

int main() {
    KitsuWindow win(900, 600, "Test · Dos paneles con scroll");

    // ========================================================
    // HBox que reparte la ventana en 2 columnas
    // ========================================================
    auto* hbox = new KitsuHBox();
    hbox->size(-1, -1);              // ← llena la ventana
    hbox->spacing(10)->padding(10);
    hbox->align(KitsuAlign::STRETCH);

    // ========================================================
    // Helper: crear panel scrolleable
    // ========================================================
    auto make_scroll_panel = [](KitsuAlign align_mode) -> KitsuScrollView* {
        auto* scroll = new KitsuScrollView();
        scroll->size(-1, -1);        // ← llena su mitad
        scroll->bottomPadding(20);

        auto* panel = new KitsuPanel();
        panel->padding(16)->spacing(10);
        panel->align(align_mode);
        panel->justify(KitsuJustify::START);

        scroll->content()->add(panel, true);
        return scroll;
    };

    // ========================================================
    // PANEL IZQUIERDO (alineado a la izquierda)
    // ========================================================
    auto* left_scroll = make_scroll_panel(KitsuAlign::START);
    auto* left_panel = static_cast<KitsuBox*>(
        left_scroll->content()->children()[0]);

    left_panel->add((new KitsuLabel("← Izquierda"))
        ->font(KitsuFonts::large())
        ->color(KitsuTheme::active().accent));

    left_panel->add((new KitsuLabel("Un label normal")));
    left_panel->add((new KitsuButton("Botón 1"))
        ->onClick([]() { SDL_Log("Izq Botón 1"); }));
    left_panel->add((new KitsuButton("Botón 2"))
        ->onClick([]() { SDL_Log("Izq Botón 2"); }));
    left_panel->add((new KitsuCheckBox("Check A", true)));
    left_panel->add((new KitsuCheckBox("Check B", false)));
    left_panel->add((new KitsuSwitch("Switch A", true)));
    left_panel->add((new KitsuSlider(0, 100, 30))->suffix("%"));
    left_panel->add((new KitsuTextInput("Texto largo de prueba"))
        ->size(300, 32));

    for (int i = 1; i <= 20; i++) {
        left_panel->add((new KitsuLabel("Item " + std::to_string(i))));
    }

    hbox->add(left_scroll);

    // ========================================================
    // PANEL DERECHO (alineado al centro)
    // ========================================================
    auto* right_scroll = make_scroll_panel(KitsuAlign::CENTER);
    auto* right_panel = static_cast<KitsuBox*>(
        right_scroll->content()->children()[0]);

    right_panel->add((new KitsuLabel("Derecha →"))
        ->font(KitsuFonts::large())
        ->color(KitsuTheme::active().accent));

    right_panel->add((new KitsuLabel("Un label normal")));
    right_panel->add((new KitsuButton("Botón 1"))
        ->onClick([]() { SDL_Log("Der Botón 1"); }));
    right_panel->add((new KitsuButton("Botón 2"))
        ->onClick([]() { SDL_Log("Der Botón 2"); }));
    right_panel->add((new KitsuCheckBox("Check A", true)));
    right_panel->add((new KitsuCheckBox("Check B", false)));
    right_panel->add((new KitsuSwitch("Switch A", true)));
    right_panel->add((new KitsuSlider(0, 100, 70))->suffix("%"));
    right_panel->add((new KitsuTextInput("Otro texto"))
        ->size(300, 32));

    for (int i = 1; i <= 20; i++) {
        right_panel->add((new KitsuLabel("Item " + std::to_string(i))));
    }

    hbox->add(right_scroll);

    // ========================================================
    // Añadir a la ventana
    // ========================================================
    win.add(hbox, true);
    win.addOverlay(new KitsuFPSView(), true);

    run();
    return 0;
}
