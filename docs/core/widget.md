# Widget

`KitsuWidget` is the abstract base class of every visible element
in KitsuGui: buttons, labels, panels, text inputs, and even the
layout containers themselves.

You never instantiate `KitsuWidget` directly. You inherit from it
(or use one of the provided subclasses).

    #include <kitsugui/widget.h>

    using namespace KitsuGui;

---

## What a widget owns

Every widget has:

- A **position and size**, stored in `bounds`.
- An optional **parent**, forming a tree.
- A **requested size**, used by layouts.
- **Flags** for visibility and enabled/disabled state.
- A **dirty flag**, used by the renderer to know when to redraw.
- A right-click **callback** slot.

Subclasses add behavior (drawing, event handling, layout) on top of
this.

---

## Coordinate system

Widget coordinates are **relative to the parent**, except for
top-level widgets whose parent is `nullptr` (in which case they are
absolute window coordinates).

    struct KitsuWidget {
        KitsuWidget* parent = nullptr;
        SDL_Rect bounds = {0, 0, 0, 0};
        // ...
    };

To get the actual screen-space rectangle, use one of the two
helper methods:

    SDL_Rect getAbsoluteBounds() const;
    SDL_Rect getRenderBounds() const;

### getAbsoluteBounds

Walks up the tree adding each ancestor's `bounds.x/y`. This is
the rectangle used for **hit-testing** (`contains`, mouse events).

It does **not** apply `render_offset_x/y`.

### getRenderBounds

Same as `getAbsoluteBounds`, but also adds each ancestor's
`render_offset_x/y`. This is the rectangle where the widget is
actually painted.

Use this when you want to draw something at the same place the
widget is drawn.

### contains

    bool contains(int x, int y) const;

Convenience hit-test. Expects **absolute** coordinates and returns
true if they fall inside `getAbsoluteBounds()`.

---

## Size: the tri-state convention

Every widget has two size-related fields:

    SDL_Rect bounds;        // final size after layout
    int requested_w;
    int requested_h;

The **requested** size is what the widget asks for. The layout
system may override it to fill available space, or to auto-size
around the content.

For a widget that participates in a layout (`KitsuBox`), the
requested size is interpreted as:

| Value         | Meaning                                      |
|---------------|----------------------------------------------|
| `0`           | Auto — compute from children (for containers).|
| `< 0`         | Fill available space (flex).                 |
| `> 0`         | Fixed size.                                  |

For a **child** inside a `KitsuBox`, the meaning is slightly
different:

| Value         | Meaning                                      |
|---------------|----------------------------------------------|
| `<= 0`        | Stretch to fill the inner dimension.         |
| `> 0`         | Use that exact size.                         |

`setBoundsInternal(x, y, w, h)` sets both `bounds.w/h` and
`requested_w/h` to the same value. This is convenient for the
common case, but it means that if you want to change only
`bounds` without touching `requested`, you must modify
`bounds` directly.

    // Sets bounds AND requested to (20, 20, 200, 32)
    widget->setBoundsInternal(20, 20, 200, 32);

    // Only changes the actual size, leaves requested untouched
    widget->bounds.w = 200;
    widget->bounds.h = 32;

---

## Visibility and enabled

    bool visible = true;
    bool enabled = true;

- `visible = false`: the widget is skipped by `render()` and by
  `handleEvent()` in `KitsuBox`.
- `enabled = false`: the widget is still drawn, but subclasses
  should ignore input.

These two flags are independent. It is perfectly valid to have
an invisible-but-enabled widget, or a visible-but-disabled one.

---

## Dirty tracking

    virtual void markDirty();
    bool isDirty() const;
    void clearDirty();

Calling `markDirty()`:

1. Sets the internal `dirty_` flag.
2. Notifies the global renderer with the widget's rect.

KitsuGui is a **retained-mode** library. When anything changes,
the whole frame is redrawn on the next iteration of the main loop.
There is no per-region dirty-rect optimization: the flag is used
as a "something changed, redraw the next frame" trigger.

You should call `markDirty()` whenever you change a property that
affects rendering (color, size, text, etc.). Most built-in setters
already do this for you.

---

## Render offset

    int render_offset_x = 0;
    int render_offset_y = 0;

These fields belong to the **parent**, not the widget itself.
They are added on top of the parent's bounds when computing the
render rectangle of its children.

Typical use: a scroll view sets its `render_offset_y` to shift all
its children up or down without changing their `bounds`. This lets
hit-testing (`getAbsoluteBounds`) stay independent of the visual
offset, which is useful for clipping and interaction.

If you are not implementing a scrolling container, you can ignore
these fields.

---

## Right-click callback

Every widget can register a single right-click handler:

    std::function<void(int, int)> on_right_click;

The callback receives the **absolute** `(x, y)` where the click
happened. It is only invoked if the widget is visible, enabled,
and the callback is non-empty.

To trigger this mechanism from a subclass, call:

    bool checkRightClick(const SDL_Event& e);

This:

1. Returns false immediately if the event is not a right-click.
2. Returns false if there is no callback.
3. Returns false if the widget is hidden or disabled.
4. Returns false if the click is outside the widget.
5. Otherwise, calls the callback and returns true.

Inside your `handleEvent`, you can do:

    bool MyWidget::handleEvent(const SDL_Event& e) {
        if (checkRightClick(e)) return true;
        // ... other event handling ...
    }

---

## Virtual methods to override

    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void updateLayout() {}
    virtual bool handleEvent(const SDL_Event& e) { return false; }
    virtual void tick() {}

- **`render`** — required. Draw the widget using the SDL renderer.
  Use `getRenderBounds()` to know where to draw.
- **`updateLayout`** — optional. Called once per frame before
  rendering, in tree order. Containers use it to lay out children.
  Leaf widgets usually don't need to override it.
- **`handleEvent`** — optional. Return `true` to consume the event,
  `false` to let it propagate to siblings.
- **`tick`** — optional. Called once per frame. Use it for
  animations, blinking carets, polling, etc. Return value is
  ignored.

`KitsuWidget`'s destructor is virtual so that subclasses are
destroyed correctly through base pointers.

---

## Memory and ownership

KitsuWidget itself does **not** own its children or its parent.
Ownership is decided by the container:

- `KitsuBox::addChild(child, owns = true)` — the box deletes the
  child in its destructor if `owns` is true.
- `Window::add(widget)` — the window does **not** own the widget.
- `Window::addOverlay(widget)` — same, does not own.

See [Layouts](layouts.md) for details on `KitsuBox` ownership.

A widget's `parent` pointer is set by the container that adopts it
and cleared when it is removed (unless it's owned, in which case
it's deleted).

---

## Minimal custom widget

    class MyBox : public KitsuWidget {
    public:
        void render(SDL_Renderer* renderer) override {
            SDL_Rect r = getRenderBounds();
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &r);
        }
    };

    // Usage:
    auto* b = new MyBox();
    b->setBoundsInternal(10, 10, 100, 50);
    window->add(b);

---

## See also

- [Layouts](layouts.md)
- [Window](window.md)
- [Background & Color](background.md)
