---
title: Engine UI
diataxis: reference
audience: [ai, human]
related_code:
  - engine/ui/include/sgl/ui/MenuActions.hpp
  - engine/ui/include/sgl/ui/Button.hpp
  - engine/ui/include/sgl/ui/MenuModel.hpp
  - engine/ui/include/sgl/ui/MenuStyle.hpp
  - engine/ui/include/sgl/ui/RenderMenu.hpp
  - engine/ui/include/sgl/ui/PauseOverlay.hpp
  - engine/ui/include/sgl/ui/SettingsScene.hpp
  - engine/ui/include/sgl/ui/TextLayout.hpp
  - engine/ui/include/sgl/ui/TooltipSystem.hpp
  - engine/ui/include/sgl/ui/HoverAnim.hpp
  - engine/ui/src/MenuModel.cpp
  - engine/ui/src/RenderMenu.cpp
  - engine/ui/src/PauseOverlay.cpp
  - engine/ui/src/SettingsScene.cpp
  - engine/ui/src/TextLayout.cpp
  - engine/ui/src/TooltipSystem.cpp
  - engine/ui/src/HoverAnim.cpp
  - engine/ui/CMakeLists.txt
  - tests/engine/ui/MenuModelTest.cpp
  - tests/engine/ui/PauseOverlayTest.cpp
  - tests/engine/ui/SettingsSceneTest.cpp
  - tests/engine/ui/TextLayoutTest.cpp
  - tests/engine/ui/TooltipTest.cpp
  - tests/engine/ui/HoverAnimTest.cpp
related_docs:
  - source-layout.md
  - engine-input.md
  - engine-render.md
  - engine-scene.md
  - engine-save.md
  - pause-overlay.md
  - ../explanation/architecture.md
  - ../how-to/build-and-test.md
keywords: [sgl, ui, MenuModel, MenuActions, Button, renderMenu, PauseOverlay, SettingsScene, MenuStyle, TextLayout, TooltipSystem, HoverAnim]
last_reviewed: 2026-09-29
---

# Engine UI

Headless menu kit in `namespace sgl`, included as `<sgl/ui/X.hpp>`. Target `sgl_ui` is a STATIC library that PUBLIC-links `sgl_input`, `sgl_render`, `sgl_scene`, `sgl_anim`, and `sgl_save`. No SFML dependency.

## MenuActions, Button, MenuStyle

`MenuActions` holds `ActionId` slots for `up`, `down`, `confirm`, `cancel`, `left`, and `right` (`left` / `right` default-construct).

`Button` is a hit rectangle plus `label` string.

`MenuStyle` supplies `FontId`, `textSize`, colors (`idle`, `focused`, `text`, `panel`), optional `cornerRadius` (default `0`), and `bodyFont` (default empty `FontId`).

## MenuModel

Owns a list of buttons and keyboard/pointer focus.

| API | Behavior |
|-----|----------|
| `update(input, actions)` | Returns the activated button index this frame, or `nullopt` |
| `cancelled()` | `true` after `cancel` was pressed (sticky) |
| `focus()` | Current focused index |
| `buttons()` | `std::span<const Button>` |

Keyboard `up`/`down` wrap. Pointer hover moves focus to the hit button. A pointer click activates only when press and release both land inside the same button. `confirm` activates the focused row. `cancel` sets `cancelled()` and does not activate.

## TextLayout

Headless monospace width estimate and word wrap (no SFML).

| API | Behavior |
|-----|----------|
| `estimateWidth(text, size, advance = 0.6)` | `text.size() * size * advance` (byte count) |
| `wrap(text, size, maxWidth, advance = 0.6)` | Split on spaces so no line exceeds `maxWidth`, except a single word already wider; empty text yields an empty vector |

## TooltipSystem

Hover tooltip model. Hit-tests the first `Hotspot` whose `Rect<float>` contains the pointer (half-open). No tooltip until the pointer stays on the same hotspot for `TooltipStyle::delay` (default `0.35s`). Leaving or switching hotspot clears `active()` immediately and resets the timer. After the delay, `active()` holds a `TooltipView` with wrapped body lines; alpha fades `0→1` over `0.1s`. Panel is placed at pointer `+ {16, 20}` and flipped/clamped to stay inside `{0,0,1280,720}`.

`render` emits `Layer::Overlay` commands only: one `roundedRect` mesh (radius 8) plus `TextCmd` title and lines. Emits nothing when inactive.

## HoverAnim

Per-button hover scale. `update(hovered, count, dt)` resizes to `count`. Unhovered slots ease toward `1.0`, hovered toward `1.06`, using `easeOutCubic` over `0.06s` per step. `scale(index)` returns `1` for indices past `count`.

## renderMenu

`renderMenu(queue, model, style, layer)` draws a padded panel (`style.panel`) around the button bounds, then each button as `RectCmd` (`idle` / `focused`) with centered `TextCmd` labels.

`renderMenu(queue, model, style, layer, hover)` delegates to the four-argument overload when `cornerRadius <= 0` and every `hover.scale(i) == 1`. Otherwise button (and panel) fills use `roundedRect` meshes when `cornerRadius > 0`, and any non-`1` scale grows that button rect about its center (text stays centered on the scaled rect).

## PauseOverlay

`PauseOverlay : SceneI` builds a two-row menu (Resume, Quit) from `MenuActions` and `FontId`. Arkanoid and Tetris keep using this overlay (or their own pause scenes); Syndicate pause is `PauseMenuScene` instead — see [games/syndicate/docs/reference/app.md](../../games/syndicate/docs/reference/app.md) and [pause-overlay.md](pause-overlay.md).

| Activation | Request |
|------------|---------|
| Resume (index 0) or `cancel` | `PopScene` |
| Quit (index 1) | `QuitApp` |

Traits match a non-opaque pause overlay: `opaque = false`, `blocksUpdate = true`, `pausable = false`. Render uses `Layer::Overlay` (dim full-screen rect, then `renderMenu`).

## SettingsScene

`SettingsScene : SceneI` edits a `Settings` snapshot (`masterVolume`, `WindowMode`, resolution from `kResolutions`). Constructor: `(Settings initial, MenuActions actions, FontId font, std::function<void(const Settings&)> onChange)`. An empty `onChange` is treated as a no-op.

Rows: **Master volume: N%**, **Window: Windowed|Fullscreen**, **Resolution: WxH**, **Back**. `left` / `right` (and `<` / `>` chevron clicks) nudge the focused editable row: volume ±10 clamped 0..100, window toggles, resolution cycles `kResolutions`. Every change calls `onChange` once. Back or `cancel` → `PopScene`. Traits: `opaque = false`, `blocksUpdate = true`. Renders a dim full-screen rect, rounded panel, and rows on `Layer::Overlay` with `HoverAnim` on the pointer row.
