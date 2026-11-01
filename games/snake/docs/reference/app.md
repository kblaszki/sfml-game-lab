---
title: Snake app
diataxis: reference
audience: [ai, human]
related_code:
  - games/snake/app/include/snake/app/Theme.hpp
  - games/snake/app/include/snake/app/Bindings.hpp
  - games/snake/app/include/snake/app/InputMapping.hpp
  - games/snake/app/include/snake/app/Scenes.hpp
  - games/snake/app/include/snake/app/MainMenuScene.hpp
  - games/snake/app/include/snake/app/PlayScene.hpp
  - games/snake/app/include/snake/app/PauseScene.hpp
  - games/snake/app/include/snake/app/GameOverScene.hpp
  - games/snake/app/src/Bindings.cpp
  - games/snake/app/src/InputMapping.cpp
  - games/snake/app/src/Scenes.cpp
  - games/snake/app/src/MainMenuScene.cpp
  - games/snake/app/src/PlayScene.cpp
  - games/snake/app/src/PauseScene.cpp
  - games/snake/app/src/GameOverScene.cpp
  - games/snake/app/CMakeLists.txt
  - games/snake/main.cpp
  - games/snake/CMakeLists.txt
  - tests/snake/app/ScenesTest.cpp
  - tests/snake/app/PlaySceneTest.cpp
  - tests/snake/app/CMakeLists.txt
related_docs:
  - sim.md
  - ../explanation/design.md
  - ../index.md
  - ../../../../docs/reference/engine-scene.md
  - ../../../../docs/reference/engine-input.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/engine-render.md
  - ../../../../docs/reference/pause-overlay.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [snake, app, bindSnake, SnakeActions, PlayScene, PauseScene, GameOverScene, MainMenuScene, SceneStack]
last_reviewed: 2026-11-01
---

# Snake app

Presentation shell in `namespace sgl::snake`. Static library `snake_app` links `PUBLIC` `snake_sim`, `sgl_scene`, `sgl_render`, `sgl_input`, `sgl_ui`, `sgl_loop`, and `sgl_resources` (`sgl_core` comes in through those). No SFML includes. Include as `<snake/app/X.hpp>`.

Executable `snake` (`games/snake/main.cpp`) links `snake_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`. The window is **1280×720**, title **"Snake"**. `main` loads `fonts/upheavtt.ttf`, calls `bindSnake`, and pushes `mainMenu` on a `SceneStack` whose pause factory is `pause`.

This wave has no sounds and no high-score table. `GameOverScene` only shows the score from the sim.

## Theme

`Theme.hpp` holds design size **1280×720**, cell size **40** (32×18 cells fill the window), menu and board colors, the HUD score anchor, and `uiFontRelativePath` (`"fonts/upheavtt.ttf"`).

## Bindings

`bindSnake(ActionMap&)` assigns `SnakeActions` ids **1–7** and binds them:

| Keys | Action |
|------|--------|
| Up, W | moveUp |
| Down, S | moveDown |
| Left, A | moveLeft |
| Right, D | moveRight |
| Enter | confirm |
| Backspace | cancel |
| Escape, P | pause |

`menuActions(actions)` builds `sgl::MenuActions` with `up = moveUp`, `down = moveDown`, plus `confirm` / `cancel`, so the same keys navigate menus and turn the snake.

## Input mapping

`toSimInput` copies a pressed direction edge into `SimInput::turn`. Several directions in one frame resolve in order Up, Down, Left, Right. No press leaves `turn` empty, and the sim keeps its current `Dir`. The sim still rejects a 180° reverse.

## Scenes

Factories in `Scenes.hpp`: `mainMenu`, `play`, `pause`, `gameOver(services, score)`. Each closes over `AppServices` (`actions`, `font`, `seed`). `PlayScene` constructs `SnakeGame` with that seed.

| Scene | Traits | Requests |
|-------|--------|----------|
| `MainMenuScene` | opaque, blocks update, not pausable | Play → `ReplaceScene` `play`. Quit or cancel → `QuitApp` |
| `PlayScene` | opaque, blocks update, pausable while `over()` is false | Escape / P → `RequestPause` (no step that frame). `Died` → `ReplaceScene` `gameOver` with `score()` |
| `PauseScene` | not opaque, blocks update, not pausable | Resume, pause, or cancel → `PopScene`. Menu → `PopScene` then `ReplaceScene` `mainMenu` |
| `GameOverScene` | opaque, blocks update, not pausable | Menu or cancel → `ReplaceScene` `mainMenu`. Quit → `QuitApp` |

`RequestPause` is the only way onto `PauseScene`. The stack's pause factory supplies it. Focus loss pauses only while the top scene is pausable, so the menu, pause overlay, and game-over screen stay put.

Buttons are `MenuModel` rows: main menu **Play** / **Quit**, pause **Resume** / **Menu**, game over **Menu** / **Quit**.

## Board

`PlayScene::render` pushes one `RectCmd` per cell (40×40). Sim **+y is up**, so screen y is `(kRows - 1 - cell.y) * cellSize` and cell `(0, 0)` sits at the bottom left. Body segments are inset rects; the head (`body()[0]`) uses a distinct fill. Food is a `CircleCmd` when `food()` lies on the board. Score is a `TextCmd` (`"Score N"`) on `Layer::Hud`.

`PlayScene::game()` exposes the sim for headless tests. Those tests drive `SceneStack` with `InputState` and open no window (`snake_scenes_test`, `snake_play_scene_test`).
