---
title: Tower app
diataxis: reference
audience: [ai, human]
related_code:
  - games/tower/app/include/tower/app/Theme.hpp
  - games/tower/app/include/tower/app/Bindings.hpp
  - games/tower/app/include/tower/app/InputMapping.hpp
  - games/tower/app/include/tower/app/Scenes.hpp
  - games/tower/app/include/tower/app/MainMenuScene.hpp
  - games/tower/app/include/tower/app/PlayScene.hpp
  - games/tower/app/include/tower/app/PauseScene.hpp
  - games/tower/app/include/tower/app/GameOverScene.hpp
  - games/tower/app/src/Bindings.cpp
  - games/tower/app/src/InputMapping.cpp
  - games/tower/app/src/Scenes.cpp
  - games/tower/app/src/MainMenuScene.cpp
  - games/tower/app/src/PlayScene.cpp
  - games/tower/app/src/PauseScene.cpp
  - games/tower/app/src/GameOverScene.cpp
  - games/tower/app/CMakeLists.txt
  - games/tower/main.cpp
  - games/tower/CMakeLists.txt
  - tests/tower/app/ScenesTest.cpp
  - tests/tower/app/PlaySceneTest.cpp
  - tests/tower/app/CMakeLists.txt
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
keywords: [tower, app, bindTower, TowerActions, PlayScene, PauseScene, GameOverScene, MainMenuScene, SceneStack, RectCmd, camera, screenY]
last_reviewed: 2026-11-04
---

# Tower app

Presentation shell in `namespace sgl::tower`. Static library `tower_app` links `PUBLIC` `tower_sim`, `sgl_scene`, `sgl_render`, `sgl_input`, `sgl_ui`, and `sgl_loop` (`sgl_core` comes in through those). No SFML includes. No `sgl_audio` and no `sgl_save`. Include as `<tower/app/X.hpp>`.

Executable `tower` (`games/tower/main.cpp`) links `tower_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`. The window is **1280×720**, title **"Tower"**. `main` loads `fonts/upheavtt.ttf`, calls `bindTower`, and pushes `mainMenu` on a `SceneStack` whose pause factory is `pause`.

## Theme

`Theme.hpp` holds design size **1280×720** (the sim world `kWorldW` × view `kViewH`), menu and field colors, the HUD score anchor, and `uiFontRelativePath` (`"fonts/upheavtt.ttf"`).

## Bindings

`bindTower(ActionMap&)` assigns `TowerActions` ids **1–6** and binds them:

| Keys | Action |
|------|--------|
| Left, A | left |
| Right, D | right |
| Space, Up, W | jump |
| Enter | confirm |
| Backspace | cancel |
| Escape, P | pause |

`menuActions(actions)` builds `sgl::MenuActions` with `up = left`, `down = right`, plus `confirm` / `cancel`. Left / A moves the highlight to the previous button. Right / D moves it to the next. Enter activates the focused button.

## Input mapping

`toSimInput` copies **held** left and right into `SimInput`. A key that stays down keeps the player running. Both together cancel inside the sim. `jump` is the **pressed** edge only, so holding the key after takeoff does not jump again on the next landing. Pause and menus read the pressed edge themselves.

## Scenes

Factories in `Scenes.hpp`: `mainMenu`, `play`, `pause`, `gameOver(services, score)`. Each closes over `AppServices`:

`{const TowerActions& actions; FontId font; uint64_t seed; Tuning tuning}`

References are owned by `main()` for the whole `App::run()`; scenes must not outlive that. `PlayScene` constructs `TowerGame` with that seed and tuning. The default seed in `main` is **42** and the default `Tuning` is the sim default. A test can change `tuning` before it builds `play` (a narrow start platform, high run speed, and heavy gravity make the player walk off and fall).

| Scene | Traits | Requests |
|-------|--------|----------|
| `MainMenuScene` | opaque, blocks update, not pausable | Play → `ReplaceScene` `play`. Quit or cancel → `QuitApp` |
| `PlayScene` | opaque, blocks update, pausable | Escape / P → `RequestPause` (no step that frame). `Died` → `ReplaceScene` `gameOver` with `score()` |
| `PauseScene` | not opaque, blocks update, not pausable | Resume, pause, or cancel → `PopScene`. Menu → `PopScene` then `ReplaceScene` `mainMenu` |
| `GameOverScene` | opaque, blocks update, not pausable | Menu or cancel → `ReplaceScene` `mainMenu` |

`RequestPause` is the only way onto `PauseScene`. The stack's pause factory supplies it. Focus loss pauses only while the top scene is pausable, so the menu, pause overlay, and game-over screen stay put.

Buttons are `MenuModel` rows: main menu **Play** / **Quit**, pause **Resume** / **Menu**, game over **Menu**. Game over shows the sim score for the run that just ended. It does not keep a high-score table.

`main.cpp` opens `SfmlPlatform` **1280×720** titled `"Tower"`, loads the UI font, builds `AppServices` with constant default seed **42**, constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Playfield

`PlayScene::render` reads `player()`, `platforms()`, `cameraBottom()`, and `score()`. It does not move them. Sim **+Y is up**. Screen **+Y is down**. `RectCmd::rect.pos` is the top-left of the unrotated bounds.

A world point maps with:

`screenY = kViewH - (worldY - cameraBottom)`

`screenX = worldX` (the view is the full `kWorldW`; the camera has no X).

Body and platform `pos` is the bottom-left, so the rect's screen top-left uses the world top `pos.y + size.y`:

`screenTop = kViewH - (pos.y + size.y - cameraBottom)`

Platforms are `RectCmd`s on `Layer::World`. The player is a `RectCmd` on `Layer::Actors`. The field is a `RectCmd` on `Layer::Background`. Score is a `TextCmd` (`"Score N"`) on `Layer::Hud`. The number is the sim score for this run.

`PlayScene::game()` exposes the sim for headless tests. Those tests drive `SceneStack` with `InputState` and open no window (`tower_scenes_test`, `tower_play_scene_test`).
