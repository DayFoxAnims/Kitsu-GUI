# CheckBox

`KitsuCheckBox` is a labeled checkbox: a small square that toggles
between checked and unchecked when clicked. It is the standard way
to let the user enable or disable independent options.

    #include <kitsugui/check_box.h>

    using namespace KitsuGui;

---

## Quick start

    auto* cb = new KitsuCheckBox("Enable notifications");
    cb->setBounds(20, 20, 220, 24)
      ->withChecked(true)
      ->withCallback([](bool checked) {
          SDL_Log("Notifications: %s", checked ? "on" : "off");
      });
    window->add(cb);

---

## Constructor

    KitsuCheckBox(const std::string& text = "",
                  bool checked = false);

- `text` — the label shown to the right of the box.
- `checked` — initial state. Defaults to `false`.
- Default bounds: `{0, 0, 200, 24}`.

The constructor does **not** fire the callback. Only changes after
construction do.

---

## Fluent configuration

    KitsuCheckBox& withFont(TTF_Font* font);
    KitsuCheckBox& withChecked(bool c);
    KitsuCheckBox& withCallback(std::function<void(bool)> cb);
    KitsuCheckBox& disabled();
    KitsuCheckBox& setBounds(int x, int y, int w, int h);

All return `KitsuCheckBox&`, so they can be chained.

### withChecked

Sets the initial or current state. Same as `setChecked(c)` but
returns the checkbox, allowing chaining.

> Calling `withChecked(true)` on an already-checked checkbox is a
> no-op and does **not** fire the callback.

### withCallback

    std::function<void(bool)> callback;

Called every time the state changes to a new value, whether from
user interaction or from `setChecked` / `toggle`. Receives the new
state as a `bool`.

### disabled

Sets the checkbox to disabled. In this state it:

- Renders in disabled colors (`bg_disabled`, `border_disabled`,
  `text_disabled`).
- Ignores all mouse events.

> **Known issue:** `disabled()` sets the private `enabled_` flag,
> not the public `KitsuWidget::enabled`. Only the private flag is
> respected by this widget. Setting `enabled = false` directly has
> no effect. This will be unified in a future release.

---

## State and toggling

    bool isChecked() const;
    void setChecked(bool c);
    void toggle();

`toggle()` is a convenience that calls `setChecked(!checked)`.

Setting the checkbox to the same value it already has is a no-op:
no `markDirty`, no callback.

---

## Events

The checkbox reacts to:

- `SDL_MOUSEMOTION` — updates the hover state, which tints the
  label in the accent color.
- `SDL_MOUSEBUTTONDOWN` (left, inside) — records the press.
- `SDL_MOUSEBUTTONUP` (left, inside, after a press) — toggles the
  state.

A click that starts inside and ends outside is **cancelled** (no
toggle). This is standard behavior and prevents accidental toggles
from drag gestures.

`MOUSEMOTION` returns `inside`, so parents can track hover state
across siblings. `MOUSEBUTTONDOWN` and `MOUSEBUTTONUP` return
`true` when they are consumed.

---

## Rendering

The checkbox draws, in order:

1. **Box background** — a rounded square, size from
   `KitsuTheme::checkbox_size` (default 18px). Fill color depends
   on state:
   - Checked: `accent`.
   - Hover (unchecked): `bg_tertiary`.
   - Normal: `bg_secondary`.
   - Disabled: `bg_disabled`.
2. **Box border** — drawn as an outline in a state-dependent color.
   Uses `radius_checkbox` from the theme.
3. **Check mark** — if checked, a hand-drawn 3px-thick checkmark
   using `SDL_RenderDrawLine`. Uses `text_on_accent` (or
   `text_disabled` if disabled).
4. **Label** — the text, rendered to the right of the box.

The label color also depends on state:

| State          | Label color       |
|----------------|-------------------|
| Normal         | `text_primary`    |
| Hover          | `accent`          |
| Disabled       | `text_disabled`   |

---

## Text caching

The label texture is cached and invalidated when any of these
change:

- The label text.
- The RGB color (which depends on state).
- The active font pointer.

Switching states only rebuilds the texture when the color actually
differs, so hovering does not cause excessive rebuilds.

---

## Sizing

The checkbox uses `bounds.h` for vertical centering of the box and
label. The **width** is mostly used for hit-testing; the visible
content (box + text) may be smaller.

Set the bounds large enough to contain the label:

    cb->setBounds(20, 20, 220, 24);

If the label is wider than `bounds.w`, it will overflow visually
but the widget's hit area stays at `bounds.w`.

---

## Common patterns

### Enabling a dependent field

    auto* cb = new KitsuCheckBox("Advanced mode");
    auto* field = new KitsuTextInput();
    cb->withCallback([field](bool on) {
        field->enabled = on;
    });

> Note: for `KitsuTextInput`, use the private flag mechanism (see
> [TextInput](text_input.md#known-issues-and-notes)).

### Checkbox already checked

    auto* cb = new KitsuCheckBox("Remember me", true);
    // The callback is not called for the initial state.

### Programmatic toggle

    cb->toggle();   // fires the callback

---

## Memory and ownership

- The checkbox owns its **text texture**.
- The checkbox does **not** own its **font**.
- Ownership of the checkbox itself is decided by its container
  (see [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [Switch](switch.md) — same idea, different visual
- [Radio](radio.md) — for mutually exclusive options
- [Layouts](../core/layouts.md)
