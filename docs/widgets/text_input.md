# TextInput

`KitsuTextInput` is an editable single-line text field. It supports
UTF-8 input, text selection with the mouse, clipboard operations,
a blinking caret, and its own right-click context menu.

    #include <kitsugui/text_input.h>

    using namespace KitsuGui;

---

## Quick start

    auto* input = new KitsuTextInput();
    input->setBounds(20, 20, 250, 32)
         ->withPlaceholder("Type your name...")
         ->withOnEnter([](const std::string& text) {
             SDL_Log("Entered: %s", text.c_str());
         });
    window->add(input);

The input handles focus, selection, and editing automatically. Set
the global font once at startup with `KitsuTextInput::setFont`.

---

## Constructor

    KitsuTextInput(const std::string& text = "");

- `text` — initial content. The cursor is placed at the end.
- Default size: `250 x 32`.
- Default placeholder: empty (no hint shown).
- Colors are captured from `KitsuTheme::current()` at construction
  time (see [Theme binding](#theme-binding)).

The constructor also creates a private `KitsuContextMenu` with four
items: Copy, Cut, Paste, Select All. You can access it with
`getContextMenu()` but you should not need to.

---

## Focus

Only **one text input can be focused at a time**, enforced by a
static pointer (`s_focused`). When a new input gets focus, the
previous one is unfocused automatically.

    void setFocus(bool f);
    void unfocus();
    bool isFocused() const;
    static KitsuTextInput* getFocused();

Focusing an input:

1. Unfocuses the previously focused input (if any).
2. Calls `SDL_StartTextInput()` to enable platform text input
   (this is what opens the on-screen keyboard on mobile).
3. Starts the caret blink timer.
4. Increments `g_active_animations` so the main loop keeps running
   at full speed while the caret blinks.

Unfocusing reverses all of that, including `SDL_StopTextInput()`
and clearing any active selection.

---

## Fluent configuration

    KitsuTextInput& withFont(TTF_Font* font);
    KitsuTextInput& withText(const std::string& text);
    KitsuTextInput& withPlaceholder(const std::string& text);
    KitsuTextInput& withCallback(std::function<void(const std::string&)> cb);
    KitsuTextInput& withOnEnter(std::function<void(const std::string&)> cb);
    KitsuTextInput& withTheme(Theme t);
    KitsuTextInput& setBounds(int x, int y, int w, int h);

### withText vs setText

`withText` is a thin wrapper around `setText`. Both:

- Replace the current content.
- Reset the cursor to the end.
- Clear any selection.
- Fire the `callback` (see below).

### withCallback

    std::function<void(const std::string&)> callback;

Called on **every text change**: typing, deleting, pasting,
cutting, replacing via `setText`. Receives the new full text.

Use it for live validation, search-as-you-type, autosave, etc.

### withOnEnter

    std::function<void(const std::string&)> on_enter;

Called when the user presses **Enter** or **Numpad Enter** while
the input is focused. Does **not** insert a newline (the input is
single-line).

### withTheme

> **Not implemented yet.** Like `KitsuButton::withTheme`, this
> method stores a `Theme` value but the color update function
> always reads from `KitsuTheme::current()`, ignoring the stored
> theme. Reserved for future per-widget overrides.

---

## Reading and writing text

    std::string getText() const;
    void setText(const std::string& t);

`setText` replaces the whole content and fires the callback.
Calling it with the same text is **not** a no-op — the callback
still fires. If you need a silent update, keep a local flag and
compare before calling.

---

## Editing operations (public API)

    void selectAll();
    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboard();

These are the same operations the context menu and keyboard
shortcuts trigger. They use the system clipboard via
`SDL_SetClipboardText` / `SDL_GetClipboardText`.

- `selectAll` selects the whole text and places the cursor at the
  end. Does nothing if the text is empty.
- `copyToClipboard` does nothing if there is no selection.
- `cutToClipboard` copies and then deletes the selection.
- `pasteFromClipboard` inserts the clipboard content at the cursor,
  replacing any active selection. Fires the callback.

---

## Keyboard shortcuts

While focused, the input handles:

| Key                    | Action                          |
|------------------------|---------------------------------|
| `Ctrl+A`               | Select all                      |
| `Ctrl+C`               | Copy selection                  |
| `Ctrl+V`               | Paste                           |
| `Ctrl+X`               | Cut selection                   |
| `Backspace`            | Delete char/selection before cursor |
| `Delete`               | Delete char/selection after cursor  |
| `Left` / `Right`       | Move cursor (or collapse selection) |
| `Home` / `End`         | Jump to start / end             |
| `Enter` / `Numpad Enter` | Fire `on_enter`               |

Alt-modified keys are ignored (so you can use `Alt+Tab`, etc.).
Undo/redo is **not implemented**.

---

## Mouse interaction

- **Left click** inside → focus + place cursor at the click
  position.
- **Left drag** → extend the selection.
- **Right click** inside → focus + open the context menu at the
  click position.
- **Mouse leave while selecting** → selection continues to update
  (clamped to the input's bounds).

The context menu is auto-closed when:

- The user clicks outside it.
- The user presses `ESC`.
- An item is activated (item runs its action and closes the menu).
- The input is destroyed (see [Menus and focus](#menus-and-focus)).

---

## UTF-8 handling

The cursor, selection, and text width calculations are all done in
**byte offsets**, but always aligned to UTF-8 character boundaries.
Two helpers prevent the caret from landing inside a multi-byte
sequence:

    int prevUtf8Index(int i) const;
    int nextUtf8Index(int i) const;

This means arrow keys, backspace, and delete move by **whole
characters**, not bytes, even for accents, emoji, and CJK input.

Cursor position measured in pixels uses SDL_ttf:

    int textWidthUpTo(int i) const;

which re-renders `text.substr(0, i)` to measure. This is exact but
not free; for very long strings in very tight loops, this could be
a bottleneck. In practice, single-line inputs stay short enough
that this is fine.

---

## Rendering

The input draws five layers, in order:

1. **Background** — solid rectangle, `bg_tertiary` from the theme.
2. **Selection** — a translucent rectangle over the selected text
   range (`selection_color`, default semi-transparent blue).
3. **Border** — two concentric `SDL_RenderDrawRect` calls (2px
   total). Uses `border_focus` when focused, `border` otherwise.
   The corners are **not rounded**.
4. **Text** — either the current content or the placeholder
   (rendered in `text_secondary`) if the content is empty.
5. **Caret** — a vertical 2px line at the cursor position, drawn
   only when the input is focused, the caret is in its visible
   blink phase, and there is no active selection.

Text is always drawn at `x + 8` inside the input rect, and
vertically centered.

---

## Caret blinking

The caret toggles every 500 ms while the input is focused. The
blink is driven by `tick()`, which is called once per frame by the
main loop.

Because blinking requires the loop to keep running, focusing an
input increments `g_active_animations`. This tells the loop to
keep waking up regularly instead of blocking on `SDL_WaitEvent`.
Unfocusing decrements it.

---

## Text caching

The text texture is cached and invalidated when any of these
change:

- The display string (text or placeholder).
- The RGB color.
- The active font pointer.

The placeholder uses its own color, so switching between "empty
with placeholder" and "has content" triggers a rebuild only when
needed.

---

## Menus and focus

The input owns a private `KitsuContextMenu` created in the
constructor. When you right-click inside the input, that menu is
shown via `KitsuContextMenu::showAt()`, which registers it as the
**active menu** in a global singleton.

This has two consequences:

1. **Only one context menu can be open at a time** across the
   whole app. Opening the input's menu while a dropdown's menu is
   open will close the dropdown's.
2. **The input's menu must be hidden before the input is
   destroyed**, or the singleton will hold a dangling pointer. The
   destructor should call `hide()` on the menu if it is the active
   one.

If you spawn a second `KitsuTextInput` while the first one's menu
is open, the first input is still the owner of that menu. Focus
moves to the new input, but the menu remains attached to the old
one until it closes.

---

## Known issues and notes

- **`indexFromX` uses `bounds.x` instead of `getAbsoluteBounds().x`.**
  If the input is inside a layout with a non-zero parent offset,
  the click-to-cursor mapping will be off by the parent's offset.
  Workaround: place inputs directly on the root or on a box whose
  origin is `(0, 0)`.
- **Two "enabled" flags exist.** `KitsuWidget::enabled` is public
  and is the one normally used; `KitsuTextInput::enabled_` is
  private and defaults to `true`. Only the private one is checked
  in `handleEvent`. Setting `enabled = false` on the input will
  **not** disable it. This is a bug that will be unified in a
  future release.
- **The context menu labels are in Spanish** ("Copiar", "Cortar",
  "Pegar", "Seleccionar todo"). They are hardcoded; no
  localization system yet.
- **`withTheme` stores the theme but does not use it.** See above.

---

## Memory and ownership

- The input owns its **text texture**.
- The input owns its **context menu** and destroys it in the
  destructor.
- The input does **not** own its **font**.
- Ownership of the input itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [Layouts](../core/layouts.md)
- [ContextMenu](context_menu.md) *(to be documented)*
- [Label](label.md)
- [Button](button.md)
