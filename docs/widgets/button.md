# Button

`KitsuButton` is a clickable, text-labeled button. It is one of the
few widgets with built-in state management: normal, hover, pressed,
and disabled.

    #include <kitsugui/button.h>

    using namespace KitsuGui;

---

## Quick start

    auto* btn = new KitsuButton("Click me");
    btn->setBounds(20, 20, 120, 40)
       ->withCallback([]() {
           SDL_Log("Button clicked!");
       });
    window->add(btn);

That's it. The button handles hover and press automatically as long
as events reach it (see [Event flow](#event-flow)).

---

## Constructor

    KitsuButton(const std::string& text,
                int width  = 120,
                int height = 40);

- `text` — the label shown inside the button.
- `width`, `height` — initial size. Also stored as
  `requested_w/h`, so the button declares this size to a parent
  layout.

The constructor captures no colors. Colors are resolved **at render
time** from `KitsuTheme::current()`, based on the current state.

---

## States

    enum class ButtonState {
        NORMAL,
        HOVER,
        PRESSED,
        DISABLED
    };

The button starts in `NORMAL` and transitions automatically:

| From      | Event                               | To        |
|-----------|-------------------------------------|-----------|
| `NORMAL`  | Mouse enters                        | `HOVER`   |
| `HOVER`   | Mouse leaves                        | `NORMAL`  |
| `HOVER`   | Left mouse button down (inside)     | `PRESSED` |
| `PRESSED` | Left mouse button up (inside)       | `HOVER` + callback |
| `PRESSED` | Left mouse button up (outside)      | `NORMAL`  |
| any       | `disabled()` called                 | `DISABLED`|

`DISABLED` is sticky: once set, the button ignores all events.

You can read the state with:

    ButtonState getState() const;

and set it manually with:

    void setState(ButtonState s);

> Calling `setState` triggers a full redraw of the frame
> (`markAllDirty`). This is intentional to avoid flicker in the
> current retained-mode implementation, but it means changing
> state frequently is not free.

---

## Fluent configuration

    KitsuButton& disabled();
    KitsuButton& withCallback(std::function<void()> cb);
    KitsuButton& withFont(TTF_Font* font);
    KitsuButton& withCorner(float radius);
    KitsuButton& withTheme(Theme t);
    KitsuButton& setBounds(int x, int y, int w, int h);
    KitsuButton& setPosition(int x, int y);

All return `KitsuButton&` and can be chained:

    auto* b = new KitsuButton("Save");
    b->setBounds(0, 0, 140, 40)
     ->withFont(my_font)
     ->withCorner(8.0f)
     ->withCallback([]() { save(); });

### withCallback

    KitsuButton& withCallback(std::function<void()> cb);

Sets the function called when the button is activated (press +
release inside). There is only one callback per button; setting it
again replaces the previous one.

### withFont

    KitsuButton& withFont(TTF_Font* font);

Sets the font used to render the button's text. If not set, the
global default font is used (see [Default font](#default-font)).

### withCorner

    KitsuButton& withCorner(float radius);

Sets the corner radius in pixels. The default is
`KitsuStyle::ButtonRadius`.

The border thickness is fixed at **3 pixels** in the current
implementation and is not configurable.

### withTheme

    KitsuButton& withTheme(Theme t);

> **Not implemented yet.** This method stores a `Theme` value but
> the render code always reads colors from
> `KitsuTheme::current()`, ignoring the stored theme. Reserved for
> a future per-widget theme override.

### setBounds and setPosition

`setBounds` sets the whole rectangle and updates `requested_w/h`.
`setPosition` changes only `x` and `y`, leaving size and requested
size untouched.

---

## Default font

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

Buttons that don't specify their own font use a **single global
default**. Set it once at startup:

    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 15);
    KitsuButton::setFont(font);

The button does **not** own the font; you close it yourself after
all buttons are destroyed.

---

## Callback firing rules

The callback is fired on `SDL_MOUSEBUTTONUP` only if:

1. The button was previously in `PRESSED` state.
2. The mouse button is the left one.
3. The release happens **inside** the button's bounds.

If the user presses inside and releases outside, the button returns
to `NORMAL` and no callback fires. This matches standard button
behavior on every platform.

---

## Event flow

The button reacts to `SDL_MOUSEMOTION`, `SDL_MOUSEBUTTONDOWN`, and
`SDL_MOUSEBUTTONUP`. It expects **absolute** mouse coordinates
(as SDL provides them) and uses `getAbsoluteBounds()` for
hit-testing.

If a parent layout ignores events, the button will not receive
them. In normal use, the root tree forwards events to children in
reverse order (top-most first), and the button returns `true` when
it consumes an event.

The button **does not consume** `MOUSEMOTION` events it isn't
interested in; it returns `inside` (true if the cursor is over the
button). This lets parents track hover state across siblings.

---

## Rendering

The button is drawn as three nested rounded rectangles:

1. **Border** — filled with the border color for the current state.
2. **Background** — filled with the background color, offset by
   `border_thickness` inside the border.
3. **Text** — centered both horizontally and vertically.

Colors per state:

| State      | Background         | Border             | Text            |
|------------|--------------------|--------------------|-----------------|
| `NORMAL`   | `bg_secondary`     | `accent`           | `text_primary`  |
| `HOVER`    | `accent_hover`     | `accent`           | `text_on_accent`|
| `PRESSED`  | `accent_pressed`   | `accent_pressed`   | `text_on_accent`|
| `DISABLED` | `bg_disabled`      | `border_disabled`  | `text_disabled` |

All colors come from `KitsuTheme::current()` at the moment of
rendering. Changing the theme updates all buttons on the next
frame.

---

## Text caching

The button bakes its text into an SDL texture and caches it. The
cache is invalidated when the text, the RGB color, or the font
pointer changes. This is why switching states is cheap even when
the text color changes: the texture is only rebuilt when the
color actually differs.

---

## Memory and ownership

- The button owns its **text texture**.
- The button does **not** own its **font**.
- The button does **not** own its **callback** (closures are
  copied into `std::function`).
- Ownership of the button itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [Layouts](../core/layouts.md)
- [Panel](panel.md)
- [Label](label.md)
