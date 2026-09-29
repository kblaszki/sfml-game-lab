---
title: Engine SFML backend
diataxis: reference
audience: [ai, human]
related_code:
  - engine/sfml/include/sgl/sfml/EventTranslate.hpp
  - engine/sfml/include/sgl/sfml/SfmlClock.hpp
  - engine/sfml/src/SfmlClock.cpp
  - engine/sfml/include/sgl/sfml/SfmlAssets.hpp
  - engine/sfml/src/SfmlAssets.cpp
  - engine/sfml/include/sgl/sfml/SfmlRenderer.hpp
  - engine/sfml/src/SfmlRenderer.cpp
  - engine/sfml/include/sgl/sfml/SfmlDraw.hpp
  - engine/sfml/include/sgl/sfml/SfmlAudio.hpp
  - engine/sfml/src/SfmlAudio.cpp
  - engine/sfml/include/sgl/sfml/SfmlPlatform.hpp
  - engine/sfml/src/SfmlPlatform.cpp
  - engine/save/include/sgl/save/Settings.hpp
  - engine/sfml/CMakeLists.txt
  - tests/engine/sfml/EventTranslateTest.cpp
  - tests/engine/sfml/SfmlDrawTest.cpp
  - tests/engine/sfml/CMakeLists.txt
related_docs:
  - source-layout.md
  - engine-loop.md
  - engine-render.md
  - engine-resources.md
  - engine-input.md
  - engine-save.md
keywords: [eng, sgl_sfml, SfmlPlatform, SfmlRenderer, SfmlAssets, SfmlClock, EventTranslate, setDisplay, WindowMode, letterbox, MeshCmd, antiAliasingLevel]
last_reviewed: 2026-09-29
---

# Engine SFML backend

Only SFML-facing engine module (`namespace sgl::sfml`). Target `sgl_sfml` is a STATIC library that PUBLIC-links `sgl_loop`, `sgl_resources`, `sgl_audio`, and `sgl_save`, and PRIVATE-links `SFML::Graphics`, `SFML::Window`, `SFML::System`, and `SFML::Audio`. `SfmlAudio` implements `AudioI` (16 voices). Includes are `<sgl/sfml/X.hpp>`.

## EventTranslate

`toKey(sf::Keyboard::Key)` maps the gameplay/UI set; everything else is `Key::Unknown`.

| SFML key | `sgl::Key` |
|----------|------------|
| A, D, W, S, Z, X, C, P, H | A, D, W, S, Z, X, C, P, H |
| Num1–Num9 (top-row digits, not numpad) | Num1–Num9 |
| Left, Right, Up, Down | Left, Right, Up, Down |
| Enter, Escape, Backspace, Space, LShift | Enter, Escape, Backspace, Space, LShift |
| any other | Unknown |

`pointerPixel(event)` → `std::optional<sf::Vector2i>` for `MouseMoved` / `MouseButtonPressed` / `MouseButtonReleased` (pixel position); otherwise `nullopt`.

`translate(event, designPosition)` → `std::optional<InputEvent>`:

| SFML event | Result |
|------------|--------|
| `Closed` | `WindowClosed` |
| `FocusLost` / `FocusGained` | `FocusLost` / `FocusGained` |
| `KeyPressed` / `KeyReleased` | `KeyDown` / `KeyUp` via `toKey` |
| `MouseButtonPressed` | `MouseDown` at `designPosition` (caller maps pixels) |
| `MouseButtonReleased` | `MouseUp` at `designPosition` |
| `MouseMoved` | `MouseMove` at `designPosition` |
| other | `nullopt` |

Windowless unit tests live in `event_translate_test` (links `SFML::Window` only).

## SfmlClock

`SfmlClock : ClockI` — `restart()` returns `Seconds` from `sf::Clock::restart().asSeconds()`.

## SfmlAssets

GPU caches over `ResourceCache`:

- `texture(key, Image)` — build `sf::Image` from pixels, `loadFromImage`; failure → `DecodeFailed`
- `font(key, path)` — missing path → `NotFound`; `openFromFile` failure → `DecodeFailed`
- `get(TextureId)` / `get(FontId)` — raw SFML pointers (or null)

## SfmlDraw

`SfmlDraw.hpp` converts draw commands to SFML types:

- `toShape(RectCmd|LineCmd|CircleCmd)` — rectangle / line / circle shapes
- `toSprite(SpriteCmd, texture)` — textured sprite with center origin
- `toVertexArray(MeshCmd)` — `sf::VertexArray` with `PrimitiveType::Triangles` or `TriangleStrip` from `MeshKind`; each vertex maps position, `detail::toSfColor` color, and `texCoords` from `uv`

## SfmlRenderer

`SfmlRenderer(sf::RenderTarget&, const SfmlAssets&)` implements `RendererI`:

- `begin()` — clear target; reset `skippedCommands()`
- `submit(RenderQueue&)` — walk `queue.sorted()`, `std::visit` `SpriteCmd` / `RectCmd` / `TextCmd` / `LineCmd` / `CircleCmd` / `MeshCmd`; missing texture (or font) skips and increments the counter; untextured `MeshCmd` (`texture` nullopt) always draws; textured mesh uses `sf::RenderStates` with the bound texture. `Anchor::Center` origins text at local-bounds center. `SfmlDraw.hpp` builds the sprite and the rectangle with origin at the center and position at top-left plus half the size, so `rotationDeg` turns around the center of the unrotated bounds. `rotationDeg == 0` draws as before. `LineCmd` becomes a rotated `sf::RectangleShape` of length `|to-from|` and height `thickness`, origin at the left-middle, positioned at `from`. `CircleCmd` becomes an `sf::CircleShape` with origin at the center.
- `end()` — `display()` only when the target is an `sf::RenderWindow`

## SfmlPlatform

`SfmlPlatform : PlatformI` owns window + assets + renderer. `setDisplay` is on `SfmlPlatform` only — not on `PlatformI`. Scenes must not call it; wiring happens outside scenes.

**Member order:** `sf::RenderWindow window_` first, then `SfmlAssets assets_`, `SfmlAudio` (unique_ptr), then `SfmlRenderer renderer_` — assets destroy before the window so GL textures die while the context still lives. Stores constructor `title_` for recreate.

Constructor `(Vec2u designSize, title)`: opens windowed at design size via shared `openDisplay` (same path as `setDisplay`).

`setDisplay(WindowMode mode, Vec2u size)` recreates the SFML window:

| Mode | VideoMode | Style / State |
|------|-----------|---------------|
| `Windowed` | `{size.x, size.y}` | `Style::Default`, `State::Windowed` |
| `Fullscreen` | `VideoMode::getDesktopMode()` (size ignored) | `Style::Default`, `State::Fullscreen` |

Shared create details: `ContextSettings` with `antiAliasingLevel = 4`, key-repeat off, framerate 60. After create, rebuilds the design-size `sf::View` and reapplies letterbox using the new window size. `designSize_` stays the original ctor design size (letterbox reference); it is not replaced by the settings resolution. `WindowMode` comes from `<sgl/save/Settings.hpp>` (`sgl_sfml` PUBLIC-links `sgl_save`).

**Letterbox:** on `Resized`, recompute a centered viewport that preserves design aspect (pillarbox or letterbox bars). Resize is consumed inside `poll()` and is not forwarded as an `InputEvent`. Mouse pixel positions from `pointerPixel` (`MouseMoved` / `MouseButtonPressed` / `MouseButtonReleased`) are mapped with `mapPixelToCoords` before `translate`.

`assets()` exposes the cache for loading after construction. `renderer()` returns the `SfmlRenderer`. `audio()` returns `SfmlAudio&` so callers can `upload` PCM without a downcast.
