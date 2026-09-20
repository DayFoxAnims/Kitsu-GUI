# Slider

`KitsuSlider` is a draggable control that lets the user pick a
numeric value from a range. It supports horizontal and vertical
orientations, optional step snapping, and an optional numeric
readout.

    #include <kitsugui/slider.h>

    using namespace KitsuGui;

---

## Quick start

    auto* slider = new KitsuSlider(0.0f, 100.0f, 50.0f);
    slider->setBounds(20, 20, 250, 24)
          ->withCallback([](float value) {
              SDL_Log("Value: %.1f", value);
          });
    window->add(slider);

The slider starts at 50, uses a range of 0..100, and fires the
callback on every drag update.

---

## Constructor

    KitsuSlider(float min = 0.0f,
                float max = 100.0f,
                float value = 0.0f);

- `min`, `max` — the range. `max` must be greater than `min` for
  the slider to have meaningful behavior, but the widget does not
  validate this.
- `value` — the initial value. It is **not** clamped to the range
  at construction time; call `withRange` or `setValue` after if
  you need to enforce that.
- Default bounds: `{0, 0, 250, 24}`, horizontal.

> The default constructor does **not** set a step. Dragging is
> continuous. Use `withStep()` to snap.

---

## Fluent configuration

    KitsuSlider& withOrientation(SliderOrientation o);
    KitsuSlider& withValue(float v);
    KitsuSlider& withRange(float min, float max);
    KitsuSlider& withStep(float s);
    KitsuSlider& withShowNumber(bool show);
    KitsuSlider& withNumberSuffix(const std::string& suffix);
    KitsuSlider& withDecimals(int d);
    KitsuSlider& withFont(TTF_Font* font);
    KitsuSlider& withCallback(std::function<void(float)> cb);
    KitsuSlider& disabled();

All return `KitsuSlider&` and can be chained.

### withOrientation

Sets the orientation and **automatically changes the default
bounds**:

| Orientation  | New bounds      |
|--------------|-----------------|
| `HORIZONTAL` | `250 x 24`      |
| `VERTICAL`   | `24 x 200`      |

If you need a custom size, set bounds **after** calling this.

### withRange

Updates `min` and `max` and clamps the current value into the new
range if needed. Does **not** fire the callback.

### withStep

Sets the snap step. When `step > 0`, every value set (whether by
drag or programmatically) is rounded to the nearest multiple of
`step`:

    value = roundf(v / step) * step

`step = 0` (default) means continuous dragging.

### withShowNumber

Enables or disables the numeric readout next to the track. The
readout shows the current value with `decimals` precision and an
optional suffix.

### withNumberSuffix

Appends a string after the number, useful for units:

    slider->withNumberSuffix("%");     // "42%"
    slider->withNumberSuffix(" px");   // "120 px"

### withDecimals

Sets the number of decimal digits in the readout. Default is `0`,
which shows integers only.

    slider->withDecimals(1);   // "42.5"

### disabled

Disables the slider. It renders in disabled colors and ignores
mouse events.

> **Known issue:** like other widgets, `disabled()` sets the
> private `enabled_` flag, not the public `KitsuWidget::enabled`.

---

## State and value

    float getValue() const;
    void setValue(float v);

`setValue`:

1. Clamps `v` to `[min, max]`.
2. Rounds it to the nearest multiple of `step` (if `step > 0`).
3. If the resulting value differs from the current one, marks
   dirty and fires the callback.

Setting the same value twice in a row is a no-op: no callback, no
redraw.

`withValue(v)` is a thin wrapper around `setValue`.

---

## Events

### Mouse interaction

- **Click on the track** (anywhere along it, within the knob's
  vertical/horizontal band) — jumps the value to the clicked
  position and starts dragging.
- **Click on the knob** — starts dragging without changing the
  value.
- **Drag** — continuously updates the value.
- **Release** — stops dragging.

While dragging, the callback fires on every value change. If you
need to debounce, do it yourself on the callback side.

### Value calculation

Dragging maps the mouse position along the track to a normalized
`0..1` value, then converts it back:

    value = min + norm * (max - min)

For vertical sliders, the orientation is **inverted**: top is
`max`, bottom is `min`.

---

## Rendering

The slider draws four layers, in order:

1. **Track (empty)** — the background bar, colored `bg_tertiary`
   (or `bg_disabled` when disabled).
2. **Fill** — the filled portion of the track up to the current
   value. Uses `accent` (or `border_disabled` when disabled).
3. **Knob** — a rounded rect the size of `slider_knob_size` from
   the theme, positioned at the value. Has its own border.
4. **Number** — the readout text, if `show_number` is enabled.

Track thickness and knob size come from the theme:
`slider_track_thickness` and `slider_knob_size`. Corner radii come
from `radius_slider_track` and `radius_slider_knob`.

### Number placement

The number is positioned depending on orientation:

- **Horizontal**: to the **right** of the track.
- **Vertical**: **below** the track.

The track is shortened to make room for the number when
`show_number` is true.

### Color changes on hover/drag

When the mouse is over the slider or dragging is active:

- The knob border changes from `border` to `accent`.
- The number text color changes from `text_primary` to `accent`.

This gives visual feedback without changing the track colors.

---

## Static font

    static void setFont(TTF_Font* font);
    static TTF_Font* getFont();

The number readout uses a global default font if the slider does
not specify its own. Set it once at startup:

    KitsuSlider::setFont(my_font);

Without a font, the number is not rendered even if
`show_number = true`.

---

## Static constants (internal)

The `.cpp` defines a few hardcoded constants that are **not**
configurable from the theme:

    TRACK_THICKNESS = 6    (fallback; theme value is used instead)
    KNOB_SIZE       = 16   (fallback; theme value is used instead)
    NUMBER_GAP      = 12
    NUMBER_WIDTH    = 50

`NUMBER_GAP` and `NUMBER_WIDTH` are truly hardcoded — the gap
between track and text, and the reserved width for the readout,
cannot be changed without editing the source.

---

## Known issues and notes

- **`withHeight(int)` is defined in the `.cpp` but not declared in
  the header.** Using it in user code will not compile. It is a
  leftover from an earlier API; ignore it.
- **The vertical orientation does not re-check bounds after
  `withOrientation`.** If you switch orientation at runtime, the
  bounds are updated to the defaults, discarding any custom size
  you had set before.
- **The callback fires very frequently during drags** (potentially
  every mouse motion event). If your callback is expensive, use a
  timer or a dirty flag to throttle it.
- **`step` is applied on read**, meaning the stored `value` is
  always a multiple of `step`. If you change `step` after setting
  a value, the value is not re-snapped until the next `setValue`.

---

## Common patterns

### Volume control

    auto* vol = new KitsuSlider(0.0f, 100.0f, 75.0f);
    vol->withShowNumber(true)
       ->withNumberSuffix("%")
       ->withStep(5.0f)
       ->withCallback([](float v) {
           set_volume((int)v);
       });

### Temperature with decimals

    auto* temp = new KitsuSlider(15.0f, 30.0f, 22.5f);
    temp->withDecimals(1)
        ->withNumberSuffix("°C");

### Vertical fader

    auto* fader = new KitsuSlider(0.0f, 1.0f, 0.5f);
    fader->withOrientation(SliderOrientation::VERTICAL)
         ->setBounds(20, 20, 24, 200);

---

## Memory and ownership

- The slider owns its **number texture**.
- The slider does **not** own its **font**.
- Ownership of the slider itself is decided by its container (see
  [Layouts](../core/layouts.md)).

---

## See also

- [Widget](../core/widget.md)
- [ProgressBar](progress_bar.md) — the read-only counterpart
- [Layouts](../core/layouts.md)
