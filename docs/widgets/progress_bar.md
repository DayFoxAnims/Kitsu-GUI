# ProgressBar

`KitsuProgressBar` is a read-only bar that shows progress between
0% and 100%. It supports a determinate mode (with a value) and an
indeterminate mode (a looping animation), plus several text
position options.

    #include <kitsugui/progress_bar.h>

    using namespace KitsuGui;

---

## Quick start

    auto* pb = new KitsuProgressBar();
    pb->setBounds(20, 20, 250, 24)
      ->withValue(0.42f)
      ->withTextPosition(ProgressTextPosition::INSIDE);
    window->add(pb);

The bar shows 42% filled, with the number `42%` centered inside.

---

## Constructor

    KitsuProgressBar();

- No arguments.
- Default bounds: `{0, 0, 250, 24}`.
- Default mode: `DETERMINATE`, value `0.0`.
- Default text position: `INSIDE`.
- Default colors: orange fill (`255, 136, 0`), dark gray empty
  (`80, 80, 85`), gray text.

---

## Modes

    enum class ProgressMode {
        DETERMINATE,     // bar reflects a specific value
        INDETERMINATE    // looping animation, no value shown
    };

### DETERMINATE

The bar fills from 0 to 100% based on `value`. The readout (if
enabled) shows the current percentage.

### INDETERMINATE

The bar shows a moving "window" of about 40% of its width, sliding
from left to right on a loop. No value is shown, no percentage is
displayed. This is the standard pattern for operations of unknown
duration.

Switching to `INDETERMINATE`:

- Increments `g_active_animations`, which keeps the main loop
  running at full speed while the bar is visible.
- Resets the animation phase.

Switching **away** from `INDETERMINATE`:

- Decrements `g_active_animations`.
- Stops the loop from waking up unnecessarily.

This means you should **not** leave bars in indeterminate mode
when they are not visible; it wastes CPU.

---

## Fluent configuration

    KitsuProgressBar& withMode(ProgressMode m);
    KitsuProgressBar& withValue(float v);
    KitsuProgressBar& withValue(float v, float max);
    KitsuProgressBar& withTextPosition(ProgressTextPosition p);
    KitsuProgressBar& withTextColor(const Color& c);
    KitsuProgressBar& withFillTextColor(const Color& c);
    KitsuProgressBar& withTrackColors(const Color& filled,
                                      const Color& empty);
    KitsuProgressBar& withFont(TTF_Font* font);
    KitsuProgressBar& withHeight(int h);
    KitsuProgressBar& setBounds(int x, int y, int w, int h);

### withValue

Two overloads:

- `withValue(v)` — sets the value directly, clamped to `[0.0, 1.0]`.
- `withValue(v, max)` — sets the value as `v / max`. Useful for
  file downloads or counting operations where you have "current"
  and "total".

### withTextPosition

    enum class ProgressTextPosition {
        NONE,     // no text
        INSIDE,   // centered inside the bar
        ABOVE,    // above the track
        BELOW,    // below the track
        RIGHT     // to the right of the track
    };

The default is `INSIDE`.

> In `INDETERMINATE` mode, **no text is drawn** regardless of the
> text position. The readout only appears in determinate mode.

### withTextColor / withFillTextColor

These two colors are used for the split-color trick in `INSIDE`
mode:

- `text_color` — used for the part of the number that sits over
  the **empty** track (default: gray `180, 180, 190`).
- `fill_text_color` — used for the part that sits over the
  **filled** track (default: white).

The bar renders the text twice: once in `text_color`, then again
in `fill_text_color` with a clip rect set to the fill area. The
result is a number that changes color as the fill sweeps over it.
This is what makes progress bars look professional.

In `ABOVE`, `BELOW`, and `RIGHT` modes, only `text_color` is used.

### withTrackColors

Sets the fill and empty colors of the bar. Default: orange on dark
gray.

    pb->withTrackColors(
        KitsuTheme::current().accent,
        KitsuTheme::current().bg_tertiary
    );

### withHeight

    KitsuProgressBar& withHeight(int h);

Sets `bounds.h` and `requested_h` together. Convenient for
changing the bar's thickness without touching the width.

---

## Value

    float getValue() const;
    void setValue(float v);

The value is always a float between `0.0` and `1.0`. Anything
outside that range is clamped:

    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;

Setting the same value twice is a no-op.

---

## Animation (tick)

    void tick() override;

In `INDETERMINATE` mode, `tick` advances the animation phase based
on the elapsed time since the last call:

    anim_phase += elapsed_ms * 0.0008f;

The phase wraps around `1.0`, so the animation loops smoothly
regardless of frame rate.

`tick` also calls `markDirty()` on every frame while the bar is
indeterminate, which is what keeps the frame loop busy.

**You do not need to call `tick` yourself** — `run()` calls it
once per frame for every widget in the tree.

---

## Rendering

The bar draws, in order:

1. **Empty track** — a solid rectangle across the whole bounds,
   filled with `track_empty`.
2. **Fill** — a rectangle from the left edge to `bounds.w * value`
   (or the sliding window in indeterminate mode), filled with
   `track_filled`.
3. **Text** — if applicable (see above).

There is **no border**, no corner radius, and no shadow. The bar is
a plain rectangle.

### Indeterminate rendering

The sliding window is `40%` of the bar's width. Its position is
computed from `anim_phase` and clipped to the bar's bounds, so it
appears to slide in from the left, cross the bar, and slide out on
the right.

---

## Static font

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

The readout uses a global default font if the bar does not specify
its own. Without a font, no text is drawn even if `text_position`
is not `NONE`.

---

## Memory and ownership

- The bar owns its **text texture**.
- The bar does **not** own its **font**.
- Ownership of the bar itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## Known issues and notes

- **The destructor decrements `g_active_animations` only if the
  bar was in `INDETERMINATE` mode.** If you switch modes manually,
  this stays consistent — but if you force-set the mode without
  going through `withMode`, you can leak the counter. Use the
  public API.
- **The `INDETERMINATE` animation runs continuously** and forces a
  full frame redraw every frame. Do not leave indeterminate bars
  in the tree if they are not visible.
- **The corner radius from the theme is ignored.** The bar is
  always a rectangle, even if `radius_progress_bar > 0`. This is
  a known gap in the current implementation.

---

## Common patterns

### Download progress

    auto* pb = new KitsuProgressBar();
    pb->withValue(downloaded, total);
    // later:
    pb->setValue(downloaded, total);

### Loading spinner

    auto* pb = new KitsuProgressBar();
    pb->withMode(ProgressMode::INDETERMINATE);

When the operation finishes:

    pb->withMode(ProgressMode::DETERMINATE);
    pb->withValue(1.0f);

Or simply hide/delete the bar.

### Custom colors

    auto* pb = new KitsuProgressBar();
    pb->withTrackColors(
        Color(60, 180, 100),   // green fill
        Color(30, 30, 35)      // dark empty
    );

---

## See also

- [Widget](../core/widget.md)
- [Slider](slider.md) — the interactive counterpart
- [Layouts](../core/layouts.md)
