# Icon

`KitsuIconView` displays a single icon. It supports two sources:

- **FontAwesome glyphs** — drawn from a FontAwesome font, using
  Unicode codepoints.
- **System theme icons** — PNG or SVG files resolved through the
  XDG icon theme spec (Papirus, Breeze, Adwaita, etc.).

    #include <kitsugui/icon.h>

    using namespace KitsuGui;

---

## Quick start

### FontAwesome icon

    auto* icon = new KitsuIconView(0xF00C);   // fa-check
    icon->setBounds(20, 20, 20, 20);
    icon->withColor(80, 180, 80);
    window->add(icon);

### System theme icon

    KitsuIconTheme::instance().load("Papirus");

    auto* icon = new KitsuIconView("dialog-warning", 32,
                                   IconSource::SYSTEM_THEME);
    icon->setBounds(20, 20, 32, 32);
    window->add(icon);

---

## Constructors

    KitsuIconView(uint16_t unicode, int size = 20);
    KitsuIconView(const std::string& name, int size = 20,
                  IconSource src = IconSource::SYSTEM_THEME);

### Unicode constructor

Creates a **FontAwesome** icon from a Unicode codepoint. The icon
uses the global FontAwesome font (set once via
`KitsuIconView::setFont`).

### Name constructor

Creates a **system theme** icon by name, resolved through
`KitsuIconTheme`. The name follows the XDG icon naming spec:

    "dialog-warning", "edit-copy", "document-save",
    "actions/edit-delete", "apps/web-browser", ...

The icon is looked up at render time on the current theme's search
path.

---

## Fluent configuration

    KitsuIconView& withColor(Uint8 r, Uint8 g, Uint8 b);
    KitsuIconView& withSource(IconSource src);
    void setUnicode(uint16_t u);
    void setIconName(const std::string& name);
    void setSize(int s);

### withColor

Sets the RGB color. **Only used for FontAwesome icons.** System
theme icons are always drawn in their original colors.

Default FontAwesome color is `(40, 40, 45)` — dark gray.

### withSource

Switches between `IconSource::FONTAWESOME` and
`IconSource::SYSTEM_THEME`. This invalidates the current texture,
so the icon reloads on the next render.

### setUnicode / setIconName

Change the icon's content. Both also switch the source
automatically. Useful for updating an icon in place:

    icon->setIconName("dialog-error");

### setSize

Changes the icon's size and updates bounds to match. This is a
**destructive** operation for system icons: the cached texture is
destroyed and reloaded at the new size.

---

## Icon source: FontAwesome

FontAwesome glyphs are drawn with `TTF_RenderGlyph_Blended`,
which renders a single Unicode codepoint from a TTF font. This
means the icon is a **vector glyph**, not a bitmap — it scales
cleanly if you change the font size.

### Setup

    TTF_Font* fa = TTF_OpenFont("fa-solid-900.ttf", 20);
    KitsuIconView::setFont(fa);

The font size determines the base glyph size; the widget's `bounds`
controls the display size.

### Finding Unicode values

FontAwesome codepoints are published in the FontAwesome cheatsheet
or in the `fontawesome-free` package under `metadata/icons.yml`.

Common examples:

    fa-check     0xF00C
    fa-times     0xF00D
    fa-home      0xF015
    fa-user      0xF007
    fa-save      0xF0C7
    fa-trash     0xF1F8
    fa-search    0xF002

### Caching

The FA texture is cached on the widget itself and invalidated when:

- The Unicode codepoint changes.
- The size changes.

The color is **not** part of the cache key for FA icons. If you
change the color after first render, you must call
`setUnicode` again (or any other invalidator) for it to take
effect. This is a known limitation.

---

## Icon source: system theme

System icons are resolved via `KitsuIconTheme`, a class that
parses XDG `index.theme` files and searches theme directories in
order. Results are cached globally per renderer.

### Theme loading

    KitsuIconTheme::instance().load("Papirus");

Loads the theme and its inheritance chain (typically ending at
`hicolor`). Loading is a one-time operation; after that, icon
lookups are fast.

Search paths (in order):

1. `~/.icons`
2. `~/.local/share/icons`
3. `$PREFIX/share/icons` (Termux)
4. `/usr/share/icons`
5. `/usr/local/share/icons`
6. `data/icons` (relative to the app)

You can add custom paths:

    KitsuIconTheme::instance().addSearchPath("/my/icons");

The path is added at the **front** of the list, so it takes
priority over the defaults.

### Icon name resolution

`findIcon(name, size)` resolves the name using the XDG spec:

1. Exact match in a directory whose `Size` matches `size`.
2. Match in a directory whose `MinSize..MaxSize` contains `size`.
3. Fallback to the closest directory by absolute size difference.

Subdirectory prefixes (`actions/edit-copy`) are supported and
checked as a nested directory.

Supported extensions: `.png`, `.svg`.

### SVG support

SVG icons are converted to PNG on the fly using
`rsvg-convert`:

    rsvg-convert -w <size> -h <size> -o /tmp/kitsu_icon_XXX.png <src>

**If `rsvg-convert` is not in the PATH**, SVG icons are silently
skipped and only PNG icons are used. Install
`librsvg` / `librsvg2-bin` to enable SVG support.

---

## Caching and ownership

System icons are cached in a global `KitsuIconCache` singleton,
but **only for the main window's renderer**. This is important:

### Main renderer

    cached = true;
    // The cache owns the texture.
    // Do NOT destroy it.

The icon view stores a pointer to the cached texture and does not
free it. It is freed when the cache is cleared (typically in
`~Window`).

### Other renderers (popups)

    cached = false;
    // The icon view owns the texture.
    // It MUST destroy it.

Popups use their own SDL renderer, and textures are not shareable
between SDL renderers. So the cache is bypassed for non-main
renderers, and each popup's icon loads and frees its own texture.

The `KitsuIconView` tracks ownership with the
`owns_system_texture` flag and only destroys the texture if it
owns it.

### Clearing the cache

The cache is cleared automatically when the main window is
destroyed:

    Window::~Window() {
        KitsuIconCache::instance().clear();
        // ...
    }

Do not call `clear()` while icons are still visible; their
pointers will dangle.

---

## Rendering

The icon draws centered in its bounds:

    dst.x = bounds.x + (bounds.w - tex_w) / 2;
    dst.y = bounds.y + (bounds.h - tex_h) / 2;

There is no background, no border, no padding. The icon is a
transparent overlay.

If the texture failed to load (icon not found, no font, no
renderer), nothing is drawn and the widget is effectively
invisible.

---

## Static font (FontAwesome)

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

The FA font is global. Without it, all FA icons silently render
nothing. Set it once at startup:

    KitsuIconView::setFont(TTF_OpenFont("fa-solid-900.ttf", 20));

The icon view does **not** own the font; you close it yourself.

---

## Known issues and notes

- **FontAwesome color changes require invalidation.** Because
  `cached_r/g/b` are not part of the FA cache key, changing
  `withColor()` after the first render has no visual effect. Work
  around it by calling `setUnicode()` again with the same value.
- **System icons ignore `withColor()`.** They render in their
  original colors; the method is a no-op for them.
- **SVG support depends on an external binary.** If
  `rsvg-convert` is missing, SVG icons are silently dropped, which
  can make a theme look incomplete with no error message.
- **No fallback icon.** If the name is not found, nothing is
  drawn.
- **The icon cache key does not include the theme.** If you change
  the icon theme at runtime and reload the cache, all icons update
  correctly, but individual `KitsuIconView`s that already cached
  their pointer keep the old texture. Reload the cache or recreate
  the views.
- **`system()` is used internally to run `rsvg-convert`.** This is
  a shell-invocation pattern; it is safe for constant input (the
  size and path come from the library), but a future version
  should switch to a proper process API.
- **No support for icon sizes larger than the source.** Small
  icons are upscaled with bilinear filtering, which looks soft.

---

## Common patterns

### Toolbar with themed icons

    KitsuIconTheme::instance().load("Papirus");

    auto* hbox = new KitsuHBox();
    hbox->setBounds(0, 0, -1, 40);
    hbox->setSpacing(8);
    hbox->setPadding(4);

    const char* names[] = {
        "document-new",
        "document-open",
        "document-save",
        "edit-undo",
        "edit-redo"
    };

    for (const char* name : names) {
        auto* icon = new KitsuIconView(name, 24,
                                       IconSource::SYSTEM_THEME);
        icon->setBounds(0, 0, 24, 24);
        hbox->addChild(icon, true);
    }

### FontAwesome checkmark in a list

    auto* check = new KitsuIconView(0xF00C, 16);
    check->withColor(76, 175, 80);   // green
    check->setBounds(4, 4, 16, 16);

### Status icon that changes

    status_icon->setIconName("dialog-warning");
    status_icon->withColor(255, 180, 0);   // (only works for FA)

---

## See also

- [Widget](../core/widget.md)
- [Popup](popup.md) — uses icons for dialog helpers
- [Theming](../theming.md) — icon themes are configured through
  `KitsuConfig`
