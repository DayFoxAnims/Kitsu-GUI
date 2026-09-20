# FPSView

`KitsuFPSView` is a small overlay that displays the current frames
per second, averaged over the last few samples. It is meant for
debugging and profiling, not for production UIs.

    #include <kitsugui/fps.h>

    using namespace KitsuGui;

---

## Quick start

    auto* fps = new KitsuFPSView();
    window->addOverlay(fps);

    // Set a font once at startup
    KitsuFPSView::setFont(my_mono_font);

That's it. The view updates itself every time a frame is rendered,
as long as `run()` is calling `frameRendered()` — which it does
automatically.

---

## Constructor

    KitsuFPSView();

- No arguments.
- Initial bounds: `{10, 10, 90, 24}`.
- Becomes the **singleton instance** (`s_instance`) immediately.

Only one `KitsuFPSView` should exist at a time. Creating a second
one overrides the pointer used by the main loop.

---

## How it works

The view does **not** poll the clock itself. The main loop calls:

    fps->frameRendered();

once for every frame actually drawn. The view counts frames and,
every **500 ms**, computes:

    fps = frame_count * 1000 / elapsed_ms

This value is pushed into an 8-slot ring buffer. The display shows
the **average of the non-zero slots**, which smooths out sudden
spikes.

The label text is only rebuilt when the rounded FPS number changes,
so the widget does almost nothing on most frames.

---

## Rendering

The widget draws:

1. A **semi-transparent black background** (`0, 0, 0, 180`) over
   its bounds.
2. The FPS text, centered, in a warm yellow (`255, 220, 60`).

Its bounds are **resized automatically** to fit the rendered text
plus 16px horizontal and 8px vertical padding. This means the
widget's position is fixed at `(10, 10)` but its size adapts to
the font and the digit count.

---

## Static font

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

The view uses a single global font. If you never set one, no text
is rendered and the view stays invisible (the background is still
drawn, but you won't see anything).

Set it once after loading your fonts:

    TTF_Font* font = TTF_OpenFont("DejaVuSansMono-Bold.ttf", 14);
    KitsuFPSView::setFont(font);

Monospace fonts are recommended so the label does not jump around
when the digit count changes.

---

## Singleton access

    static KitsuFPSView* getInstance();

Returns the currently registered instance, or `nullptr` if none
exists. The main loop uses this to call `frameRendered()`:

    if (auto* fps = KitsuFPSView::getInstance()) {
        fps->frameRendered();
    }

If you never create a `KitsuFPSView`, this check is a no-op.

---

## Placement

The default position `(10, 10)` puts the view in the top-left
corner. To move it:

    fps->setBounds(20, 20, 90, 24);   // x, y, w, h

The size will be overwritten on the next render when the text
changes, but the position is preserved. In other words: you can
move it, but not resize it manually.

To put it in a different corner, compute the position from the
window size and re-set it on resize.

---

## Adding as an overlay

Always add the FPS view with `addOverlay`, not `add`:

    window->addOverlay(fps);

Overlays are rendered **after** the main tree, so the FPS view
sits on top of everything. If you add it as a regular widget, it
will be drawn under other widgets and may not be visible.

---

## Memory and ownership

- The view owns its **text texture** and destroys it in the
  destructor.
- The view does **not** own its **font**.
- The window does **not** own the view when added via
  `addOverlay`.
- The view clears its own singleton pointer in the destructor.

---

## Notes and limitations

- **The widget calls `markDirty()` whenever the label changes**
  (every 500 ms). This forces a full frame redraw. For a debugging
  overlay, this is fine; for a production app, it means you can
  never have a fully idle loop while the FPS view is active.
- **The FPS counter is not a real-time measurement.** It is
  sampled over 500 ms windows and then averaged over 8 windows. A
  sudden drop in FPS may take up to 4 seconds to be fully
  reflected.
- **No styling options.** The background color, text color, and
  padding are hardcoded.
- **No way to pause or reset the counter.**

---

## See also

- [Widget](../core/widget.md)
- [Window](../core/window.md)
