# KitsuGui Documentation

Welcome to the KitsuGui documentation.

> **Documentation accuracy notice**
>
> These docs were written with AI assistance and may not always
> reflect the current state of the code. The API is unstable and
> changes frequently. When in doubt, read the headers in
> `include/kitsugui/` — they are the source of truth.

If you are new here, start with **Getting Started**. If you
already know the basics, jump to the section you need.

---

## Table of contents

### 1. Introduction

- [Getting Started](getting-started.md) — install, build, and run
  your first KitsuGui app.
- [Architecture](architecture.md) — how the library is structured
  internally (retained mode, event flow, renderer).

### 2. Core

- [Window](core/window.md) — the main application window.
- [Background & Color](core/background.md) — the color type and
  how the window background works.
- [Widget](core/widget.md) — base class for every UI element.
- [Layouts](core/layouts.md) — HBox, VBox, alignment, justify.

### 3. Widgets

**Text and buttons**

- [Label](widgets/label.md)
- [Button](widgets/button.md)
- [TextInput](widgets/text_input.md)

**Containers**

- [Panel](widgets/panel.md)
- [ScrollView](widgets/scroll_view.md)

**Selection and toggles**

- [CheckBox](widgets/check_box.md)
- [Switch](widgets/switch.md)
- [Radio](widgets/radio.md)
- [Dropdown](widgets/dropdown.md)

**Continuous controls**

- [Slider](widgets/slider.md)
- [ProgressBar](widgets/progress_bar.md)

**Overlays and floating UI**

- [ContextMenu](widgets/context_menu.md)
- [Popup](widgets/popup.md)

**Visual and utility**

- [Icon](widgets/icon.md)
- [FPSView](widgets/fps.md)

### 4. Theming

- [Theming](theming.md) — themes, tokens, and how to customize
  the look of the library.

### 5. Cookbook

*(to be documented)*

---

## Conventions used in this documentation

- Code blocks are C++17.
- `Kitsu*` prefixes refer to classes from the library.
- `setBounds(x, y, w, h)` uses absolute coordinates for top-level
  widgets, and coordinates relative to the parent for children.
- A `-1` in width or height means "fill available space" in the
  context of a layout. A `0` means "auto-size".
- "The theme" refers to the global `KitsuTheme::current()`.

---

## Known documentation gaps

- The [Cookbook](cookbook/) section is empty. It is meant to hold
  short recipes for common tasks (custom widgets, right-click
  menus, theming overrides, ...).
- `KitsuConfig` is referenced but not documented yet.
- Some widgets (viewport, internal helpers) do not have dedicated
  pages.
- Screenshots are missing across the docs.

If you find a page that contradicts the code, open an issue.

---

## Contributing to the docs

The documentation lives in `docs/` as plain Markdown files. If you
find something wrong or missing, open an issue or send a pull
request.

Because the API is unstable, docs may lag behind the code. When
updating a widget, update its `.md` file too if the public API
changed.

---

Made with care by DayFox — and with AI assistance.
