# Getting Started

This guide walks you through installing, building, and running your
first KitsuGui application.

> **A word of honesty up front.**
>
> KitsuGui is an early-stage, personal project. It is **not**
> production-ready, it is **not** battle-tested, and it currently
> has a number of known limitations and rough edges. The API may
> change between minor versions without warning. Some widgets have
> bugs, some features are stubs, and some behaviors are
> inconsistent between widgets.
>
> Treat this library as a fun experiment to build UI on top of
> SDL2, not as a framework to bet your next product on. If you are
> looking for something stable and complete, consider Dear ImGui,
> Nuklear, or a full toolkit like Qt.
>
> If that doesn't scare you away, welcome aboard.

---

## What you need

- A C++17 compiler (clang or gcc).
- CMake 3.10 or newer.
- The following libraries, with development headers:
  - SDL2
  - SDL2_ttf
  - SDL2_gfx
  - SDL2_image
- A TTF font file at runtime (KitsuGui does not bundle one).

KitsuGui has been primarily developed and tested on **Termux for
Android** with clang. It should work on Linux and macOS with
little or no adjustment, but those platforms have not received the
same attention.

---

## Installing the dependencies

### Termux

    pkg update
    pkg install cmake clang sdl2 sdl2_ttf sdl2_gfx sdl2_image
    pkg install font-dejavu

The `font-dejavu` package provides the TTF files used in the
examples under:

    /data/data/com.termux/files/usr/share/fonts/TTF/

### Debian / Ubuntu

    sudo apt install build-essential cmake \
                     libsdl2-dev libsdl2-ttf-dev \
                     libsdl2-gfx-dev libsdl2-image-dev \
                     fonts-dejavu

### Arch

    sudo pacman -S base-devel cmake sdl2 sdl2_ttf sdl2_gfx sdl2_image \
                   ttf-dejavu

### macOS (Homebrew)

    brew install cmake sdl2 sdl2_ttf sdl2_gfx sdl2_image

Fonts are available at `/Library/Fonts/` or with Homebrew under
`/opt/homebrew/share/fonts/`.

---

## Getting the source

    git clone https://github.com/DayFoxAnims/Kitsu-GUI.git KitsuGui
    cd KitsuGui

---

## Building

    mkdir build
    cd build
    cmake ..
    make

This produces:

- `libkitsugui.a` — the static library.
- `kitsu_example` — the example program (if `BUILD_EXAMPLES=ON`,
  which is the default).

To build only the library and skip the examples:

    cmake .. -DBUILD_EXAMPLES=OFF
    make

### Common build problems

**`SDL2_ttf not found`**

Install `sdl2_ttf` (or `libsdl2-ttf-dev`) and re-run `cmake ..`.
On Termux, `pkg-config` needs to see it — the package name is
`SDL2_ttf` with the underscore.

**`error: use of undeclared identifier 'setBounds'`**

This means a header is out of sync with its source. Make sure you
did not edit half of a header/source pair. KitsuGui is under
active refactoring, so small inconsistencies can slip through.

**Linker errors about `SDL_...` symbols**

The example target links the SDL libraries twice (once via
`kitsugui`, once explicitly). If you add your own target, follow
the same pattern:

    add_executable(my_app my_main.cpp)
    target_link_libraries(my_app kitsugui
                                  ${SDL2_LIBRARIES}
                                  ${SDL2_TTF_LIBRARIES})

---

## A minimal application

    #include <kitsugui.h>
    #include <SDL2/SDL_ttf.h>

    using namespace KitsuGui;

    int main() {
        // 1. Open a window
        Window win(800, 600, "Hello KitsuGui");
        win.setBackground(KitsuTheme::current().bg_primary);

        // 2. Load a font
        TTF_Init();
        TTF_Font* font = TTF_OpenFont(
            "/path/to/DejaVuSans.ttf", 15);

        KitsuLabel::setDefaultFont(font);

        // 3. Build a small UI
        auto* panel = new KitsuPanel();
        panel->setBounds(0, 0, -1, -1);
        panel->setPadding(40);
        panel->setSpacing(16);
        panel->setAlignment(KitsuAlign::CENTER);
        panel->setJustify(KitsuJustify::CENTER);

        auto* title = new KitsuLabel("Hello, world");
        title->setBounds(0, 0, -1, 32);
        title->withAlign(TextAlign::CENTER);
        panel->addChild(title, true);

        auto* button = new KitsuButton("Click me");
        button->setBounds(0, 0, 140, 40)
              ->withCallback([]() {
                  SDL_Log("Button clicked!");
              });
        panel->addChild(button, true);

        win.add(panel);

        // 4. Run the main loop
        run();

        // 5. Cleanup
        delete panel;
        TTF_CloseFont(font);
        TTF_Quit();

        return 0;
    }

Compile it by dropping the file into `examples/` and adding it
to `CMakeLists.txt`, or by linking against the built
`libkitsugui.a` directly.

---

## Understanding the flow

1. **Create a `Window`.** This initializes SDL, creates the OS
   window, the renderer, and the root container.
2. **Load fonts.** KitsuGui uses raw `TTF_Font*` pointers; you
   manage their lifetime. Set the global default font so widgets
   pick it up automatically.
3. **Build widgets with `new`.** KitsuGui does not hide pointers
   behind smart handles. You allocate widgets, configure them, and
   add them to a container.
4. **Add widgets to a container.** `KitsuPanel`, `KitsuBox`, or
   the window itself. Containers can own their children
   (`addChild(child, true)`) or just reference them
   (`addChild(child, false)`).
5. **Call `run()`.** This enters the main loop: poll events, tick
   widgets, redraw when dirty. It returns when the user closes the
   window.
6. **Free your widgets.** KitsuGui does not delete widgets added
   with `Window::add()`. If you used a panel that owns its
   children, deleting the panel is enough. Otherwise, delete each
   widget manually.

---

## The global font pattern

KitsuGui uses static pointers for shared fonts. Each widget class
has its own:

    KitsuLabel::setDefaultFont(font);
    KitsuButton::setFont(font);
    KitsuTextInput::setFont(font);
    KitsuDropdown::setFont(font);
    KitsuMenuItem::setFont(font);
    KitsuFPSView::setFont(font);

If you forget one, the corresponding widget will not render text.
This is a known rough edge that will be unified eventually.

The widgets do **not** own the font. Call `TTF_CloseFont` after
all widgets using it are destroyed.

---

## Retained mode in practice

KitsuGui is a **retained-mode** library, but with a simple twist:

- Widgets keep their state between frames (position, size, text,
  colors, focus...).
- When anything changes, the widget calls `markDirty()`.
- The main loop redraws the **entire frame** whenever anything is
  dirty.

There is no per-widget clipping, no partial redraw, no dirty-rect
optimization. "Dirty" is really just a "something changed, redraw
the next frame" flag.

This is fine for small UIs. For large ones, or for animations
running at 60 FPS, expect the whole window to be repainted every
frame.

---

## What works today

A non-exhaustive list, in rough order of polish:

- `Window`, `KitsuBox`, `KitsuPanel`
- `KitsuLabel`, `KitsuButton`
- `KitsuTextInput` (with selection, clipboard, context menu)
- `KitsuDropdown` (with a caveat on `clearItems`)
- `KitsuContextMenu`, `KitsuMenuItem`
- `KitsuFPSView`
- Theming via `KitsuTheme`
- Layouts via `KitsuBox` with alignment, justify, spacing
- Config loading (`KitsuConfig`)
- Icon themes and shapes via SDL2_gfx

## What is shaky or missing

- `clearItems` on `KitsuDropdown` does not clear the popup menu.
- `withTheme` on `KitsuButton` and `KitsuTextInput` stores the
  theme but does not use it.
- No keyboard navigation in dropdowns, menus, or lists.
- No scrolling in menus or dropdowns.
- No screen-edge avoidance for popups.
- `KitsuTextInput` has two "enabled" flags; only the private one
  is respected.
- `KitsuTextInput`'s click-to-cursor mapping is off inside nested
  layouts.
- Context menu labels and the dropdown placeholder are hardcoded
  in Spanish.
- No localization system.
- No undo/redo in text inputs.
- No focus ring, no tab order.
- No built-in animation system.
- Widgets are only partially documented.
- No unit tests, no CI.
- No semantic versioning guarantees.

If any of these matter to you, KitsuGui is not ready for your
project. That's okay — it's not trying to be.

---

## Where to go next

- [Architecture](architecture.md) — how the library is put
  together internally.
- [Widget](core/widget.md) — the base class every widget inherits
  from.
- [Layouts](core/layouts.md) — how `KitsuBox` positions things.
- [Window](core/window.md) — the entry point.
- [Theming](theming.md) *(to be documented)* — how to customize
  colors.

---

## A final note

KitsuGui exists because building UI on top of SDL2 is a nice
learning exercise, and because having a small, understandable,
hackable toolkit is fun. It is not trying to compete with mature
GUI libraries.

If you find a bug, that's expected. If you find a limitation not
listed here, feel free to open an issue. If you want to fix
something yourself, even better.

Made with care by DayFox — and with plenty of rough edges on
purpose.
