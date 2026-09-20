# Panel

`KitsuPanel` is a **layout container with a background and an
optional border**. It extends `KitsuBox`, which means it positions
its children the same way a box does, but it also draws a rounded
rectangle behind them.

Think of a panel as "a box that looks like a card".

    #include <kitsugui/panel.h>

    using namespace KitsuGui;

---

## Quick start

    auto* panel = new KitsuPanel();
    panel->setBounds(0, 0, -1, -1);   // fill the parent
    panel->setPadding(40);
    panel->setSpacing(20);
    panel->setAlignment(KitsuAlign::CENTER);
    panel->setJustify(KitsuJustify::CENTER);

    auto* title = new KitsuLabel("Welcome");
    panel->addChild(title, true);

    window->add(panel);

The panel is **vertical by default** (unlike `KitsuBox`, which is
horizontal by default — see the warning below).

---

## Constructor

    KitsuPanel(bool horizontal = false);

> **Note the default.** `KitsuBox` defaults to **horizontal**,
> but `KitsuPanel` defaults to **vertical**. This is intentional
> (panels are usually stacked columns), but easy to miss.
>
> To create a horizontal panel:
>
>     KitsuPanel h(true);

Inside the constructor:

1. `KitsuBox(horizontal)` runs first.
2. Alignment is set to `CENTER`, justify to `CENTER`.
3. Padding defaults to `20`, spacing to `12`.
4. Background, border, and corner radius are captured from
   `KitsuTheme::current()` (see [Theme binding](#theme-binding)).

---

## Inherited behavior from KitsuBox

Because it is a `KitsuBox`, a panel gives you all the layout
methods:

- `addChild(child, owns)` / `removeChild` / `clearChildren`
- `setAlignment`, `setJustify`
- `setSpacing`, `setPadding`, `setMargin`
- `setAutoLayout`
- `getChildren`, `isHorizontal`, `getSpacing`, `getPadding`

See [Layouts](../core/layouts.md) for the full reference. All the
sizing, alignment, and distribution rules described there apply
verbatim to panels.

---

## Fluent helpers

`KitsuPanel` provides fluent versions of the layout setters that
otherwise return `void`:

    KitsuPanel& setBounds(int x, int y, int w, int h);
    KitsuPanel& setPadding(int p);
    KitsuPanel& setSpacing(int s);
    KitsuPanel& setMargin(int m);
    KitsuPanel& withPadding(int p);   // alias for setPadding
    KitsuPanel& withSpacing(int s);   // alias for setSpacing
    KitsuPanel& withMargin(int m);    // alias for setMargin

This lets you configure a panel inline:

    auto* p = new KitsuPanel();
    p->setBounds(20, 20, 400, 300)
     ->setPadding(24)
     ->setSpacing(12)
     ->setAlignment(KitsuAlign::CENTER)
     ->setJustify(KitsuJustify::CENTER);

---

## Appearance overrides

By default, the panel follows the current theme for background,
border, and corner radius. You can override any of these
individually:

    KitsuPanel& withBackground(const Color& c);
    KitsuPanel& withBorder(const Color& c, int thickness = 1);
    KitsuPanel& withCorner(float radius);

Calling any of these **detaches that property from the theme**:
the panel keeps using the explicit value regardless of later theme
changes.

    // Custom dark card, keeps theme border and radius
    panel->withBackground(Color(30, 30, 30));

    // Fully custom: background, border, and corners
    panel->withBackground(Color(30, 30, 30))
         ->withBorder(Color(200, 200, 200), 2)
         ->withCorner(12.0f);

There is **no method to re-attach** a property to the theme. If
you need the theme value again, read it manually from
`KitsuTheme::current()` and pass it explicitly:

    panel->withBackground(KitsuTheme::current().bg_secondary);

---

## Theme binding

The panel stores whether each of the three appearance properties
is currently bound to the theme:

    bool use_theme_bg     = true;   // background
    bool use_theme_border = true;   // border color AND thickness
    bool use_theme_radius = true;   // corner radius

These flags start as `true`. They flip to `false` the moment you
call the corresponding `with*` method.

Note that **border color and border thickness share a single
flag**. Calling `withBorder(color)` or `withBorder(color, 1)` both
detach the whole border from the theme. If you only wanted to
change the color and keep the theme's thickness, you can't — you
must supply a thickness explicitly.

The same happens on the **background side**: the constructor reads
`bg_secondary`, `border`, `border_normal`, and `radius_medium` at
construction time. If you change the theme later and never called
any `with*` method, the panel will **still use the original
values captured at construction**.

> **Practical rule:** if your app supports runtime theme
> switching, do not rely on panels picking up the new theme
> automatically. Either recreate them or call the corresponding
> `with*` methods with the new theme values.

---

## Rendering

The panel draws, in order:

1. **Background** — a rounded rectangle at the full bounds.
2. **Border** — if `thickness > 0`, the panel draws a second
   rounded rect (the border color) and then a third one (the
   background color) inset by `thickness`. This produces a crisp
   border without depending on SDL_ttf or `SDL_RenderDrawRect`
   with rounded corners.
3. **Children** — via `KitsuBox::render`, which iterates over
   visible children.

The inner rect uses `SDL_max(0.0f, radius - thickness)` as its
corner radius so the inner edge stays smooth even for large
borders.

If `thickness == 0`, the border is skipped entirely.

---

## Common patterns

### A card

    auto* card = new KitsuPanel();
    card->setBounds(0, 0, 300, 200)
        ->setPadding(24)
        ->setSpacing(12)
        ->setAlignment(KitsuAlign::START)
        ->setJustify(KitsuJustify::START);
    card->withCorner(12);

### A modal-like dialog

    auto* dialog = new KitsuPanel();
    dialog->setBounds(100, 100, 400, 250)
          ->setPadding(20)
          ->setSpacing(16);
    dialog->setAlignment(KitsuAlign::CENTER);
    dialog->setJustify(KitsuJustify::CENTER);

    // title, message, buttons, ...
    window->add(dialog);

For true modals with input blocking, see `KitsuPopup`
(documentation pending).

### A centered content area

    auto* area = new KitsuPanel();
    area->setBounds(0, 0, -1, -1)
        ->setPadding(40)
        ->setSpacing(20);
    area->setAlignment(KitsuAlign::CENTER);
    area->setJustify(KitsuJustify::CENTER);

    window->add(area);

---

## Memory and ownership

`KitsuPanel` inherits `KitsuBox`'s ownership model:

- Children added with `addChild(child, owns = true)` (the default)
  are deleted by the panel in its destructor and on `removeChild`.
- Children added with `owns = false` are not deleted; their
  `parent` pointer is cleared on removal instead.

The panel does not own the colors it is given; `Color` is a value
type.

---

## See also

- [Layouts](../core/layouts.md)
- [Widget](../core/widget.md)
- [Button](button.md)
- [Label](label.md)
- [Background & Color](../core/background.md)
