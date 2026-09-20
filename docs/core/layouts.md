# Layouts

KitsuGui offers a single container type for layout: **`KitsuBox`**.
It can arrange its children either horizontally (HBox) or vertically
(VBox), with optional automatic sizing and distribution.

All layout logic lives in `KitsuBox`. There is no separate grid,
stack, or flow container.

    #include <kitsugui/box.h>

    using namespace KitsuGui;

---

## Creating a box

    KitsuBox(bool horizontal = true);

> **Warning:** the default is **horizontal**. To create a vertical
> box you must pass `false`:
>
>     KitsuBox vbox(false);   // vertical
>     KitsuBox hbox(true);    // horizontal
>     KitsuBox hbox;          // horizontal (default!)

### KitsuHBox and KitsuVBox

    using KitsuHBox = KitsuBox;
    using KitsuVBox = KitsuBox;

These are **typedefs**, not separate types. Because of that:

    KitsuHBox h;    // horizontal (matches intuition)
    KitsuVBox v;    // horizontal (!) — same default as KitsuBox

**Do not rely on `KitsuVBox` to give you a vertical box.** Always
use `KitsuBox(false)` explicitly if you want a vertical layout.

---

## Manual vs auto layout

    void setAutoLayout(bool enabled);
    bool isAutoLayout() const;

By default, a `KitsuBox` uses **auto-layout**: it positions and
sizes its children based on their `requested_w/h` and the box's
own settings.

If you disable auto-layout:

    box->setAutoLayout(false);

the box will only:

- Apply fill semantics for children with a negative requested size
  (see below).
- Recursively call `updateLayout()` on each child.

This is useful for "canvas" containers where you want to position
children manually with `setBounds()`.

The `Window` root is created with auto-layout **disabled** so that
widgets added with `add()` keep their explicit bounds.

---

## Children: adding and ownership

    void addChild(KitsuWidget* child, bool owns = true);
    void removeChild(KitsuWidget* child);
    void clearChildren();

- `addChild(child, owns = true)` — appends the child and, if
  `owns` is true, records it so the box deletes it in its
  destructor and on removal.
- `removeChild(child)` — removes the child. If it was owned, it
  is deleted. Otherwise, its `parent` pointer is cleared.
- `clearChildren()` — deletes all owned children and empties both
  the visible and owned lists.

If a child already has a parent when you call `addChild`, it is
**not** reparented automatically. Remove it from the old parent
first.

---

## Aligning children (cross-axis)

    void setAlignment(KitsuAlign a);

`align` controls how children are positioned along the **cross
axis**:

- In an HBox, the cross axis is vertical (Y).
- In a VBox, the cross axis is horizontal (X).

| `KitsuAlign` | Effect                                             |
|--------------|----------------------------------------------------|
| `START`      | Pack at the top (HBox) or left (VBox).             |
| `CENTER`     | Center along the cross axis.                       |
| `END`        | Pack at the bottom (HBox) or right (VBox).         |
| `STRETCH`    | Force every child to fill the cross axis.          |

Example:

    hbox->setAlignment(KitsuAlign::CENTER);   // vertical centering
    vbox->setAlignment(KitsuAlign::START);    // left alignment

---

## Justifying children (main axis)

    void setJustify(KitsuJustify j);

`justify_content` controls how children are distributed along the
**main axis**:

- In an HBox, the main axis is horizontal (X).
- In a VBox, the main axis is vertical (Y).

| `KitsuJustify`  | Effect                                            |
|-----------------|---------------------------------------------------|
| `START`         | Packed at the start (default).                    |
| `CENTER`        | Packed in the middle.                             |
| `END`           | Packed at the end.                                |
| `SPACE_BETWEEN` | First and last flush; space distributed between.  |
| `SPACE_AROUND`  | Equal space around each child.                    |
| `SPACE_EVENLY`  | Equal space before, between, and after.           |

These only take effect when the box has extra free space along
its main axis.

---

## Spacing, padding, margin

    void setSpacing(int s);      // gap between children
    KitsuBox& setPadding(int p); // inner padding on all four sides
    KitsuBox& setMargin(int m);  // outer margin (see note below)

- **Spacing** is the gap inserted between adjacent children along
  the main axis. It does not apply to the cross axis.
- **Padding** is the empty space between the box's border and its
  children. A single integer is applied on all four sides.
- **Margin** is defined on `KitsuWidget`, but the current layout
  code only subtracts it from the fill target when the box is in
  non-auto-layout mode. In auto-layout, it is effectively unused.

Set spacing and padding explicitly when you want predictable
results:

    box->setPadding(16);
    box->setSpacing(8);

---

## Sizing the box itself

The box's own `requested_w` and `requested_h` follow the tri-state
convention:

| Value | Meaning                                                    |
|-------|------------------------------------------------------------|
| `0`   | Auto-size: compute from children + padding.                |
| `> 0` | Fixed size: use exactly that, distribute remaining space.  |
| `< 0` | Same as `0` for the box itself (with slight nuance below). |

For the common "fill the parent" case, use:

    box->setBounds(0, 0, -1, -1);

Inside a parent that has a known size, `-1` will expand to fill.
At the top level (root of the `Window`), the size is set by
`syncRootSize()` anyway.

For "auto-size to fit children":

    box->setBounds(0, 0, 0, 0);

The box will recompute its own `bounds.w/h` after each layout pass.

---

## Sizing children

When a `KitsuBox` lays out its children, it looks at each child's
`requested_w` (in HBox) or `requested_h` (in VBox):

- `> 0` → the child gets exactly that size on the main axis.
- `<= 0` → the child is treated as flexible and shares the
  remaining space (after fixed children and spacing).

For the cross axis:

- `> 0` → the child gets exactly that size.
- `<= 0` → the child fills the cross axis (unless `align` is set
  to `START`, `CENTER`, or `END`, in which case the child keeps
  its requested cross size and is positioned accordingly; for
  `STRETCH`, it always fills).

Putting it together, a child that fills both axes is:

    child->setBounds(0, 0, -1, -1);

A child that is 200 wide, 32 tall, and positioned manually inside
a non-auto-layout box:

    child->setBounds(20, 20, 200, 32);

A child that fills width but keeps a fixed height of 40 in a VBox:

    child->setBounds(0, 0, -1, 40);

---

## Example: centered card

    auto* vbox = new KitsuBox(false);      // vertical
    vbox->setBounds(0, 0, -1, -1);         // fill the parent
    vbox->setPadding(40);
    vbox->setSpacing(16);
    vbox->setAlignment(KitsuAlign::CENTER);
    vbox->setJustify(KitsuJustify::CENTER);

    auto* title = new KitsuLabel("Hello");
    title->setBounds(0, 0, -1, 30);        // fill width, 30 tall
    vbox->addChild(title, true);

    auto* button = new KitsuButton("OK");
    button->setBounds(0, 0, 120, 36);      // fixed size
    vbox->addChild(button, true);

    window->add(vbox);

`vbox` is vertical, so:

- `align` (`CENTER`) controls the **horizontal** placement of
  children.
- `justify` (`CENTER`) controls the **vertical** distribution.

The result is a vertically centered column of widgets, each
horizontally centered.

---

## Nested layouts

Boxes can contain other boxes. A common pattern is a horizontal
top bar inside a vertical root:

    root (VBox)
    ├── top_bar (HBox, 48 tall)
    │   ├── logo
    │   └── user_menu (aligned right via spacer or justify)
    └── content (VBox, fills the rest)

Because layout is recursive, each container computes its own
children before the parent positions it. The order in
`updateLayout()` is:

1. Fill pass (if not auto-layout).
2. Auto-layout on direct children (if enabled).
3. Recursive `updateLayout()` on each child.

---

## The Window root

`Window` creates a `KitsuBox(false)` (vertical) as its root, sized
to the full window, with auto-layout **disabled**. Widgets added
through `Window::add()` keep the bounds you set on them.

If you want auto-layout for the whole window, add your own box
on top:

    auto* root = new KitsuBox(false);
    root->setBounds(0, 0, -1, -1);
    root->setPadding(20);
    window->add(root);

    // add widgets to `root` instead of `window`
    root->addChild(myButton, true);

---

## See also

- [Widget](widget.md)
- [Window](window.md)
- [Background & Color](background.md)
