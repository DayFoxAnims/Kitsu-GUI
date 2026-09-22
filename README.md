# KitsuGui

A C++ GUI library built on top of SDL2.

Created by DayFox.

---

## A word of honesty

KitsuGui is an **early-stage, experimental project**. It is not
production-ready. The API may change between minor versions without
warning, and behaviors you rely on today may disappear tomorrow.

**Most of the code in this library was generated with the help of
AI (DeepSeek).** It has not been battle-tested, reviewed
exhaustively, or validated against a formal specification. Bugs,
inconsistencies, and design mistakes are expected. The
documentation in `docs/` was also written with AI assistance and
may not be perfectly accurate — especially if the code has changed
since the docs were last updated.

Use KitsuGui for learning, experimentation, or personal projects
where you are comfortable reading the source when something does
not match the docs. Do not use it as the foundation of a product
you ship to others without auditing it yourself.

If that doesn't scare you away, welcome aboard.

---

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![SDL2](https://img.shields.io/badge/SDL2-2.x-green.svg)
![Status: pre-alpha](https://img.shields.io/badge/status-pre--alpha-red.svg)

## What is KitsuGui?

KitsuGui is a lightweight, modular UI library designed for C++
applications that use SDL2 as their graphics backend. It provides
a set of reusable widgets, a theme system, flex-like layouts, and
a retained-mode renderer that only repaints when something
actually changes.

The goal is to offer a solid, hackable foundation with no heavy
dependencies, ideal for personal projects, internal tools, and
small-to-medium applications that want a fast UI without pulling
in a full framework.

It is not trying to compete with Dear ImGui, Nuklear, Qt, or
similar mature toolkits.

---

## Features

- Widgets: Button, Label, Panel, Box, TextInput, CheckBox, Switch,
  Slider, ProgressBar, Radio, ScrollView, Dropdown, ContextMenu,
  Popup, Icon, FPSView, and more.
- Interchangeable theme system (`KitsuTheme`) with semantic colors
  (bg_primary, text_secondary, accent_hover, etc.).
- Flex-like layouts: HBox, VBox, alignment, justification, spacing,
  and padding.
- Retained-mode rendering: widgets keep their state, and the frame
  is repainted only when something changes.
- Global event capture for context menus and modals.
- UTF-8 support in text inputs.
- Vector icons via SDL2_gfx and XDG icon themes.
- Image loading with SDL2_image.
- TTF fonts through SDL2_ttf with texture caching.
- Config and theme files loadable at runtime.

---

## Requirements

- CMake 3.10 or higher.
- Compiler with C++17 support.
- SDL2
- SDL2_ttf
- SDL2_gfx
- SDL2_image

On Termux:

    pkg install cmake clang sdl2 sdl2_ttf sdl2_gfx sdl2_image

For SVG icon support (optional), also install `librsvg` so that
`rsvg-convert` is available on the PATH.

---

## Building

    git clone <repo-url> KitsuGui
    cd KitsuGui
    mkdir build && cd build
    cmake ..
    make

The example executable is generated at `build/kitsu_example`.

To build only the library (without examples):

    cmake .. -DBUILD_EXAMPLES=OFF

---



## Project structure

    KitsuGui/
    ├── CMakeLists.txt
    ├── README.md
    ├── docs/
    │   ├── INDEX.md
    │   ├── getting-started.md
    │   ├── architecture.md
    │   ├── theming.md
    │   ├── core/
    │   └── widgets/
    ├── include/
    │   └── kitsugui/
    ├── src/
    └── examples/
        └── main.cpp

---

## Status

**Pre-alpha.** Active development, no stability guarantees.

- The API may change between minor versions.
- Widgets and behaviors may be added, renamed, or removed without
  a deprecation period.
- Bugs are expected, especially in the interaction between
  widgets, focus, and popup windows.
- The documentation may lag behind the code.
- There are no unit tests or CI at the moment.

If you find a discrepancy between the docs and the code, assume
the code is correct and open an issue.

---

## AI-assisted development

The codebase and documentation were largely produced with the
help of **DeepSeek**, an AI assistant. This means:

- **Code quality varies.** Some parts are clean, others show
  signs of mechanical generation and could be refactored.
- **Comments may be inconsistent.** Some are in Spanish, some in
  English, some are missing.
- **Documentation may drift.** Descriptions were written against a
  specific snapshot of the code and are not automatically
  regenerated.
- **Design decisions are not always optimal.** The API has rough
  edges that a human would likely have smoothed out.

Contributions that fix bugs, tighten the design, or improve the
documentation are welcome.

---

## License

MIT License. Copyright (c) 2026 DayFox.

See [LICENSE](LICENSE) for the full text.

---

## Contact

DayFox

- X (Twitter): @dayfoxanims

---

## Credits

Thanks to the SDL2 community, to the XDG icon theme spec authors,
and to all the open source projects that inspire the building of
accessible, hackable tools.

Built with substantial AI assistance (DeepSeek).

Made with care by DayFox — and with plenty of rough edges on
purpose.
