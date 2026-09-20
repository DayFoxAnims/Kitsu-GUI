# Popup

`KitsuPopup` is a **separate OS window** used for dialogs, alerts,
and small forms. Unlike the rest of KitsuGui's widgets, a popup is
not part of the main window tree — it has its own SDL window,
renderer, and root box.

This makes popups genuinely modal on desktop, and lets them appear
above the main window in the window manager's stacking order.

    #include <kitsugui/popup.h>

    using namespace KitsuGui;

---

## Quick start

    auto* popup = Popup::message("Info", "Operation completed.");
    popup->show();

Or use one of the specialized helpers:

    Popup::confirm("Delete file",
                   "This cannot be undone. Continue?",
                   [](bool ok) {
                       if (ok) delete_file();
                   });

    Popup::input("Your name",
                 "Please enter your name:",
                 [](const std::string& name) {
                     SDL_Log("Hello, %s", name.c_str());
                 });

The helpers create the popup, configure its layout, wire the
buttons, and register it in the global popup list. You only need to
keep a pointer if you want to close it manually.

---

## Concepts

A popup is not a `KitsuWidget`. It is a standalone object that
manages:

- An `SDL_Window` with its own title and size.
- An `SDL_Renderer` for that window.
- A `KitsuRenderer` wrapping the SDL renderer.
- A root `KitsuBox` (vertical) holding the popup's widgets.

The main loop (`run()`) tracks all open popups in a global vector
`g_popups` and drives their event handling, ticking, and rendering
separately from the main window.

---

## Constructor

    KitsuPopup(int width, int height, const std::string& title);

Creates the SDL window and its renderer, then builds a root
vertical box with:

- `padding = 20`
- `spacing = 15`
- `alignment = CENTER`
- `justify = CENTER`
- auto-layout enabled

The popup is **shown immediately** (`open = true`) and added to the
global popup list. To start with it hidden, call `hide()` after
construction.

---

## Fluent configuration

    KitsuPopup& withType(PopupType t);
    KitsuPopup& withMessage(const std::string& msg);
    KitsuPopup& withModal(bool modal);
    KitsuPopup& withResizable(bool resizable);
    KitsuPopup& withCloseOnEscape(bool enabled);

### withType

    enum class PopupType {
        MESSAGE,     // just a message + OK
        CONFIRM,     // message + Yes/No
        INPUT,       // message + text field + OK/Cancel
        WARNING,     // warning icon + message + OK
        ERROR,       // error icon + message + OK
        INFO,        // info icon + message + OK
        SUCCESS,     // success icon + message + OK
        CUSTOM       // you build the layout yourself
    };

The type is **informational only** — it does not change rendering
or behavior. The helper functions in the `Popup` namespace use it
to configure the popup appropriately, but you can override
everything yourself.

### withMessage

Adds a `KitsuLabel` with the given text, wrapped to
`width - 40` and centered. Fires the label's own behavior.

### withModal

    KitsuPopup& withModal(bool modal);

If `modal = true`, the popup becomes a modal dialog for the main
window (`SDL_SetWindowModalFor`). On desktop, this means the main
window cannot be interacted with while the popup is open. On mobile
and some window managers, the behavior may vary.

The main loop also uses `isModal()` to determine whether the main
window's widget tree should receive events. If any modal popup is
open, `getActiveModal()` returns it, and `run()` skips sending
events to the main root.

### withResizable

Whether the user can resize the popup's SDL window. Default:
not resizable.

### withCloseOnEscape

If `true` (default), pressing `ESC` while the popup has keyboard
focus closes it.

---

## Adding content

    void add(KitsuWidget* widget);
    void addButton(const std::string& text,
                   std::function<void()> cb);
    void addLabel(const std::string& text);

### add

Adds an arbitrary widget to the popup's root. The popup does not
own the widget — you must delete it yourself, or use `addButton` /
`addLabel` which take ownership.

### addButton

Creates a `KitsuButton`, wires its callback, and adds it to the
root. The popup **owns** the button and deletes it in the
destructor.

### addLabel

Same as `addButton` but for a `KitsuLabel`. Centered horizontally,
wrapped to the popup width minus 40 pixels.

---

## Lifecycle

    void show();
    void hide();
    void close();
    bool isOpen() const;

### show

Makes the SDL window visible and raises it to the front. Marks all
content dirty so it redraws.

### hide

Hides the SDL window without destroying the popup. The popup stays
in the global list and keeps ticking.

### close

Sets `open = false` and calls `on_close` (if set). It does **not**
hide the window immediately — the main loop notices the popup is
no longer open and destroys it on the next frame.

### isOpen

Returns whether the popup is still alive and interactive.

---

## Rendering and ticking

Unlike regular widgets, popups are **not** rendered as part of the
main window's tree. The main loop renders them separately:

    for (auto* popup : g_popups) {
        if (popup->isOpen()) popup->render();
    }

Each popup:

1. Begins its own frame in its own `KitsuRenderer`.
2. Clears its window with the theme's `bg_primary`.
3. Renders its root box.
4. Ends the frame.

The popup's root box is re-laid out on every render, so you can
add widgets at runtime without calling `updateLayout` yourself.

`KitsuPopup::tick()` forwards to the root's `tick()`, which drives
any animated widgets (like the caret in a text input).

---

## Event handling

The main loop forwards every SDL event to all open popups. Each
popup then filters:

1. **Window events** — only if the event's `windowID` matches the
   popup's own window.
2. **Mouse events** — only if the mouse focus is on the popup's
   window.
3. **Keyboard events** — only if the keyboard focus is on the
   popup's window.

If the event passes the filter, it is forwarded to the root box's
`handleEvent`.

`ESC` handling happens **before** the root sees the event, so it
closes the popup even if a child would consume the key.

---

## Global popup list

    extern std::vector<KitsuPopup*> g_popups;

Every popup registers itself here on construction and removes
itself on destruction. The main loop uses this list to render,
tick, and eventually destroy closed popups.

You can iterate this list to inspect open popups, but do not
modify it directly. Use `close()` to mark a popup for cleanup.

### getActiveModal

    KitsuPopup* getActiveModal();

Returns the first open modal popup, or `nullptr` if none. The main
loop uses this to skip forwarding events to the main window when a
modal is open.

---

## Helper functions

The `Popup` namespace provides shortcuts:

    KitsuPopup* message(const std::string& title,
                        const std::string& msg);

    KitsuPopup* confirm(const std::string& title,
                        const std::string& msg,
                        std::function<void(bool)> cb);

    KitsuPopup* input(const std::string& title,
                      const std::string& msg,
                      std::function<void(const std::string&)> cb);

    KitsuPopup* warning(const std::string& title,
                        const std::string& msg);

    KitsuPopup* error(const std::string& title,
                      const std::string& msg);

    KitsuPopup* info(const std::string& title,
                     const std::string& msg);

    KitsuPopup* success(const std::string& title,
                        const std::string& msg);

Each helper creates a fully configured popup and returns it. You
can further customize it before or after showing.

### Icon popups

`warning`, `error`, `info`, and `success` render a horizontal row
with a themed system icon (`dialog-warning`, `dialog-error`, etc.)
and the message. The icon is loaded through
`KitsuIconTheme` (see [Icon](icon.md)) and requires an active
icon theme to be set.

> **If no icon theme is loaded**, the icon view renders nothing and
> the row looks like a plain message with extra left padding.
> Loading a theme at startup is strongly recommended.

---

## Modal behavior

When a modal popup is open:

- The main loop **skips event forwarding to the main window's
  root**. This means buttons and inputs in the main window cannot
  be interacted with while the modal is up.
- The popup itself receives all events.
- On desktop, `SDL_SetWindowModalFor` tells the window manager to
  block input to the parent window.

Non-modal popups do **not** block the main window. Both can be
interacted with, and the main loop routes events by window ID.

---

## Ownership and lifecycle of widgets

The popup has two ownership modes for content:

- `add(widget)` — the popup does **not** own the widget. You are
  responsible for deleting it.
- `addButton` / `addLabel` — the popup **owns** the widget. It is
  deleted in the popup's destructor.

`withMessage` also adds an owned label.

This split is intentional: it lets you keep a pointer to a text
input inside an input popup (so you can read its value in the OK
callback) without the popup stealing ownership.

---

## Destruction

The popup is deleted by the main loop when `isOpen()` returns
false:

    for (size_t i = 0; i < g_popups.size(); ) {
        KitsuPopup* popup = g_popups[i];
        if (!popup || !popup->isOpen()) {
            delete popup;
        } else {
            i++;
        }
    }

**Do not delete a popup yourself.** Always call `close()` and let
the loop clean up. Deleting manually while the loop holds a
pointer will cause a crash on the next iteration.

If you exit the main loop with popups still open, `run()` cleans
them up before returning.

---

## Known issues and notes

- **Popups always open in the center of the screen**, not relative
  to the main window. There is no `withPosition()` method.
- **Popup size is fixed at construction.** If your content needs
  more space, the popup will not grow. Use the `withResizable`
  option and let the user resize, or compute the size ahead of
  time.
- **`Popup::input` does not focus the text input automatically.**
  The user must click into it before typing. A future enhancement
  should focus it on show.
- **`Popup::success` uses the `dialog-information` icon**, same as
  `Popup::info`. There is no dedicated success icon in the
  standard icon theme spec.
- **Icon-based popups fail silently** if no icon theme is loaded.
  No fallback is drawn.
- **`close()` does not immediately destroy the window.** The SDL
  window lingers for one frame until the main loop picks up the
  change. This is usually invisible but can matter if you are
  timing screenshots or window counting.
- **No popup stack.** Multiple modals can be open at once, and
  only the first is returned by `getActiveModal`. In practice you
  should keep at most one modal at a time.

---

## Common patterns

### Simple alert

    Popup::message("Hello", "This is an alert.");

### Confirmation

    Popup::confirm("Delete", "Are you sure?", [](bool ok) {
        if (ok) do_delete();
    });

### Input with default value

    auto* popup = Popup::input("Name", "Enter your name:",
        [](const std::string& name) {
            SDL_Log("Name: %s", name.c_str());
        });

    // Find the text input and set a default
    // (requires iterating the popup's content, or
    //  keeping a pointer from construction)

### Custom popup

    auto* popup = new KitsuPopup(500, 300, "Custom");
    popup->withModal(true);

    // Add whatever widgets you want
    auto* label = new KitsuLabel("Choose an option:");
    popup->add(label);
    popup->addButton("OK", [popup]() { popup->close(); });
    popup->addButton("Cancel", [popup]() { popup->close(); });

---

## See also

- [Widget](../core/widget.md)
- [Icon](icon.md) — used by the icon-based helpers
- [ContextMenu](context_menu.md) — the other "floating" UI element
- [Layouts](../core/layouts.md)
