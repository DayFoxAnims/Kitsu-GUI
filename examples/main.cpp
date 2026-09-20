    #include <kitsugui.h>

    using namespace KitsuGui;

    int main() {
        Window win(800, 600, "KitsuGui Demo");
        win.setBackground(245, 240, 235);

        auto* title = new KitsuLabel("Hello, KitsuGui");
        title->setBounds(40, 40, 300, 40);
        win.add(title);
        
            auto* dd = new KitsuDropdown(260);
    dd->setBounds(0, 0, 260, 36)
       ->withPlaceholder("Select a language...")
       ->withFont(font_normal)
       ->withCallback([](int i, const std::string& text) {
           SDL_Log("Language %d: %s", i, text.c_str());
       });

    dd->addItems({
        "English",
        "Español",
        "Français",
        "Deutsch",
        "日本語",
        "中文"
    });

    dd->setSelectedIndex(0);
    panel->addChild(dd, true);

        run();
        return 0;
    }11
