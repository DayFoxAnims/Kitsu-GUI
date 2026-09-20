# Theming

KitsuGui uses a single global theme object, accessible through
`KitsuTheme::current()`. Every widget that reads colors,
dimensions, or radii from the theme consults this object at
render time.

There is no per-widget theme override that actually works — the
`withTheme(Theme)` methods on some widgets are stubs that store a
value but ignore it. Real theming happens by **changing the global
theme**.

    #include <kitsugui/theme.h>

    using namespace KitsuGui;

---

## The theme object

    struct KitsuTheme {
        std::string name;
        std::string author;
        bool is_dark;

        std::unordered_map<std::string, Color> palette;

        // Semantic tokens
        Color bg_primary, bg_secondary, bg_tertiary, bg_disabled;
        Color text_primary, text_secondary, text_disabled, text_on_accent;
        Color accent, accent_hover, accent_pressed, accent_disabled;
        Color border, border_focus, border_disabled;
        Color success, warning, error, info;
        Color scrollbar_track, scrollbar_thumb, scrollbar_thumb_hover;
        Color selection, shadow;

        // Numeric tokens
        float radius_small, radius_medium, radius_large, radius_pill;
        int spacing_small, spacing_medium, spacing_large, spacing_xlarge;
        int border_thin, border_normal, border_thick;

        // Per-widget radii
        float radius_checkbox, radius_switch, radius_switch_knob;
        float radius_slider_track, radius_slider_knob, radius_radio;
        float radius_button, radius_panel, radius_text_input;
        float radius_viewport, radius_progress_bar;

        // Per-widget border thickness
        int border_thickness_button, border_thickness_checkbox;
        int border_thickness_switch, border_thickness_text_input;
        int border_thickness_panel, border_thickness_viewport;

        // Per-widget dimensions
        int checkbox_size, switch_width, switch_height, switch_knob_pad;
        int slider_track_thickness, slider_knob_size, radio_size;
        int text_input_height, button_height;

        // Typography
        std::string font_family, font_mono;
        int font_size_small, font_size_normal, font_size_large;
        int font_size_title, font_size_huge;

        // ...
    };

---

## Using the current theme

Read values directly:

    Color bg = KitsuTheme::current().bg_primary;
    int radius = KitsuTheme::current().radius_button;

Or bind a widget to a theme token:

    window->setBackground(KitsuTheme::current().bg_primary);
    panel->withBackground(KitsuTheme::current().bg_secondary);

> **Warning:** binding a widget to a theme value is a **snapshot**,
> not a live link. If you set the background from
> `KitsuTheme::current().bg_secondary` and later change the theme,
> the widget keeps the old color. Re-apply the value after theme
> changes if you want it to follow.

Some widgets read directly from `KitsuTheme::current()` on every
render (buttons, switches, radio buttons, sliders, progress bars).
Those follow the theme automatically. Others capture values at
construction and require manual updates. When in doubt, recreate
the widget or re-apply the property.

---

## Built-in themes

Two themes ship with KitsuGui:

### KitsuMetro Light

    KitsuTheme t = KitsuTheme::KitsuMetroLight();

- Warm off-white background (`245, 240, 235`).
- Orange accent (`255, 136, 0`).
- Pure square corners everywhere (all radii = 0).
- Thin borders, medium-weight typography.

### KitsuMetro Dark

    KitsuTheme t = KitsuTheme::KitsuMetroDark();

- Dark warm-gray background.
- Same orange accent.
- Square corners.
- Lighter borders.

Both share the same layout tokens and differ only in color.

---

## Applying a theme

    KitsuTheme::apply(KitsuTheme::KitsuMetroDark());

Applying a theme:

1. Replaces the global `s_current_theme`.
2. **Does not** notify or refresh any widget.

Widgets that read `KitsuTheme::current()` on every frame pick up
the change on the next render. Widgets that captured values at
construction keep their old colors.

To force a full redraw after applying:

    KitsuTheme::apply(KitsuTheme::KitsuMetroDark());
    if (g_renderer) g_renderer->markAllDirty();

This is not automatic — you must do it yourself. A future release
may add automatic notification.

---

## Palette

The `palette` map holds raw colors used as building blocks:

    KitsuTheme::KitsuMetroLight().palette["orange"];     // (255,136,0)
    KitsuTheme::KitsuMetroLight().palette["gray_400"];   // (150,150,160)

Palette entries are not used directly by widgets; only the
semantic tokens are. The palette exists so you can build custom
themes by referencing named colors.

---

## Loading a theme from a file

    bool ok = KitsuTheme::loadFromFile("my_theme.theme");

The file format is INI-like:

    [metadata]
    name = My Theme
    author = DayFox
    is_dark = true

    [palette]
    orange = #FF8800
    dark_bg = #1A1816

    [tokens]
    bg_primary = #1A1816
    bg_secondary = #23201E
    accent = @palette:orange
    text_primary = #F0F0F5
    border = #3C3A37

    [style]
    radius_button = 8
    radius_panel = 12

    [sizes]
    button_height = 44

    [typography]
    font_size_normal = 15

Loading a file:

- Replaces the current theme **immediately**. There is no
  "preview" mode.
- Silently ignores unknown keys and malformed values.
- Returns `false` if the file cannot be opened.

---

## Saving a theme to a file

    KitsuTheme::current().saveToFile("my_theme.theme");

Dumps the current theme in the same format. Useful as a starting
point for customizing.

---

## Color resolution

The theme supports three color syntaxes in files:

### Direct hex

    bg_primary = #FF8800
    bg_primary = #F80           (short form, same as #FF8800)
    bg_primary = #FF880080      (with alpha)

### Palette references

    accent = @palette:orange

Resolves to the palette entry named `orange`. If the palette key
does not exist, the color becomes magenta (`255, 0, 255`) as a
visible error marker.

### Named tokens

Tokens like `bg_primary`, `border_focus`, etc. map to fields on
the `KitsuTheme` struct. Unknown tokens are silently ignored.

---

## Runtime helpers

### setToken

    bool KitsuTheme::setToken(const std::string& key,
                              const std::string& value);

Sets a single token. Returns `true` if the key was recognized.

    KitsuTheme::current().setToken("accent", "#00AAFF");
    KitsuTheme::current().setToken("radius_button", "6");

> **Note:** this is a non-const method on a non-const reference.
> It works because `KitsuTheme::current()` returns a non-const
> reference. Other code reading the theme will see the change on
> the next access.

### setPaletteColor

    bool KitsuTheme::setPaletteColor(const std::string& name,
                                     const std::string& hex);

Adds or replaces a palette entry.

### resolveColor

    Color KitsuTheme::resolveColor(const std::string& value) const;

Parses a color string (`#hex` or `@palette:key`) into a `Color`.
Useful if you are writing your own theme loader.

---

## Integration with KitsuConfig

`KitsuConfig` provides an application-level configuration layer on
top of the theme:

    KitsuConfig& config = KitsuConfig::instance();
    config.load();              // reads user settings
    config.applyTheme();        // applies the chosen theme
    config.applyIconTheme();    // loads the chosen icon theme

See the `KitsuConfig` documentation (pending) for details.

A typical startup looks like:

    int main() {
        KitsuConfig& config = KitsuConfig::instance();
        config.load();
        config.applyTheme();
        config.applyIconTheme();

        Window win(800, 600, "My App");
        win.setBackground(KitsuTheme::current().bg_primary);

        // ...
    }

---

## Known limitations

- **No automatic widget refresh on theme change.** Widgets read
  the theme either at construction (and never again) or at every
  render (and always). Mixed behavior means some widgets update
  and others do not. See above for how to force a redraw.
- **`withTheme(Theme)` methods on widgets are stubs.** They store
  a value that is never read. Do not rely on them.
- **No partial theme overrides.** You cannot say "use KitsuMetro
  Light but with a purple accent" without editing the theme
  object directly.
- **No dark/light auto-switching.** The `is_dark` flag is
  informational; nothing reacts to it.
- **Fonts are not managed by the theme.** The `font_family` and
  `font_size_*` fields are descriptive; each widget has its own
  static font pointer you set manually.
- **No CSS-like cascading.** Every widget reads what it needs
  from the global theme. There is no inheritance or override
  chain.

---

## See also

- [Background & Color](core/background.md)
- [Window](core/window.md)
- [Widget](core/widget.md)
- [Config](config.md) *(to be documented)*
