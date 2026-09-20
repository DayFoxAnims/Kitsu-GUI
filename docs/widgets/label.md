# Label

`KitsuLabel` displays a single block of text. It is the simplest
leaf widget in KitsuGui and the one you will use most often for
titles, descriptions, and status messages.

It is a leaf: it does not contain children, and it does not
participate in event handling. Labels are purely visual.

    #include <kitsugui/label.h>

    using namespace KitsuGui;

---

## Quick start

    auto* label = new KitsuLabel("Hello, KitsuGui");
    label->setBounds(20, 20, 200, 30);
    window->add(label);

By default, a label uses:

- The **default font** (set globally with `KitsuLabel::setDefaultFont`).
- The **`text_primary` color** of the current theme.
- **Left alignment**, **top vertical alignment**.
- **No wrapping**.

---

## Constructor

    KitsuLabel(const std::string& text,
               int width  = 0,
               int height = 0);

- `text` — the initial string. Can be empty.
- `width`, `height` — optional initial size. If omitted, the label
  will not reserve space for alignment purposes until you set
  bounds explicitly.

The constructor captures the current theme's `text_primary` color
as the initial color. Changing the theme afterwards does **not**
update the label automatically — you must set the color again.

---

## Fluent configuration

All fluent setters return `KitsuLabel&`, so they can be chained:

    auto* title = new KitsuLabel("Welcome");
    title->withFont(my_bold_font)
         ->withColor(40, 40, 40)
         ->withAlign(TextAlign::CENTER)
         ->withVAlign(TextVAlign::MIDDLE)
         ->setBounds(0, 0, 400, 40);

### withFont

    KitsuLabel& withFont(TTF_Font* font);

Sets the font used to render this label. If not set, the global
default font is used.

See [Default font](#default-font) below.

### withColor

    KitsuLabel& withColor(Uint8 r, Uint8 g, Uint8 b);

Sets the text color. The alpha is always `255` (labels do not
support transparency).

### withAlign

    KitsuLabel& withAlign(TextAlign align);

Sets the horizontal alignment. Possible values:

| `TextAlign` | Effect                                    |
|-------------|-------------------------------------------|
| `LEFT`      | Text starts at the left edge. (default)   |
| `CENTER`    | Text is centered horizontally.            |
| `RIGHT`     | Text ends at the right edge.              |

> **Important:** alignment only takes effect if the label's
> `bounds.w` is greater than zero. If `bounds.w == 0`, the
> alignment is ignored and the text always starts at `bounds.x`.

### withVAlign

    KitsuLabel& withVAlign(TextVAlign valign);

Sets the vertical alignment. Possible values:

| `TextVAlign` | Effect                                   |
|--------------|------------------------------------------|
| `TOP`        | Text starts at the top edge. (default)   |
| `MIDDLE`     | Text is centered vertically.             |
| `BOTTOM`     | Text ends at the bottom edge.            |

> **Important:** like `withAlign`, this only takes effect when
> `bounds.h > 0`.

### withWrap

    KitsuLabel& withWrap(int max_width);

Enables word wrapping at a fixed pixel width. `max_width` is
passed directly to SDL_ttf:

    TTF_RenderUTF8_Blended_Wrapped(font, text, color, max_width);

This means:

- Wrapping is based on the **font metrics**, not on the label's
  `bounds.w`.
- If you want the wrap width to match the label's width, set both
  to the same value:

      label->setBounds(20, 20, 300, 80);
      label->withWrap(300);

- Set to `0` to disable wrapping.

---

## Setting position and size

Two families of methods are available.

### setBounds — updates both bounds and requested size

    KitsuLabel& setBounds(int x, int y, int w, int h);

    label->setBounds(20, 20, 200, 30);

This calls `setBoundsInternal`, which sets `bounds.w/h` **and**
`requested_w/h` to the same values. Use this when you want the
label to declare a specific size to a parent layout.

### at and size — updates only bounds

    KitsuLabel& at(int x, int y);
    KitsuLabel& size(int w, int h);

    label->at(20, 20)->size(200, 30);

These modify `bounds` directly and do **not** touch
`requested_w/h`. Useful when you want to reposition a label
visually without changing what it reports to a layout parent.

Both forms call `markDirty()`.

---

## Changing the text

    void setText(const std::string& new_text);
    const std::string& getText() const;

    label->setText("Updated");

Setting the same text is a no-op (no markDirty, no texture
rebuild).

---

## Default font

    static void setDefaultFont(TTF_Font* font);
    static TTF_Font* getDefaultFont();

KitsuGui uses a **single global default font** for labels that
don't specify their own. Set it once at startup:

    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 15);
    KitsuLabel::setDefaultFont(font);

Then every new label automatically uses it:

    auto* l = new KitsuLabel("Hello");   // uses the default font

If you set a font on a specific label with `withFont`, that one
takes precedence over the default.

> **Note:** `KitsuLabel` does **not** own the font. You are
> responsible for closing it with `TTF_CloseFont` after all labels
> that use it are destroyed.

---

## Rendering details

### Texture caching

Labels render text by baking it into an SDL texture once, then
reusing it across frames. The cache is invalidated when any of
these changes:

- The text content.
- The RGB color.
- The wrap width.
- The active font pointer.

Because of this, calling `setText` in a tight loop with the same
value is cheap. Calling it with a new value every frame will
rebuild the texture every frame — avoid that in animated
interfaces.

### When rendering happens

The texture is built (or rebuilt) inside `render()`, not in a
`tick()` or an update step. This means you don't need to trigger
anything manually; the next frame after a change will pick it up.

### Position calculation

    int x = abs.x;   // LEFT by default
    int y = abs.y;   // TOP by default

    if (bounds.w > 0) {
        // align horizontally using bounds.w and tex_w
    }
    if (bounds.h > 0) {
        // align vertically using bounds.h and tex_h
    }

The final destination rect has the **texture's own size**
(`tex_w`, `tex_h`), not the label's bounds. In other words, the
label draws the text at its natural size, positioned inside
`bounds` according to alignment.

This is why:

- If the text is wider than the label, it will overflow. There is
  no clipping.
- If you want the text to shrink to fit, you must do it yourself
  with a different font size.

---

## Alignment behavior

Because the label does not create a background, alignment only
affects **where the text is drawn**. The widget itself still
occupies its full bounds for hit-testing purposes, but since
labels do not handle events, this is mostly irrelevant.

A common pattern is to use a label with `STRETCH` inside a
vertical layout and center it:

    auto* box = new KitsuBox(false);   // vertical
    box->setBounds(0, 0, -1, -1);
    box->setAlignment(KitsuAlign::STRETCH);

    auto* title = new KitsuLabel("Welcome");
    title->withAlign(TextAlign::CENTER)->setBounds(0, 0, -1, 40);
    box->addChild(title, true);

Here the label is stretched horizontally by the box, and the text
itself is centered inside that stretched area.

---

## Memory and ownership

- The label owns its **texture**. It is destroyed in the
  destructor and whenever the cache is invalidated.
- The label does **not** own its **font**. Manage fonts at the
  application level.
- The label does **not** own its children — it has none.
- Ownership of the label itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [Layouts](../core/layouts.md)
- [Background & Color](../core/background.md)
- [Window](../core/window.md)
