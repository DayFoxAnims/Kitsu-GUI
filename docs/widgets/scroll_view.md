# ScrollView

`KitsuScrollView` is a scrollable container. It shows a viewport of
a larger content area, with vertical and/or horizontal scrolling,
wheel support, mouse dragging, and an animated scrollbar.

It is one of the most complex widgets in KitsuGui because it
combines three responsibilities:

1. Hosting a content box that lays out children.
2. Translating mouse coordinates between viewport space and content
   space.
3. Rendering the scrollbar with hover and drag animations.

    #include <kitsugui/scroll_view.h>

    using namespace KitsuGui;

---

## Quick start

    auto* sv = new KitsuScrollView(true, false);   // vertical
    sv->setBounds(20, 20, 300, 400);

    for (int i = 0; i < 30; i++) {
        auto* item = new KitsuLabel("Item " + std::to_string(i));
        item->setBounds(0, 0, -1, 28);
        sv->addChild(item, true);
    }

    window->add(sv);

The view shows 400px of height; the content is taller, so a
scrollbar appears on the right when the mouse hovers near it.

---

## Constructor

    KitsuScrollView(bool vertical = true,
                    bool horizontal = false);

- `vertical` — enable vertical scrolling. Default `true`.
- `horizontal` — enable horizontal scrolling. Default `false`.

Both can be enabled at once, but the current scrollbar only renders
vertically.

The constructor creates an internal `KitsuBox` (vertical) named
`content` that holds your widgets. The content box:

- Has its parent set to `this` for coordinate calculations.
- Uses auto-layout with `spacing = 8` and `padding = 10` by
  default.
- Is resized on every `updateLayout()` to match the view's width.

---

## Adding and removing children

    void addChild(KitsuWidget* child, bool owns = true);
    void removeChild(KitsuWidget* child);
    void clearChildren();
    KitsuBox* getContent() const;

These forward to the internal content box. See
[Layouts](../core/layouts.md) for ownership semantics.

> **Do not** add children with `KitsuBox::addChild` on the
> scroll view directly — it is a `KitsuWidget`, not a box. The
> correct way is `scroll_view->addChild(...)`, which routes to the
> content.

If you need to manipulate the content box directly (set spacing,
alignment, etc.):

    sv->getContent()->setSpacing(16);
    sv->getContent()->setPadding(20);

---

## Fluent configuration

    KitsuScrollView& setBounds(int x, int y, int w, int h);
    KitsuScrollView& withScrollbar(bool show);
    KitsuScrollView& withScrollbarColors(const Color& track,
                                         const Color& thumb);
    KitsuScrollView& withBottomPadding(int p);
    KitsuScrollView& withScrollbarWidth(float base,
                                        float expanded);

### withScrollbar

Shows or hides the scrollbar. The scrollbar is still interactive
when hidden, but it is not drawn.

### withScrollbarColors

Sets the track and thumb colors. Both should have alpha `< 255` for
the standard look.

    sv->withScrollbarColors(
        Color(200, 200, 205, 80),   // track
        Color(140, 140, 150, 180)   // thumb
    );

The thumb hover color is **not** configurable through this method;
it uses a hardcoded orange (`255, 136, 0, 220`).

### withBottomPadding

Adds extra empty space below the content. This lets the user scroll
past the last item so it is not glued to the bottom edge.

    sv->withBottomPadding(40);

Default is `20`.

### withScrollbarWidth

Sets the scrollbar's resting and hover widths. The scrollbar
animates between these two on mouse enter/exit.

    sv->withScrollbarWidth(6.0f, 12.0f);

Default: `6.0f` base, `12.0f` expanded.

---

## Scrolling

    int getScrollY() const;
    int getScrollX() const;
    void scrollTo(int y);
    void scrollBy(int dy);

### getScrollY / getScrollX

Current scroll offset in pixels. `0` means the content is at the
top (or left).

### scrollTo

Jumps to an absolute offset. Clamped to `[0, max_scroll_y]`.

### scrollBy

Relative scroll. Positive `dy` scrolls the content **up** (you see
content that was below). Clamped.

### Programmatic scroll to a widget

To scroll to a specific widget, compute its position in the content
box and call `scrollTo`:

    int widget_y = child->bounds.y;
    sv->scrollTo(widget_y);

There is no built-in `scrollToWidget` helper.

---

## Events

The view handles several mouse interactions.

### Mouse wheel

Scrolling with the wheel works when the cursor is inside the view's
bounds:

    sv->scrollBy(-e.wheel.y * 40);

40 pixels per notch is the hardcoded step.

### Click and drag inside the content

Left-clicking inside the view and dragging **scrolls the content**
as if it were a touch surface. This is a **drag-to-scroll** gesture,
not a click on the child. The gesture activates after the cursor
moves more than 8 pixels.

> **Important:** Drag-to-scroll competes with child widgets. A
> `KitsuButton` inside the view will receive the click, but if you
> drag more than 8 pixels, the scroll view will take over and
> consume the event.

### Scrollbar interaction

- **Hover near the right edge** — the scrollbar widens.
- **Click and drag the thumb** — moves the scroll position
  proportionally.
- **Click on the track above/below the thumb** — jumps to that
  position.

The scrollbar is only interactive when `show_scrollbar`,
`vertical`, and `max_scroll_y > 0` are all true.

### Event translation to children

When a mouse event is inside the view, the view **adjusts the
event's Y coordinate** before forwarding it to the content:

    adjusted.button.y    += scroll_y;   // for MOUSEBUTTONDOWN/UP
    adjusted.motion.y    += scroll_y;   // for MOUSEMOTION

This is what makes children react correctly even though they are
visually shifted. Children's own `bounds` are never modified; only
the render offset is.

---

## Layout

The view recomputes its layout when `updateLayout()` is called:

1. The content box is given the view's inner width (minus
   scrollbar width if vertical and visible).
2. The content box's height is left at `0` (auto).
3. `content->updateLayout()` runs, which sizes the content to fit
   its children.
4. Scroll limits are recomputed.
5. `scroll_y` is clamped to the new limits.

This happens automatically once per frame through
`root->updateLayout()`.

---

## Scroll limits

    void computeScrollLimits();

Sets:

    max_scroll_y = content_h + bottom_padding - bounds.h

If the content fits inside the view, `max_scroll_y = 0` and no
scrollbar is shown.

`max_scroll_x` is computed the same way if horizontal scrolling is
enabled, but the current implementation does not render a
horizontal scrollbar.

---

## Rendering

The scroll view renders in two phases.

### Phase 1: Content (clipped)

1. Save SDL's current clip rect (implicitly, by not managing it).
2. Set the clip rect to the view's bounds.
3. Set `content->render_offset_y = -scroll_y`. This tells the
   content box to draw itself and its children shifted up by the
   scroll amount, **without modifying their bounds**.
4. Call `content->render(renderer)`.
5. Clear the render offset.
6. Clear the clip rect.

This is why children see the correct mouse coordinates: hit
testing uses `getAbsoluteBounds()`, which ignores render offsets.
Only drawing is shifted.

### Phase 2: Scrollbar (unclipped)

If the scrollbar is visible:

1. Draw the **track** as a semi-transparent rectangle on the right
   edge.
2. Draw the **thumb** on top, with a height proportional to
   `bounds.h / (bounds.h + max_scroll_y)`.
3. The thumb color changes to orange when hovered or dragged.

The scrollbar is not clipped; it draws over the content.

---

## Scrollbar animation

The scrollbar width animates between `scrollbar_base_width` and
`scrollbar_hover_width` using an exponential smoothing:

    step = diff * 0.3f

When the difference is small enough, it snaps to the target. The
animation runs only while the widths differ; when stable, the
`g_active_animations` counter is decremented so the main loop can
go back to blocking on `SDL_WaitEvent`.

This is the only animation in the widget.

---

## Geometry helpers

Three private helpers compute the scrollbar's rectangles:

    SDL_Rect getScrollbarTrack(const SDL_Rect& abs) const;
    SDL_Rect getScrollbarThumb(const SDL_Rect& abs) const;
    bool isInScrollbar(int mx, int my, const SDL_Rect& abs) const;

The track is a vertical strip on the right edge, 2px from the
border. The thumb's vertical position is proportional to
`scroll_y / max_scroll_y`.

The "in scrollbar" check uses a generous 14px-wide zone, so the
mouse does not need to be exactly on the scrollbar to trigger hover
behavior.

---

## Known issues and notes

- **Horizontal scrolling is incomplete.** The constructor accepts
  `horizontal = true` and `scroll_x` is tracked and clamped, but
  no horizontal scrollbar is rendered, and the wheel handler does
  not respond to horizontal wheel events on most platforms.
- **No scrollbar when content fits.** This is intentional, but
  means the layout does not reserve space for the scrollbar. If
  the content grows past the threshold, widgets may shift
  horizontally to make room. If this is a problem, always reserve
  the scrollbar width by setting `bottom_padding` and a fixed
  `content_w`.
- **Drag-to-scroll competes with children.** The 8-pixel threshold
  helps, but a `KitsuTextInput` inside the view can still lose a
  drag to the scroll view if the user moves the mouse too far
  before releasing.
- **No momentum scrolling.** The scroll stops as soon as the mouse
  is released. Adding inertia would be a nice enhancement for
  mobile.
- **No scroll-to-widget.** You must compute the target position
  yourself.
- **The `dragging` flag is only used for vertical drags.** Drag
  gestures in other directions are ignored.
- **`getScrollX()` always returns 0 unless horizontal scrolling
  was enabled**, and even then, nothing renders the horizontal
  offset.

---

## Common patterns

### Long list

    auto* sv = new KitsuScrollView(true, false);
    sv->setBounds(0, 0, -1, -1);

    auto* content = sv->getContent();
    content->setSpacing(2);
    content->setPadding(8);

    for (int i = 0; i < 100; i++) {
        auto* item = new KitsuLabel("Row " + std::to_string(i));
        item->setBounds(0, 0, -1, 32);
        sv->addChild(item, true);
    }

### Settings panel

    auto* sv = new KitsuScrollView(true, false);
    sv->setBounds(20, 20, 400, 500)
       ->withBottomPadding(40);

    sv->addChild(new KitsuCheckBox("Enable telemetry"), true);
    sv->addChild(new KitsuCheckBox("Auto-save"), true);
    sv->addChild(new KitsuSwitch("Dark mode"), true);
    // ...

### Scroll to top after adding items

    sv->scrollTo(0);

---

## Memory and ownership

- The view owns its **content box** and destroys it in the
  destructor. In turn, the content box owns the children added
  with `owns = true`.
- The view does **not** own its **font** (it has none).
- Ownership of the view itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [Layouts](../core/layouts.md)
- [Panel](panel.md)
- [Popup](popup.md)
