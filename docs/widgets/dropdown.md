# Dropdown

`KitsuDropdown` is a select-one-from-many widget. It shows the
current selection (or a placeholder) in a compact box, and when
clicked, opens a popup menu below it with all the available items.

    #include <kitsugui/dropdown.h>

    using namespace KitsuGui;

---

## Quick start

    auto* dd = new KitsuDropdown(200);
    dd->setBounds(20, 20, 200, 36)
       ->withPlaceholder("Pick a language...")
       ->withCallback([](int index, const std::string& text) {
           SDL_Log("Selected: %s (index %d)", text.c_str(), index);
       });

    dd->addItem("English");
    dd->addItem("Español");
    dd->addItem("日本語");

    window->add(dd);

Clicking the dropdown opens the menu below it. Selecting an item
updates the display and fires the callback.

---

## Constructor

    KitsuDropdown(int width = 200);

- `width` — the width of the dropdown box, in pixels. Also used
  as the width of the internal popup menu, so both align
  vertically.
- Default height: `36`.
- Default placeholder: `"Seleccionar..."` (hardcoded Spanish; see
  [Localization notes](#localization-notes)).
- Default selection: `-1` (nothing selected).

The constructor also creates a private `KitsuContextMenu` sized to
`width`. The menu is filled as you add items.

---

## Managing items

    int addItem(const std::string& text,
                std::function<void()> cb = nullptr);
    void addItems(const std::vector<std::string>& items);
    void clearItems();

### addItem

Adds an option. Returns its index (starting at `0`).

- If `cb` is provided, it is invoked **after** the dropdown
  selection changes, only when the user selects this item from
  the popup.
- The same `text` and `cb` are pushed into the internal menu.

Calling `addItem` after the widget is visible is fine — the menu
grows on the next `showAt` call.

### addItems

Convenience wrapper that calls `addItem` in a loop. All added items
share no callback.

### clearItems

> **Partially implemented.** This clears the internal list of
> items owned by the dropdown, but it does **not** clear the
> internal `KitsuContextMenu`. The old entries remain visible in
> the popup until the menu itself is destroyed.
>
> Effectively, this means `clearItems` is only safe to call
> **before** the dropdown has ever been shown. A follow-up release
> will wire it to `KitsuContextMenu::clearItems()`.

---

## Selection

    int getSelectedIndex() const;
    std::string getSelectedText() const;
    void setSelectedIndex(int index);
    void setSelectedText(const std::string& text);

### getSelectedIndex

Returns the index of the selected item, or `-1` if nothing is
selected.

### getSelectedText

Returns the text of the selected item, or an **empty string** if
nothing is selected. Note that this is different from returning
the placeholder: the placeholder is only used for display.

### setSelectedIndex

Changes the selection programmatically.

- Accepts `-1` to clear the selection.
- Ignores out-of-range indices.
- Is a no-op if the new index equals the current one.
- Fires the `on_change` callback (see below) when the value
  actually changes.

### setSelectedText

Finds an item whose text matches exactly and selects it. If no
match is found, does nothing. Case-sensitive.

---

## Callback

    KitsuDropdown& withCallback(
        std::function<void(int, const std::string&)> cb);

Sets the function called whenever the selection changes, whether
from user interaction or from `setSelectedIndex`. Receives:

- The new index (`-1` if cleared).
- The new text (empty string if cleared).

The callback does **not** fire for `setSelectedText` if the text
was not found (since nothing changed).

Individual items can also have their own callback (passed to
`addItem`). Both callbacks fire on selection, in this order:

1. The dropdown's `on_change`.
2. The selected item's own callback.

---

## Fluent configuration

    KitsuDropdown& withFont(TTF_Font* font);
    KitsuDropdown& withPlaceholder(const std::string& text);
    KitsuDropdown& setBounds(int x, int y, int w, int h);

### withFont

Sets the font used to render the current selection. If not set,
the global default font is used (see [Default font](#default-font)).

The font is **not** propagated to the internal menu; menu items
use their own global font (`KitsuMenuItem::setFont`).

### withPlaceholder

Sets the text shown when no item is selected. The placeholder is
rendered in `text_secondary` to distinguish it from a real
selection.

### setBounds

Sets the full rectangle. Also updates `requested_w/h`, so the
dropdown declares its size to a parent layout.

---

## Default font

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

Dropdowns that don't specify their own font use a **global
default**. Set it once at startup:

    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 15);
    KitsuDropdown::setFont(font);
    KitsuMenuItem::setFont(font);   // menu items use their own

The dropdown does **not** own the font.

---

## Popup behavior

When the user clicks inside the dropdown (left button), the
internal menu opens:

    SDL_Rect abs = getAbsoluteBounds();
    menu->showAt(abs.x, abs.y + abs.h);

The menu appears directly **below** the dropdown box, with the
same width, at absolute screen coordinates.

If another context menu was already open (for example a right-click
menu), it is closed first — only one menu can be active at a time
in the whole application.

### Menu state tracking

The dropdown keeps a `menu_open` flag to drive its border color
(`border_focus` while open). The flag is set when the dropdown
itself opens the menu, and is cleared in `tick()` when the menu is
found to be closed.

This means: **if you open the menu by calling
`dropdown->getMenu()->showAt(...)` from outside code, the dropdown
will not visually indicate the open state until the next frame.**
In practice this is invisible, but worth knowing if you hook into
the menu directly.

---

## Rendering

The dropdown draws four layers, in order:

1. **Background** — rounded rectangle, color depends on state
   (see below).
2. **Border** — rounded outline, color depends on state.
3. **Text** — the selected text, or the placeholder in
   `text_secondary`.
4. **Arrow ▼** — a small downward chevron near the right edge,
   drawn with `SDL_RenderDrawLine`.

### State-dependent colors

| State              | Background    | Border          | Text             |
|--------------------|---------------|-----------------|------------------|
| Normal             | `bg_tertiary` | `border`        | `text_primary`   |
| Mouse inside       | `bg_tertiary` | `accent`        | `text_primary`   |
| Menu open          | `bg_tertiary` | `border_focus`  | `text_primary`   |
| Disabled (`enabled = false`) | `bg_disabled` | `border_disabled` | `text_disabled` |

The border uses `border_thickness_button` from the theme.

### Text truncation

If the selected text (or placeholder) is wider than
`bounds.w - 40`, the texture is **clipped** during the blit using
a partial source rect. No ellipsis is added; the text simply cuts
off at the limit.

The `40` accounts for:

- 12px left padding.
- ~16px for the arrow.
- A few pixels of safety margin.

---

## Disabling

Set `enabled = false` to disable the dropdown:

    dd->enabled = false;

A disabled dropdown:

- Renders in disabled colors.
- Ignores all mouse events (`handleEvent` returns early).
- Cannot be opened.

Unlike `KitsuButton`, there is no fluent `disabled()` helper. Set
the flag directly.

---

## Events

The dropdown reacts to:

- `SDL_MOUSEMOTION` — updates the hover state (`mouse_inside`).
- `SDL_MOUSEBUTTONDOWN` with left button inside — opens the menu.

It does **not** handle keyboard events. Once the menu is open, all
input goes to the menu (see
[ContextMenu](context_menu.md#event-capture)).

---

## Memory and ownership

- The dropdown owns its **text texture**.
- The dropdown owns its **internal menu** and destroys it in the
  destructor.
- The dropdown does **not** own its **font**.
- Ownership of the dropdown itself is decided by its container
  (see [Layouts](../core/layouts.md)).

If the dropdown is destroyed while its menu is open, the menu is
deleted as well. This is safe because the menu is owned by the
dropdown, but it means any code holding a pointer via
`getMenu()` becomes invalid. Do not cache `getMenu()` across
frames.

---

## Known limitations

- **`clearItems()` does not clear the popup menu.** See the note
  above. Use the dropdown only with a fixed set of items, or
  delete and recreate it to reset the options.
- **No auto-flip.** If the dropdown is near the bottom of the
  window, the popup will extend past the visible area. You must
  position the dropdown with enough room below it.
- **No keyboard navigation.** Arrow keys, `Enter`, and `Tab` do
  not drive the dropdown. All interaction is mouse-based.
- **No scrolling.** If the popup has more items than fit on
  screen, the excess items are unreachable.
- **No search or filtering.** The list is fixed once populated.
- **The popup does not indicate which item is currently
  selected.** All items render the same way; there is no
  checkmark or highlight for the active choice.

---

## Localization notes

Two strings are currently hardcoded in Spanish:

- The default placeholder: `"Seleccionar..."`.
- The four items of the `KitsuTextInput` context menu
  (`"Copiar"`, `"Cortar"`, `"Pegar"`, `"Seleccionar todo"`).

There is no localization system yet. To use English (or any other
language), set the placeholder explicitly:

    dd->withPlaceholder("Select an option...");

Menu items you build yourself can be in any language you like.

---

## Example: language selector

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

---

## See also

- [Widget](../core/widget.md)
- [Layouts](../core/layouts.md)
- [ContextMenu](context_menu.md)
- [TextInput](text_input.md)
- [Panel](panel.md)
