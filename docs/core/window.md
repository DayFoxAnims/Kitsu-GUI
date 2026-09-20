# Window

The `KitsuGui::Window` class is the entry point of every KitsuGui
application. It owns the SDL window, the SDL renderer, the Kitsu
renderer, and the root widget tree.

Every KitsuGui app creates **one** `Window` at the start of
`main()`, adds widgets to it, and then calls `KitsuGui::run()` to
enter the main loop.

---

## Header

    #include <kitsugui.h>

    using namespace KitsuGui;

---

## Constructor

    Window(int width, int height,
           const std::string& title,
           bool resizable = true);

Creates the SDL window, the renderer, and the root container.

- `width`, `height` — initial size in pixels.
- `title` — window title.
- `resizable` — whether the user can resize the window.
  Defaults to `true`.

Internally, `Window` also creates:

- A `KitsuRenderer` — the retained-mode frame buffer wrapper.
- A root `KitsuBox` (vertical) sized to the full window, with
  auto-layout disabled. Widgets added with `add()` are positioned
  with `setBounds()`, not by the root.

---

## Example

    #include <kitsugui.h>

    using namespace KitsuGui;

    int main() {
        Window win(800, 600, "My KitsuGui App");
        win.setBackground(KitsuTheme::current().bg_primary);

        // ... add widgets ...

        run();   // enter the main loop
        return 0;
    }

---

## Methods

### setBackground

    void setBackground(const Color& color);
    void setBackground(int r, int g, int b);

Sets the color used to clear the frame every time the window is
redrawn. The background is not a widget; it is drawn directly by
the renderer before the widget tree.

See [Background & Color](background.md) for details on the
`Color` type.

---

### add

    void add(KitsuWidget* widget);

Adds a widget to the root container. The widget becomes part of
the layout tree.

The `Window` **does not own** the widget. Memory management is the
caller's responsibility. This is intentional: it lets you keep
widgets on the stack, inside other objects, or managed by your
own container.

---

### addOverlay

    void addOverlay(KitsuWidget* widget);

Adds a widget as an **overlay**. Overlays:

- Are not part of the layout tree.
- Are rendered **after** the main tree, on top of everything.
- Do not receive events from the layout pass; you must route them
  manually if needed.

Typical use case: `KitsuFPSView`.

---

### getRoot

    KitsuBox* getRoot() const;

Returns the root container. Useful if you want to manipulate the
whole tree, iterate children, or replace the root layout behavior.

---

### syncRootSize

    void syncRootSize();

Resizes the root container to match the current window size. This
is called automatically when the window receives a
`SDL_WINDOWEVENT_RESIZED` event (handled inside `run()`), so you
normally don't need to call it yourself.

---

### Accessors

    SDL_Window*    getSDLWindow()     const;
    SDL_Renderer*  getSDLRenderer()   const;
    KitsuRenderer* getKitsuRenderer() const;

Direct access to the underlying SDL and Kitsu objects. Use with
care; bypassing the Kitsu API may cause inconsistent state.

---

## Ownership & lifetime

- The `Window` owns:
  - the `SDL_Window`,
  - the `SDL_Renderer`,
  - the `KitsuRenderer`,
  - the root `KitsuBox`.
- The `Window` does **not** own:
  - widgets added with `add()` or `addOverlay()`.

When the `Window` is destroyed, the SDL resources are released. Any
widgets you allocated yourself must be freed before or after,
depending on your design.

Typical pattern:

    int main() {
        Window win(800, 600, "App");

        auto* panel = new KitsuPanel();
        // ... configure panel ...
        win.add(panel);

        run();

        delete panel;   // free after the loop ends
        return 0;
    }

---

## Relationship with Background

The window background is the **first thing drawn** every frame.
It covers the whole window and gives the base color on top of
which every widget is rendered.

If you want a widget to blend with the background, you usually set
its own background color to match `KitsuTheme::current().bg_primary`
or leave it transparent (depending on the widget).

See [Background & Color](background.md).

---

## Threading notes

The `Window` (and `run()`) must be created and driven from the main
thread. SDL requires the event loop and rendering to happen on the
main thread on most platforms, especially on mobile.

---

## See also

- [Background & Color](background.md)
- [Widget](widget.md)
- [Layouts](layouts.md)
- [Getting Started](../getting-started.md)
