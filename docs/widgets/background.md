# Background & Color

This page covers two things that always go together:

- The `KitsuGui::Color` type — a small, dependency-free RGBA color.
- The **window background** — the base color painted every frame.

Understanding both is essential because the background is the
first thing the renderer draws, and every widget sits on top of
it.

---

## Color

### Header

    #include <kitsugui/color.h>

### Definition

    struct Color {
        Uint8 r, g, b, a;

        Color();
        Color(int r, int g, int b, int a = 255);
        Color(const char* hex);   // e.g. "#FF8800" or "FF8800"

        static Color fromHSV(float h, float s, float v);
    };

`Color` is a plain struct. No inheritance, no virtual methods. It
is meant to be cheap to copy and easy to construct inline.

### Constructors

    Color();                              // black, opaque
    Color(255, 128, 0);                   // orange, opaque
    Color(255, 128, 0, 128);              // orange, 50% transparent
    Color("#FF8800");                     // from hex string
    Color("FF8800");                      // also valid

### fromHSV

    static Color Color::fromHSV(float h, float s, float v);

Builds a color from hue (0–360), saturation (0–1) and value (0–1).
Useful for generating palettes programmatically:

    Color accent = Color::fromHSV(210, 0.8f, 0.9f);   // blue

The alpha channel of the returned color is `255`.

---

## Window background

The background is set on the `Window`:

    void Window::setBackground(const Color& color);
    void Window::setBackground(int r, int g, int b);

### What it does

Every frame, before rendering any widget, the renderer clears the
whole window with the background color. This happens inside the
main loop (see `run.cpp`).

### Example

    Window win(800, 600, "App");

    // From a Color object
    win.setBackground(Color(245, 240, 235));

    // From RGB values
    win.setBackground(245, 240, 235);

    // From a theme color
    win.setBackground(KitsuTheme::current().bg_primary);

### Default

If you don't call `setBackground`, the default is a warm off-white
`Color(245, 240, 235)`, chosen to look neutral with the default
Kitsu theme.

---

## Relationship with the theme

`KitsuTheme::current()` exposes semantic colors. The most common
one to use as a window background is:

- `bg_primary` — the main background of the app.
- `bg_secondary` — for panels and cards.
- `bg_tertiary` — for inputs and subtle surfaces.

Setting the window background to `bg_primary` is the standard way
to make the app follow the active theme:

    win.setBackground(KitsuTheme::current().bg_primary);

When the theme changes at runtime, call `setBackground` again with
the new color. There is no automatic re-binding.

---

## Relationship with KitsuBG

Despite the file name `bg.cpp`, the code in that file implements
the `Color` type, not a widget. There is **no `KitsuBG` widget**.
Historically, the file was named after "background utilities", and
the name stuck.

If in the future a background widget is added (for gradients,
patterns, etc.), it will be documented here as a separate section.

---

## Alpha and blending

`Color` carries an alpha channel, but the window background is
always painted opaque. Alpha matters for widgets drawn on top:
their `a` component is respected if the widget enables blending.

The window background itself does not blend with anything; it is
the lowest layer.

---

## See also

- [Window](window.md)
- [Widget](widget.md)
- [Theming](../theming.md) *(to be documented)*
