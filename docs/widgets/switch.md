# Switch

`KitsuSwitch` is a labeled toggle switch: an on/off control with a
sliding knob, visually distinct from a checkbox. It is functionally
equivalent to `KitsuCheckBox` — the difference is purely visual.

    #include <kitsugui/switch.h>

    using namespace KitsuGui;

---

## CheckBox or Switch?

Both widgets represent a boolean state and share the same API
surface (`withChecked`, `withCallback`, `toggle`). The choice is
aesthetic:

- **CheckBox** — traditional, compact, familiar on desktop.
- **Switch** — more modern, reads like a light switch, common in
  mobile and settings panels.

You can use them interchangeably in your layout.

---

## Quick start

    auto* sw = new KitsuSwitch("Dark mode");
    sw->setBounds(20, 20, 220, 22)
       ->withChecked(false)
       ->withCallback([](bool on) {
           SDL_Log("Dark mode: %s", on ? "on" : "off");
       });
    window->add(sw);

---

## Constructor

    KitsuSwitch(const std::string& text = "",
                bool checked = false);

- `text` — the label shown to the right of the switch.
- `checked` — initial state. Defaults to `false`.
- Default bounds: `{0, 0, 220, 22}`.

> The constructor uses a **hardcoded height of 22px** for the
> default. The actual rendering height comes from
> `KitsuTheme::switch_height`, which is also 22 by default. If you
> change the theme's switch height, you should also update the
> bounds manually.

---

## Fluent configuration

    KitsuSwitch& withFont(TTF_Font* font);
    KitsuSwitch& withChecked(bool c);
    KitsuSwitch& withCallback(std::function<void(bool)> cb);
    KitsuSwitch& disabled();
    KitsuSwitch& setBounds(int x, int y, int w, int h);

Same signature as `KitsuCheckBox`. See
[CheckBox](check_box.md#fluent-configuration) for details on each
method — the semantics are identical.

### disabled

> **Known issue:** like the checkbox, `disabled()` sets the private
> `enabled_` flag, not the public `KitsuWidget::enabled`. Only the
> private flag is respected by this widget.

---

## State and toggling

    bool isChecked() const;
    void setChecked(bool c);
    void toggle();

Identical to the checkbox: `toggle()` calls
`setChecked(!checked)`, and setting the same value is a no-op.

The callback receives the new boolean state on every actual change.

---

## Events

The switch reacts to the same events as the checkbox:

- `SDL_MOUSEMOTION` — hover state.
- `SDL_MOUSEBUTTONDOWN` (left, inside) — records the press.
- `SDL_MOUSEBUTTONUP` (left, inside, after a press) — toggles.

A press that ends outside the switch is cancelled.

---

## Rendering

The switch draws:

1. **Track** — a rounded rectangle sized from
   `KitsuTheme::switch_width` × `switch_height` (default 44×22).
   Colors depend on state:

   | State          | Track fill    | Track border     | Knob color       |
   |----------------|---------------|------------------|------------------|
   | Checked        | `accent`      | `accent`         | `text_on_accent` |
   | Hover          | `bg_tertiary` | `accent`         | `accent`         |
   | Normal         | `bg_tertiary` | `border`         | `border`         |
   | Disabled       | `bg_disabled` | `border_disabled`| `text_disabled`  |

2. **Knob** — a **square** (not a circle), placed on the left when
   unchecked and on the right when checked. The knob size is
   `(switch_width / 2) - 2 * switch_knob_pad`, and its height is
   `switch_height - 2 * switch_knob_pad`.

   The knob uses the same `radius_switch` from the theme as the
   track, so in KitsuMetro (radius = 0) it is a pure square, and in
   a rounded theme it would be a rounded rect.

3. **Label** — the text, rendered to the right of the track.

The switch does **not** animate the knob's movement. It jumps from
one position to the other instantly.

---

## Theme integration

Unlike some other widgets, the switch reads most of its dimensions
from the theme at render time:

- `KitsuTheme::switch_width`
- `KitsuTheme::switch_height`
- `KitsuTheme::switch_knob_pad`
- `KitsuTheme::radius_switch`
- `KitsuTheme::border_thickness_switch`

This means changing the theme at runtime will affect the switch
**on the next frame**, without needing to recreate it. The label
color and text also follow the theme.

---

## Common patterns

### Settings toggle

    auto* sw = new KitsuSwitch("Wi-Fi");
    sw->withChecked(true)
      ->withCallback([](bool on) {
          set_wifi_enabled(on);
      });
    settings_panel->addChild(sw, true);

### Read-only display

There is no "read-only" mode. If you want a switch that shows a
state but cannot be changed, disable it:

    sw->disabled();

It will render greyed out and ignore clicks.

---

## Memory and ownership

- The switch owns its **text texture**.
- The switch does **not** own its **font**.
- Ownership of the switch itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [CheckBox](check_box.md) — same behavior, different look
- [Radio](radio.md) — for mutually exclusive options
- [Theming](../theming.md) *(to be documented)*
