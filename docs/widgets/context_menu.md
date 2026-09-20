# ContextMenu

`KitsuContextMenu` is a floating popup menu shown at an absolute
position on the screen. It is used for right-click menus, dropdown
popups, and any other "menu appears on demand" scenario.

The menu captures **all events** while open: nothing underneath it
reacts to clicks or keyboard input until the menu closes.

    #include <kitsugui/context_menu.h>

    using namespace KitsuGui;

---

## Overview

Two classes are involved:

- **`KitsuContextMenu`** — the container. Manages items, position,
  visibility, and event capture.
- **`KitsuMenuItem`** — a single row. Handles hover, click, and
  invokes a callback. Can also act as a separator.

Both are constructed with `new` and owned by the menu. You rarely
need to touch `KitsuMenuItem` directly — the `addItem` and
`addSeparator` helpers on the menu build them for you.

---

## Quick start

    auto* menu = new KitsuContextMenu(200);
    menu->addItem("Open",   []() { open_file(); });
    menu->addItem("Save",   []() { save_file(); });
    menu->addSeparator();
    menu->addItem("Quit",   []() { quit(); });

    // Show it at an absolute screen position
    menu->showAt(300, 200);

    // Later, to close it:
    menu->hide();

The menu appears at `(300, 200)` with the items stacked vertically.

---

## Construction

    KitsuContextMenu(int width = 200);

- `width` — the total width of the menu in pixels, including
  padding. All items are laid out inside this width.
- The menu starts **hidden** (`visible = false`, `open = false`).
- Its `parent` is set to `nullptr` on purpose: the menu uses
  **absolute screen coordinates**, not parent-relative ones.

The menu is designed to be spawned once and reused. You can add
items, then call `showAt` multiple times. Items are not cleared
between shows.

---

## Adding items

    KitsuMenuItem* addItem(const std::string& text,
                           std::function<void()> cb = nullptr);
    KitsuMenuItem* addSeparator();
    void clearItems();

### addItem

Adds a clickable row with the given label. If `cb` is provided,
it is invoked when the item is activated (mouse press + release
inside).

Each call also triggers a re-layout of the whole menu, so the
height grows to fit the new item.

### addSeparator

Adds a horizontal line that is not clickable. Internally this is a
`KitsuMenuItem` with `separator = true` and `item_enabled = false`.

### clearItems

Deletes every item (owned) and relayouts the menu. The menu keeps
its `menu_width` and stays hidden.

### Ownership

The menu **owns** every item added. They are deleted in:

- `clearItems()`
- the menu's destructor

Do not delete items returned by `addItem` yourself.

---

## Showing and hiding

    void showAt(int x, int y);
    void hide();
    bool isOpen() const;

### showAt

Shows the menu at **absolute screen coordinates** `(x, y)`.

What it does:

1. If another menu is currently open, calls `hide()` on it.
2. Positions the menu at `(x, y)` and makes it visible.
3. Sets `open = true`.
4. Sets `ignore_next_up_ = true` (see [Why the first click
   doesn't close it](#why-the-first-click-doesnt-close-it)).
5. Relayouts items.
6. Registers itself as the global active menu.
7. Forces a full redraw of the frame.

Coordinates are **absolute**. If you want to show the menu below a
widget, compute the position from that widget's bounds:

    SDL_Rect abs = button->getAbsoluteBounds();
    menu->showAt(abs.x, abs.y + abs.h);

### hide

Hides the menu and clears the global active menu pointer if it was
this menu. Also clears the "ignore next up" flag and marks all
items as dirty (so their hover state is reset).

`hide()` is called automatically when:

- The user clicks outside the menu.
- The user presses `ESC`.
- An item is activated.

### isOpen

Returns `true` while the menu is visible and accepting events.

---

## The global active menu

Only **one context menu can be open at a time** across the entire
application. This is enforced by a static pointer:

    static KitsuContextMenu* s_active_menu;
    static KitsuContextMenu* getActive();

When `showAt` is called:

- If another menu is already open, it is hidden first.
- The menu that just opened becomes the active one.

When `hide` is called:

- If this menu was the active one, the static pointer is cleared.

This design is what allows the main loop to route all events to
"the currently open menu" without the caller having to track which
menu is visible.

If you are writing custom event routing code, use:

    KitsuContextMenu* m = KitsuContextMenu::getActive();
    if (m && m->isOpen()) {
        m->handleEvent(event);
        // do not forward the event to the rest of the app
    }

---

## Event capture

The menu is a **modal overlay**. While open, it consumes **every**
event it receives:

- `MOUSEMOTION` → forwards to items for hover state.
- `MOUSEBUTTONDOWN` inside → forwards to items.
- `MOUSEBUTTONDOWN` outside → calls `hide()`, consumes.
- `MOUSEBUTTONUP` → forwards if inside, consumes silently if outside.
- `KEYDOWN` with `ESC` → calls `hide()`, consumes.
- Any other `KEYDOWN` → consumes (so typing does not leak to the
  app behind the menu).
- `MOUSEWHEEL` and everything else → consumes.

This is intentional. The menu behaves like a modal dialog: while
it's open, nothing else in the app should react to input.

The main loop is responsible for **only sending events to the
active menu**, and not to the widget tree underneath. If you
forward events to both, the menu will not work correctly.

---

## Why the first click doesn't close it

There is a subtle timing problem with context menus:

1. The user right-clicks or left-clicks a widget.
2. The widget calls `menu->showAt(...)` during `MOUSEBUTTONDOWN`.
3. The same physical click also produces a `MOUSEBUTTONUP`.
4. That `MOUSEBUTTONUP` arrives **after the menu is already open**.
5. The menu sees an "up outside" event and would close immediately.

To avoid this, `showAt` sets an internal flag:

    bool ignore_next_up_ = true;

While this flag is set, the menu consumes every event **without
processing it**, until it sees a `MOUSEBUTTONUP`. That event is
discarded and the flag is cleared. From then on, the menu works
normally.

You do not need to do anything for this — it is automatic. But if
you notice the menu "eating" one `MOUSEBUTTONUP` right after
opening, this is why.

---

## Item behavior

A `KitsuMenuItem`:

- Highlights on hover using `accent_hover`.
- Highlights on press using `accent_pressed`.
- Changes text color to `text_on_accent` when highlighted.
- Uses `text_disabled` when `item_enabled == false`.
- Fires its callback on `MOUSEBUTTONUP` **only if** it was
  previously pressed **and** the release happens inside the item.

The callback sequence is:

1. `KitsuContextMenu::getActive()->hide()` — close the menu first.
2. Invoke the item's callback.

Closing before the callback ensures that the menu is not in an
inconsistent state if the callback opens another menu, shows a
dialog, or destroys the widget that spawned the menu.

Separators have no hover, no press, no callback.

---

## Rendering

The menu draws, in order:

1. **Shadow** — a black rectangle at `(x+2, y+2)` with 40/255
   alpha. Gives the impression of elevation.
2. **Background** — a rounded rectangle filled with
   `bg_secondary`, using `radius_panel` from the theme.
3. **Border** — a 1px outline using `border` from the theme.
4. **Items** — each item is drawn by `KitsuMenuItem::render`.

Items draw their own background only when hovered or pressed, so
the menu background shows through for non-active items.

The menu's corner radius comes from the current theme at render
time (not captured at construction).

---

## Layout

Items are laid out vertically by `relayout()`, called whenever:

- An item is added or removed.
- `showAt` is invoked.

The height of the menu is recomputed each time:

    height = padding_v + sum(item heights) + padding_v

where separators contribute `separator_height` (9px) and normal
items contribute `item_height` (32px).

`menu_width` is fixed at construction; individual items are laid
out to `menu_width - 2 * padding_h`.

---

## Known limitations

- **No automatic screen-edge avoidance.** If you open a menu near
  the bottom or right edge of the window, it will extend beyond
  the visible area. You must clamp the coordinates yourself:

      int mx = click_x;
      int my = click_y;
      if (mx + menu_width > window_width) mx = window_width - menu_width;
      // (similar for bottom)

- **No submenus.** Unlike the `GUI_Menu` reference implementation
  we based this on, KitsuGui's context menu is flat. Every item is
  a leaf.

- **No keyboard navigation.** `ESC` closes the menu, but arrow
  keys do not move a highlight between items. You must use the
  mouse.

- **The menu does not resize itself if the theme's font size
  changes** at runtime. Item heights are fixed.

- **Item text is truncated visually** by the width, but there is
  no ellipsis. Long labels simply overflow the item's bounds (the
  SDL_Texture is drawn at its natural size).

- **No animation.** The menu appears and disappears instantly.

---

## Memory and ownership

- The menu owns its **items** (both normal and separator). They
  are deleted by `clearItems` and by the destructor.
- The menu owns **itself** in the sense that you `new` it and must
  `delete` it when done.
- The menu does **not** own the callbacks (they are copied into
  `std::function`).

If you open a menu and then destroy the widget that owns it (for
example a `KitsuTextInput`), you must ensure the menu is hidden
first, or the global active pointer will dangle. Best practice:

    MyWidget::~MyWidget() {
        if (KitsuContextMenu::getActive() == my_menu) {
            my_menu->hide();
        }
        delete my_menu;
    }

---

## Typical use cases

### Right-click on a widget

    widget->on_right_click = [menu](int x, int y) {
        menu->showAt(x, y);
    };

### Dropdown popup

A `KitsuDropdown` positions its menu below the widget's bottom
edge:

    SDL_Rect abs = dropdown->getAbsoluteBounds();
    menu->showAt(abs.x, abs.y + abs.h);

### Standalone menu

    auto* menu = new KitsuContextMenu(180);
    menu->addItem("Refresh", []() { refresh(); });
    menu->addItem("Settings", []() { open_settings(); });
    menu->showAt(100, 100);

    // In your main loop, forward events to the active menu first,
    // then to the widget tree only if no menu is open.

---

## See also

- [Widget](../core/widget.md)
- [Panel](panel.md)
- [TextInput](text_input.md)
- [Dropdown](dropdown.md) *(to be documented)*
