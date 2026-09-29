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
  - engine/ui/src/MenuModel.cpp
  - engine/ui/src/RenderMenu.cpp
  - engine/ui/src/PauseOverlay.cpp
  - engine/ui/CMakeLists.txt
  - tests/engine/ui/MenuModelTest.cpp
  - tests/engine/ui/PauseOverlayTest.cpp
related_docs:
  - source-layout.md
  - engine-input.md
  - engine-render.md
  - engine-scene.md
  - pause-overlay.md
  - ../explanation/architecture.md
  - ../how-to/build-and-test.md
keywords: [sgl, ui, MenuModel, MenuActions, Button, renderMenu, PauseOverlay, MenuStyle]
last_reviewed: 2026-09-29
---

# Engine UI

Headless menu kit in `namespace sgl`, included as `<sgl/ui/X.hpp>`. Target `sgl_ui` is a STATIC library that PUBLIC-links `sgl_input`, `sgl_render`, and `sgl_scene`. No SFML dependency.

## MenuActions, Button, MenuStyle

`MenuActions` holds `ActionId` slots for `up`, `down`, `confirm`, and `cancel`.

`Button` is a hit rectangle plus `label` string.

`MenuStyle` supplies `FontId`, `textSize`, and colors: `idle`, `focused`, `text`, and `panel`.

## MenuModel

Owns a list of buttons and keyboard/pointer focus.

| API | Behavior |
|-----|----------|
| `update(input, actions)` | Returns the activated button index this frame, or `nullopt` |
| `cancelled()` | `true` after `cancel` was pressed (sticky) |
| `focus()` | Current focused index |
| `buttons()` | `std::span<const Button>` |

Keyboard `up`/`down` wrap. Pointer hover moves focus to the hit button. A pointer click activates only when press and release both land inside the same button. `confirm` activates the focused row. `cancel` sets `cancelled()` and does not activate.

## renderMenu

`renderMenu(queue, model, style, layer)` draws a padded panel (`style.panel`) around the button bounds, then each button as `RectCmd` (`idle` / `focused`) with centered `TextCmd` labels.

## PauseOverlay

`PauseOverlay : SceneI` builds a two-row menu (Resume, Quit) from `MenuActions` and `FontId`.

| Activation | Request |
|------------|---------|
| Resume (index 0) or `cancel` | `PopScene` |
| Quit (index 1) | `QuitApp` |

Traits match a non-opaque pause overlay: `opaque = false`, `blocksUpdate = true`, `pausable = false`. Render uses `Layer::Overlay` (dim full-screen rect, then `renderMenu`).
