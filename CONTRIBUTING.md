# Contributing

KitsuGui is a personal, early-stage project. Contributions are
welcome, but please keep expectations aligned with the current
state of the library.

---

## Before you start

1. Read the [Getting Started](docs/getting-started.md) guide.
2. Read [Architecture](docs/architecture.md) to understand the
   internal design.
3. Skim the [documentation index](docs/INDEX.md).

The docs are useful, but **the source in `include/kitsugui/` is
the source of truth**. The library is AI-assisted and the docs may
lag behind the code.

---

## What kind of contributions are useful

- **Bug fixes.** The library has known rough edges. If you find
  one and can fix it cleanly, please do.
- **Documentation improvements.** If a page contradicts the code,
  or an example does not compile, fix it.
- **Small, focused features.** New widgets or layout helpers are
  welcome if they fit the existing style.
- **Portability fixes.** KitsuGui is primarily developed on Termux
  with clang. Patches that help it run on other platforms are
  appreciated.

---

## What to avoid

- **Large refactors without discussion.** Open an issue first.
- **Adding dependencies.** KitsuGui deliberately sticks to SDL2
  and its satellite libraries (ttf, gfx, image).
- **Breaking the public API without a good reason.** If you do,
  update the docs.
- **Assuming stability.** This project is pre-alpha. Do not build
  a product on it.

---

## Code style

There is no strict style guide, but the existing code follows
these tendencies:

- C++17.
- `Kitsu` prefix for public types.
- Methods use `camelCase`, member variables use `snake_case` or a
  trailing underscore when private.
- 4 spaces, no tabs.
- Comments in Spanish or English (both appear in the codebase).

When in doubt, match the surrounding code.

---

## Submitting a change

1. Fork the repository.
2. Create a branch: `git checkout -b fix/short-description`.
3. Make your changes. Keep commits focused.
4. Update the docs if the public API changed.
5. Open a pull request with a short description of what and why.

Small PRs are easier to review than large ones. If you are unsure
whether a change is welcome, open an issue first.

---

## AI-assisted development

The codebase and docs were largely produced with AI assistance
(DeepSeek). If you contribute, do not assume the existing code was
reviewed by a human or follows a formal design. Read before you
trust.

---

## License

By contributing, you agree that your contributions will be
licensed under the MIT License, the same as the rest of the
project
