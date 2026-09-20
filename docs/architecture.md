# Architecture

This page explains how KitsuGui is put together internally: the
object model, the event flow, the rendering model, and the main
loop.

It is aimed at contributors and at users who need to understand
why something behaves the way it does.

---

## The object tree

Every visible element is a `KitsuWidget`. Widgets form a tree:

- A widget may have a `parent` pointer.
- A container (like `KitsuBox` or `KitsuPanel`) holds a list of
  `children`.

Coordinates are **relative to the parent**. Two helpers compute
the absolute position:

    SDL_Rect getAbsoluteBounds() const;   // for hit testing
    SDL_Rect getRenderBounds() const;     // for drawing

`getRenderBounds()` adds each ancestor's `render_offset_x/y`,
which is used by containers like `KitsuScrollView` to shift
children without changing their logical position.

---

## The Window root

A `Window` creates a `KitsuBox(false)` (vertical) as its **root**,
sized to the whole window, with **auto-layout disabled**. Widgets
added with `Window::add()` keep the bounds you set.

Overlays are handled separately: `Window::addOverlay()` stores
them in a vector, and `run.cpp` renders them **after** the main
tree, without affecting layout.

---

## The main loop

`run.cpp` is the heart of the library. It is a single blocking
loop, no threads. Each iteration:

1. **Polls events** with `SDL_PollEvent` in a `while` loop.
2. **Routes events** to the appropriate handler (see below).
3. **Ticks widgets** (`tick()` on the root and on popups).
4. **Redraws the frame** if anything is dirty.
5. **Sleeps** (`SDL_WaitEvent` or `SDL_WaitEventTimeout`) when
   there is no work to do.

This keeps CPU usage near zero when nothing is happening.

---

## Event routing priorities

Events are routed in strict priority order. Once a handler
consumes an event, the lower priorities never see it.

### Priority 1 — Context menu (global capture)

    KitsuContextMenu* active_menu = KitsuContextMenu::getActive();
    if (active_menu && active_menu->isOpen()) {
        active_menu->handleEvent(event);
        continue;
    }

If a context menu is open, it consumes **every** event: mouse,
keyboard, wheel. The widget tree below never sees them. This is
why clicking outside a menu closes it without activating the
button underneath.

Only one context menu can be open at a time (enforced by a static
pointer).

### Priority 2 — Root widget tree

    if (!active_modal && root) {
        root->handleEvent(event);
    }

If no modal popup is open, the event is passed to the root box,
which forwards it down the tree.

The root's `handleEvent` iterates its children **in reverse**
order, so the topmost widget gets the event first. Each child
returns `true` to consume or `false` to let the event continue.

This is how `KitsuBox::handleEvent` works:

    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->handleEvent(e)) return true;
    }
    return false;

### Priority 3 — Popups

    for (auto* popup : g_popups) {
        if (popup->isOpen()) popup->handleEvent(event);
    }

Every open popup receives the event. Each popup filters by window
ID: only events targeting its own SDL window are processed.

A modal popup also has priority 2 disabled for the main window, so
while a modal is up, the main tree is inert.

---

## Popups

A `KitsuPopup` is a **separate SDL window** with its own renderer
and root box. It is not part of the main widget tree.

Popups live in a global vector `g_popups`. The main loop:

- Registers popups on construction.
- Renders each open popup separately after the main window.
- Cleans up closed popups by deleting them on the next iteration.

This design gives popups genuine modality on desktop and separate
event routing on mobile.

---

## Rendering model

KitsuGui is **retained mode**: widgets keep their state, and
`run()` redraws the whole frame when anything is dirty.

### Dirty flag

Each widget has a `dirty_` flag. Calling `markDirty()`:

1. Sets the flag.
2. Notifies the global `KitsuRenderer` with the widget's rect.

    if (g_renderer) {
        g_renderer->markDirty(abs.x, abs.y, abs.w, abs.h);
    }

Despite the name, the renderer does **not** redraw only that
rectangle. The flag is used as a "something changed, redraw the
next frame" trigger. The whole window is cleared and repainted.

### Frame drawing

When anything is dirty, `run.cpp` executes:

    kr->beginFrame();
    SDL_RenderClear(renderer);
    root->updateLayout();
    root->render(renderer);
    for (overlay : overlays) overlay->render(renderer);
    if (active_menu) active_menu->render(renderer);
    kr->endFrame();

The order matters:

1. **Clear** — fills the window with the background color.
2. **Layout** — recomputes sizes and positions for auto-layout
   containers.
3. **Tree render** — draws the widget tree in tree order (parents
   before children).
4. **Overlays** — draws FPS and similar widgets on top.
5. **Menu** — draws the active context menu on top of everything.

### KitsuRenderer

`KitsuRenderer` is a thin wrapper around the SDL renderer that
tracks whether the frame is dirty. It does not implement partial
redraw or clipping; it just centralizes the dirty flag.

`beginFrame` and `endFrame` are bookkeeping. If no widget is
dirty, `run.cpp` skips the whole block and goes back to waiting
for events.

---

## Layout

Only `KitsuBox` (and its subclasses `KitsuPanel` and
`KitsuRadioGroup`) implement layout. `updateLayout()` is called
top-down on the tree once per frame, before rendering.

The algorithm has two modes:

- **Auto-layout off** — only fills children with `requested_w < 0`
  or `requested_h < 0` to match the parent's inner size. Does not
  touch positions.
- **Auto-layout on** — runs the flex-like algorithm
  (`layoutHorizontal` / `layoutVertical`) that distributes
  children, honors alignment and justify, applies spacing and
  padding.

See [Layouts](core/layouts.md) for details.

---

## Ticking and animations

`tick()` is called once per frame on every visible widget. It is
used for time-based updates: caret blinking in text inputs, FPS
averaging in the FPS view, phase advancement in indeterminate
progress bars, and scrollbar width animation in scroll views.

The main loop uses a global counter `g_active_animations` to
decide whether it can block on `SDL_WaitEvent` or must poll on a
16ms timer:

    if (had_events) {
        SDL_Delay(16);
    } else if (g_active_animations > 0 || force_render) {
        SDL_WaitEventTimeout(nullptr, 16);
    } else {
        SDL_WaitEvent(nullptr);
    }

Widgets that animate increment `g_active_animations` when they
start and decrement it when they stop. This keeps the loop running
at ~60 FPS while animations are active, and lets it block
indefinitely when nothing is happening.

---

## Focus and keyboard input

Keyboard focus is **not** part of the widget tree. Only
`KitsuTextInput` currently uses it:

- A static `KitsuTextInput* s_focused` tracks the focused input.
- Setting focus on a new input unfocuses the previous one.
- Focusing calls `SDL_StartTextInput`; unfocusing calls
  `SDL_StopTextInput`.

There is no generic focus system for other widgets. Buttons react
to mouse only.

---

## Memory and ownership

KitsuGui uses **raw pointers** and manual ownership. There are no
smart pointers anywhere in the public API.

Ownership rules vary by container:

- `KitsuBox::addChild(child, owns = true)` — the box owns and
  deletes the child.
- `Window::add(widget)` — the window does **not** own the widget.
- `Window::addOverlay(widget)` — same, does not own.
- `KitsuPopup::add(widget)` — does not own.
- `KitsuPopup::addButton()` and `addLabel()` — popup owns.

Because ownership is inconsistent, always check the docs for each
container. The general rule is: "if the container created it, the
container owns it; if you created it, you own it".

### Static globals

Three globals track application-wide state:

- `g_window` — the main window (set by `Window` constructor).
- `g_renderer` — the main `KitsuRenderer` (set by `run()`).
- `g_active_animations` — counter for the main loop's sleep
  strategy.

Cleaning up correctly means letting the `Window` destructor run
before `SDL_Quit`. `run()` handles this in its final cleanup
block.

---

## Threading

KitsuGui is **single-threaded**. The main loop, event handling,
layout, and rendering all happen on the main thread.

SDL requires the main thread for event pumping and rendering on
most platforms, especially Android. Do not attempt to add threads
for rendering or event handling.

Background work (file loading, network) must be done on your own
threads and communicated back via flags or queues that the main
loop polls in `tick()`.

---

## What is not implemented

Some things you might expect from a GUI library that KitsuGui
does not do:

- **No partial redraw.** The whole frame is cleared and repainted
  when anything is dirty.
- **No clipping beyond the scroll view.** Widgets can draw outside
  their bounds; only `KitsuScrollView` sets an SDL clip rect.
- **No z-order within a container.** Children are drawn in
  reverse order of addition, but there is no way to raise or
  lower a specific widget.
- **No generic focus system.** Only text inputs participate.
- **No hit-testing shape support.** Everything is a rectangle.
- **No alpha blending for widgets by default.** The renderer does
  not enable `SDL_BLENDMODE_BLEND` globally; widgets that need it
  (menus, popups) enable it manually.
- **No automatic DPI scaling.** Coordinates are physical pixels.
- **No internationalization.** Strings hardcoded in Spanish in a
  few places (context menu items, dropdown placeholder).
- **No accessibility layer.**
- **No tests.** The library has no unit tests or integration
  tests.

---

## Where to look in the source

    include/kitsugui/   — public headers
    src/                — implementations
    src/run.cpp         — main loop and event routing
    src/window.cpp      — window, renderer, root
    src/widget.cpp      — base class and hit testing
    src/box.cpp         — layout algorithm
    src/context_menu.cpp — global event capture example
    examples/main.cpp   — usage reference

Start with `run.cpp` if you want to understand the library's
control flow. Almost everything else is either a widget or a
helper called from there.

---

## See also

- [Widget](core/widget.md)
- [Layouts](core/layouts.md)
- [Window](core/window.md)
- [Getting Started](getting-started.md)
- [Theming](theming.md)
