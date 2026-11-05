---
title: Tower app
diataxis: reference
audience: [ai, human]
related_code:
  - games/tower/app/include/tower/app/Theme.hpp
  - games/tower/app/include/tower/app/Bindings.hpp
  - games/tower/app/include/tower/app/InputMapping.hpp
  - games/tower/app/include/tower/app/Sounds.hpp
  - games/tower/app/include/tower/app/Feedback.hpp
  - games/tower/app/include/tower/app/SpriteBank.hpp
  - games/tower/app/include/tower/app/Scenes.hpp
  - games/tower/app/include/tower/app/MainMenuScene.hpp
  - games/tower/app/include/tower/app/PlayScene.hpp
  - games/tower/app/include/tower/app/PauseScene.hpp
  - games/tower/app/include/tower/app/GameOverScene.hpp
  - games/tower/app/src/Bindings.cpp
  - games/tower/app/src/InputMapping.cpp
  - games/tower/app/src/Sounds.cpp
  - games/tower/app/src/Feedback.cpp
  - games/tower/app/src/SpriteBank.cpp
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
  - tests/tower/app/SoundsTest.cpp
  - tests/tower/app/FeedbackTest.cpp
  - tests/tower/app/SpriteBankTest.cpp
  - tests/tower/app/CMakeLists.txt
related_docs:
  - sim.md
  - assets.md
  - ../explanation/design.md
  - ../index.md
  - ../../../../docs/reference/engine-scene.md
  - ../../../../docs/reference/engine-input.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/engine-render.md
  - ../../../../docs/reference/engine-audio.md
  - ../../../../docs/reference/engine-save.md
  - ../../../../docs/reference/engine-resources.md
  - ../../../../docs/reference/pause-overlay.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [tower, app, bindTower, TowerActions, PlayScene, PauseScene, GameOverScene, MainMenuScene, SceneStack, Sounds, Feedback, HighScoreTable, SpriteBank, RectCmd, camera, screenY, autojump, bonuses]
last_reviewed: 2026-11-05
---

# Tower app

Presentation shell in `namespace sgl::tower`. Static library `tower_app` links `PUBLIC` `tower_sim`, `sgl_scene`, `sgl_render`, `sgl_input`, `sgl_ui`, `sgl_loop`, `sgl_resources`, `sgl_audio`, and `sgl_save` (`sgl_core` comes in through those). No SFML includes. Include as `<tower/app/X.hpp>`.

Executable `tower` (`games/tower/main.cpp`) links `tower_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`. The window is **1280×720**, title **"Tower"**. `main` loads `fonts/upheavtt.ttf`, optionally loads `tower/atlas.png` + `atlas.txt` into `SpriteBank` (see [assets.md](assets.md)), uploads `soundSpecs()`, loads the high-score table, calls `bindTower`, and pushes `mainMenu` on a `SceneStack` whose pause factory is `pause`.

## Theme

`Theme.hpp` holds design size **1280×720** (the sim world `kWorldW` × view `kViewH`), menu and field colors, the HUD score anchor, and `uiFontRelativePath` (`"fonts/upheavtt.ttf"`). Procedural draws use those colors when a sprite region is missing.

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

`toSimInput` copies **held** left, right, and jump into `SimInput`. Holding jump while grounded (including after each landing) keeps autojumping. Pause and menus still read the pressed edge themselves.

## Sounds

`soundSpecs()` returns a span of `{key, Pcm}` for **jump**, **land**, **die**, **wall**, and **bonus**. Each buffer is built with `sgl::blip`. `SoundIds` holds the uploaded `SoundId` handles. `main.cpp` uploads every spec through `platform.audio().upload` (`SfmlAudio&`) and stores the ids in `AppServices`.

`PlayScene` plays **jump** only when a takeoff actually happens: grounded before `step`, jump held, and airborne with `vy > 0` after `step`. `Feedback::onEvents` plays **land** for `Landed`, **die** for `Died`, **wall** for `WallBounced`, and **bonus** for `BonusPicked`.

## High scores

Capacity **5** (`kHighScoreCapacity`). Path is `userDataDir("tower") / "tower.scores"`, or `temp_directory_path() / "tower.scores"` if `userDataDir` fails (the process does not abort). `main.cpp` loads with `ScoreStore::load`. `GameOverScene` inserts the final sim score (`int` stored as `uint32_t`), lists the table as ranking lines `"#N score"` (1-based, best first), shows **"New record"** when the score ranks, and saves with `ScoreStore::save`. The inserted line uses the title color. A save failure shows `"Save failed: …"` and the scene stays up. A score that does not qualify is not written.

## Scenes

Factories in `Scenes.hpp`: `mainMenu`, `play`, `pause`, `gameOver(services, score)`. Each closes over `AppServices`:

`{const TowerActions& actions; FontId font; uint64_t seed; AudioI& audio; SoundIds sounds; path scoresPath; HighScoreTable& highScores; Tuning tuning; SpriteBank sprites}`

References are owned by `main()` for the whole `App::run()`; scenes must not outlive that. `SpriteBank` is a value (empty when the atlas is absent). `PlayScene` constructs `TowerGame` with that seed and tuning, and `Feedback` from `audio` and `sounds`. The default seed in `main` is **42** and the default `Tuning` is the sim default. A test can change `tuning` before it builds `play` (a narrow start platform, high run speed, and heavy gravity make the player walk off and fall).

| Scene | Traits | Requests |
|-------|--------|----------|
| `MainMenuScene` | opaque, blocks update, not pausable | Play → `ReplaceScene` `play`. Quit or cancel → `QuitApp` |
| `PlayScene` | opaque, blocks update, pausable | Escape / P → `RequestPause` (no step that frame). `Died` → `ReplaceScene` `gameOver` with `score()` |
| `PauseScene` | not opaque, blocks update, not pausable | Resume, pause, or cancel → `PopScene`. Menu → `PopScene` then `ReplaceScene` `mainMenu` |
| `GameOverScene` | opaque, blocks update, not pausable | Menu or cancel → `ReplaceScene` `mainMenu` |

`RequestPause` is the only way onto `PauseScene`. The stack's pause factory supplies it. Focus loss pauses only while the top scene is pausable, so the menu, pause overlay, and game-over screen stay put.

Buttons are `MenuModel` rows: main menu **Play** / **Quit**, pause **Resume** / **Menu**, game over **Menu**. Game over shows the sim score for the run that just ended and the high-score table.

`main.cpp` opens `SfmlPlatform` **1280×720** titled `"Tower"`, loads the UI font, optionally loads the atlas into `SpriteBank`, uploads `soundSpecs()`, loads scores via `ScoreStore` (temp fallback), builds `AppServices` with constant default seed **42**, constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Playfield

`PlayScene::render` reads `player()`, `platforms()`, `cameraBottom()`, and `score()`. It does not move them. Sim **+Y is up**. Screen **+Y is down**. `RectCmd::rect.pos` and `SpriteCmd::position` are the top-left of the unrotated bounds.

A world point maps with:

`screenY = kViewH - (worldY - cameraBottom)`

`screenX = worldX` (the view is the full `kWorldW`; the camera has no X).

Body and platform `pos` is the bottom-left, so the rect's screen top-left uses the world top `pos.y + size.y`:

`screenTop = kViewH - (pos.y + size.y - cameraBottom)`

When `SpriteBank` has the region, play draws a `SpriteCmd`: `bg/sky`, `plat/ice` on each platform, `player/idle` or `player/jump`, and optional `bonus/speed`, `bonus/jump`, `bonus/points`. Missing regions fall back to `RectCmd`s (`fieldFill`, `platformFill`, `playerFill`, Theme bonus fills). Score is a `TextCmd` (`"Score N"`) on `Layer::Hud`. The number is the sim score for this run.

`PlayScene::game()` exposes the sim for headless tests. Those tests drive `SceneStack` with `InputState` and open no window (`tower_scenes_test`, `tower_play_scene_test`, `tower_sounds_test`, `tower_feedback_test`, `tower_sprite_bank_test`).
