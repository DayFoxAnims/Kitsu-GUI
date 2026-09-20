# Radio

Radio buttons let the user pick **one option out of a set**. Unlike
checkboxes and switches, radio buttons are not independent: when
one is selected, the others in the same group are automatically
deselected.

KitsuGui provides two classes for this:

- **`KitsuRadioButton`** — a single option.
- **`KitsuRadioGroup`** — a container that enforces mutual
  exclusivity among its buttons.

    #include <kitsugui/radio.h>

    using namespace KitsuGui;

---

## Quick start

    auto* group = new KitsuRadioGroup();
    group->setBounds(20, 20, 220, 100);

    group->addRadio(new KitsuRadioButton("Small"));
    group->addRadio(new KitsuRadioButton("Medium", true));
    group->addRadio(new KitsuRadioButton("Large"));

    group->setOnChange([](KitsuRadioButton* r) {
        SDL_Log("Selected: %s", r->getText().c_str());
    });

    window->add(group);

The group lays out the buttons vertically and handles the
mutual-exclusion logic. The initial selection is "Medium" (the one
created with `checked = true`).

---

## KitsuRadioButton

### Constructor

    KitsuRadioButton(const std::string& text = "",
                     bool checked = false);

- `text` — the label shown to the right of the circle.
- `checked` — initial state. Defaults to `false`.
- Default bounds: `{0, 0, 220, 24}`.

The button does **not** fire its callback for the initial state.

### Fluent configuration

    KitsuRadioButton& withFont(TTF_Font* font);
    KitsuRadioButton& withChecked(bool c);
    KitsuRadioButton& withCallback(std::function<void(bool)> cb);
    KitsuRadioButton& setBounds(int x, int y, int w, int h);

Same shape as the checkbox. See
[CheckBox](check_box.md#fluent-configuration) for details.

### State

    bool isChecked() const;
    void setChecked(bool c);

Unlike a checkbox, `KitsuRadioButton` has **no `toggle()`** and no
`disabled()` fluent method. Radio buttons are not meant to be
toggled off by clicking them again; that is the group's job.

### Callback

    std::function<void(bool)> callback;

Fired only with `true`, and only when the button transitions from
unchecked to checked. Since radio buttons in a group can only be
selected, not deselected, the callback never fires with `false` in
normal use.

If a button is created with `checked = true` and then added to a
group, the initial `true` does **not** fire the callback.

### Standalone use

A `KitsuRadioButton` **without** a group behaves almost like a
checkbox: clicking it toggles it on, and clicking again does
nothing (it never turns off). This is rarely what you want — use a
group unless you have a specific reason not to.

---

## KitsuRadioGroup

### Constructor

    KitsuRadioGroup();

Inherits from `KitsuBox` (vertical), so it inherits all the layout
machinery: `setPadding`, `setSpacing`, `setAlignment`,
`setJustify`, etc.

Defaults:

- Vertical orientation.
- `spacing = 6`.
- `padding = 0`.
- `alignment = START`, `justify = START`.
- Auto-layout enabled.

### Adding radios

    void addRadio(KitsuRadioButton* radio, bool owns = true);
    void removeRadio(KitsuRadioButton* radio);

- `addRadio(radio, owns = true)` — attaches the button to the
  group. If `owns` is true, the group deletes the button in its
  destructor and on removal.
- If the added button is already checked, any previously selected
  button is unchecked, and the new one becomes the selection.
- The group calls `radio->setGroup(this)` internally, wiring up
  the exclusivity.

### Removing radios

    void removeRadio(KitsuRadioButton* radio);

Removes the button from the group. If it was owned, it is deleted.
If it was selected, the group's selection becomes `nullptr`.

### Getting the selection

    KitsuRadioButton* getSelected() const;

Returns the currently selected radio, or `nullptr` if none is
selected.

### The change callback

    void setOnChange(std::function<void(KitsuRadioButton*)> cb);

Fired when the selection changes through user interaction. Receives
the newly selected radio.

> **Important:** `setOnChange` fires **only when the user clicks a
> radio**. If you change the selection programmatically with
> `setChecked`, the group's callback is **not** fired. This is a
> real limitation; if you need to observe programmatic changes,
> subscribe to each radio's own `withCallback` instead.

### The selection flow

When the user clicks an unselected radio:

1. The radio's `handleEvent` receives the click.
2. If the radio belongs to a group, it calls
   `group->notifySelected(this)`.
3. The group unchecks the previously selected radio (if any).
4. The group calls `activateFromGroup()` on the new radio, which
   sets its state to checked and fires its own callback with
   `true`.
5. The group fires its `on_change` callback.
6. All affected widgets are marked dirty.

Clicking an already-selected radio does nothing.

---

## Rendering

### KitsuRadioButton

Draws, in order:

1. **Outer circle** — filled with the state-dependent border color.
   Radius from `KitsuTheme::radio_size / 2 - 1`.
2. **Inner fill** — a smaller circle, inset by 2px, filled with the
   background color.
3. **Center dot** — only if checked. A small filled circle in
   `accent`.
4. **Label** — the text, rendered to the right of the circle.

Color table:

| State          | Outer (border)    | Inner (fill)   | Dot          | Text          |
|----------------|-------------------|----------------|--------------|---------------|
| Normal         | `border`          | `bg_secondary` | `accent`     | `text_primary`|
| Hover          | `accent`          | `bg_secondary` | `accent`     | `accent`      |
| Checked        | `accent`          | `bg_secondary` | `accent`     | `text_primary`|
| Disabled       | `border_disabled` | `bg_disabled`  | `text_disabled` | `text_disabled` |

> Note: `KitsuTheme::radio_size` is the total diameter of the
> widget. The circle is always drawn as a perfect circle; the
> theme's `radius_radio` field is unused by this widget (it exists
> only as a hint for future themes that might want to render radio
> buttons differently).

### KitsuRadioGroup

The group is a `KitsuBox` and does not draw anything itself. Only
its children (the radios) render. This means you can put the group
inside a `KitsuPanel` to give it a background, or leave it
transparent inside another container.

---

## Layout

Because `KitsuRadioGroup` inherits from `KitsuBox`, its children
are laid out using the standard box algorithm. By default:

- Vertical stack.
- 6px spacing between radios.
- Alignment `START` (left).

Each radio's `bounds.w` is determined by the box's layout rules.
Set a large enough width so the labels are not cut off, or use
`KitsuAlign::STRETCH` to fill the container.

Example with a fixed-width group:

    auto* group = new KitsuRadioGroup();
    group->setBounds(20, 20, 220, 0);       // height auto
    group->setAutoLayout(true);

    group->addRadio(new KitsuRadioButton("Small"));
    group->addRadio(new KitsuRadioButton("Medium"));
    group->addRadio(new KitsuRadioButton("Large"));

The group will size itself to `220 x (3*24 + 2*6 + padding)`.

---

## Static font

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

Radio buttons use a global default font if they don't specify their
own. Set it once at startup:

    KitsuRadioButton::setFont(my_font);

Without a font, labels are not rendered.

---

## Common patterns

### Settings with a default

    auto* theme_group = new KitsuRadioGroup();
    theme_group->addRadio(new KitsuRadioButton("Light"));
    theme_group->addRadio(new KitsuRadioButton("Dark", true));
    theme_group->addRadio(new KitsuRadioButton("System"));
    theme_group->setOnChange([](KitsuRadioButton* r) {
        apply_theme(r->getText());
    });

### Preselecting programmatically

    group->addRadio(new KitsuRadioButton("Small"));
    auto* medium = new KitsuRadioButton("Medium");
    group->addRadio(medium);
    group->addRadio(new KitsuRadioButton("Large"));

    // Later, to programmatically select:
    medium->setChecked(true);

> This does **not** fire the group's `on_change`. If you need the
> callback, call it yourself after `setChecked`.

### Reading the selection

    if (auto* sel = group->getSelected()) {
        SDL_Log("Selected: %s", sel->getText().c_str());
    }

---

## Known issues and notes

- **`setOnChange` does not fire for programmatic changes.** Only
  user clicks propagate to the group's callback. Subscribe to each
  radio's `withCallback` if you need to observe all changes.
- **Radio buttons have no `disabled()` fluent method** and no
  public way to disable them individually. Setting
  `enabled = false` on a radio has no effect, because the private
  `enabled_` flag is what's checked.
- **No `getText()` accessor is declared on `KitsuRadioButton`** in
  the current header, though the widget stores the text. You can
  read it via a friend function or by tracking it in your own
  code. This is a small gap in the API.
- **The group does not re-run layout when a radio is removed** in
  the current implementation. If you remove a radio and the layout
  looks wrong, call `group->updateLayout()` manually.
- **`setChecked` on a radio inside a group bypasses the group's
  exclusivity.** If you call `radio->setChecked(true)` directly,
  the previously selected radio is **not** automatically
  unchecked. Always go through `group->notifySelected()` or
  equivalent logic in your app.

---

## Memory and ownership

- `KitsuRadioButton` owns its **text texture**.
- `KitsuRadioButton` does **not** own its **font**.
- `KitsuRadioGroup` owns its radios if they were added with
  `owns = true` (the default) and deletes them in its destructor.
- The group itself does not own its parent container.

---

## See also

- [Widget](../core/widget.md)
- [CheckBox](check_box.md) — for independent options
- [Switch](switch.md) — alternative visual for booleans
- [Layouts](../core/layouts.md)
