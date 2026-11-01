---
title: Snake app
diataxis: reference
audience: [ai, human]
related_code:
  - games/snake/app/include/snake/app/Theme.hpp
  - games/snake/app/include/snake/app/Bindings.hpp
  - games/snake/app/include/snake/app/InputMapping.hpp
  - games/snake/app/include/snake/app/Sounds.hpp
  - games/snake/app/include/snake/app/Feedback.hpp
  - games/snake/app/include/snake/app/Scenes.hpp
  - games/snake/app/include/snake/app/MainMenuScene.hpp
  - games/snake/app/include/snake/app/PlayScene.hpp
  - games/snake/app/include/snake/app/PauseScene.hpp
  - games/snake/app/include/snake/app/GameOverScene.hpp
  - games/snake/app/src/Bindings.cpp
  - games/snake/app/src/InputMapping.cpp
  - games/snake/app/src/Sounds.cpp
  - games/snake/app/src/Feedback.cpp
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
  - tests/snake/app/SoundsTest.cpp
  - tests/snake/app/FeedbackTest.cpp
  - tests/snake/app/CMakeLists.txt
related_docs:
  - sim.md
  - ../explanation/design.md
  - ../index.md
  - ../../../../docs/reference/engine-scene.md
  - ../../../../docs/reference/engine-input.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/engine-render.md
  - ../../../../docs/reference/engine-audio.md
  - ../../../../docs/reference/engine-save.md
  - ../../../../docs/reference/pause-overlay.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [snake, app, bindSnake, SnakeActions, PlayScene, PauseScene, GameOverScene, MainMenuScene, SceneStack, Sounds, Feedback, HighScoreTable]
last_reviewed: 2026-11-01
---

# Snake app

Presentation shell in `namespace sgl::snake`. Static library `snake_app` links `PUBLIC` `snake_sim`, `sgl_scene`, `sgl_render`, `sgl_input`, `sgl_ui`, `sgl_loop`, `sgl_resources`, `sgl_audio`, and `sgl_save` (`sgl_core` comes in through those). No SFML includes. Include as `<snake/app/X.hpp>`.

Executable `snake` (`games/snake/main.cpp`) links `snake_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`. The window is **1280×720**, title **"Snake"**. `main` loads `fonts/upheavtt.ttf`, uploads `soundSpecs()`, loads the high-score table, calls `bindSnake`, and pushes `mainMenu` on a `SceneStack` whose pause factory is `pause`.

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

## Sounds

`soundSpecs()` returns a span of `{key, Pcm}` for **eat** and **die**. Each buffer is built with `sgl::blip`. `SoundIds` holds the uploaded `SoundId` handles (`eat`, `die`). `main.cpp` uploads every spec through `platform.audio().upload` (`SfmlAudio&`) and stores the ids in `AppServices`.

## Feedback

`Feedback(AudioI&, SoundIds)` maps `SnakeEvent`s to playback. `onEvents` plays **eat** for `Ate` and **die** for `Died` at volume `1`. `Moved` is silent. `PlayScene` owns `Feedback` and feeds it the `game_.step` events each tick, including the tick that replaces the scene on `Died`.

## High scores

Capacity **5** (`kHighScoreCapacity`). Path is `userDataDir("snake") / "snake.scores"`, or `temp_directory_path() / "snake.scores"` if `userDataDir` fails (the process does not abort). `main.cpp` loads with `ScoreStore::load`. `GameOverScene` inserts the final sim score (`int` stored as `uint32_t`), lists the table as ranking lines `"#N score"` (1-based, best first), shows **"New record"** when the score ranks, and saves with `ScoreStore::save`. The inserted line uses the title color. A save failure shows `"Save failed: …"` and the scene stays up. A score that does not qualify is not written.

## Scenes

Factories in `Scenes.hpp`: `mainMenu`, `play`, `pause`, `gameOver(services, score)`. Each closes over `AppServices`:

`{const SnakeActions& actions; FontId font; uint64_t seed; AudioI& audio; SoundIds sounds; path scoresPath; HighScoreTable& highScores}`

References are owned by `main()` for the whole `App::run()`; scenes must not outlive that. `PlayScene` constructs `SnakeGame` with that seed and `Feedback` from `audio` and `sounds`.

| Scene | Traits | Requests |
|-------|--------|----------|
| `MainMenuScene` | opaque, blocks update, not pausable | Play → `ReplaceScene` `play`. Quit or cancel → `QuitApp` |
| `PlayScene` | opaque, blocks update, pausable while `over()` is false | Escape / P → `RequestPause` (no step that frame). After `step`, `Feedback::onEvents`. `Died` → `ReplaceScene` `gameOver` with `score()` |
| `PauseScene` | not opaque, blocks update, not pausable | Resume, pause, or cancel → `PopScene`. Menu → `PopScene` then `ReplaceScene` `mainMenu` |
| `GameOverScene` | opaque, blocks update, not pausable | Menu or cancel → `ReplaceScene` `mainMenu`. Quit → `QuitApp` |

`RequestPause` is the only way onto `PauseScene`. The stack's pause factory supplies it. Focus loss pauses only while the top scene is pausable, so the menu, pause overlay, and game-over screen stay put.

Buttons are `MenuModel` rows: main menu **Play** / **Quit**, pause **Resume** / **Menu**, game over **Menu** / **Quit**.

`main.cpp` opens `SfmlPlatform` **1280×720** titled `"Snake"`, loads the UI font, uploads `soundSpecs()`, loads scores via `ScoreStore` (temp fallback), builds `AppServices` with constant default seed **42**, constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Board

`PlayScene::render` pushes one `RectCmd` per cell (40×40). Sim **+y is up**, so screen y is `(kRows - 1 - cell.y) * cellSize` and cell `(0, 0)` sits at the bottom left. Body segments are inset rects; the head (`body()[0]`) uses a distinct fill. Food is a `CircleCmd` when `food()` lies on the board. Score is a `TextCmd` (`"Score N"`) on `Layer::Hud`.

`PlayScene::game()` exposes the sim for headless tests. Those tests drive `SceneStack` with `InputState` and open no window (`snake_scenes_test`, `snake_play_scene_test`, `snake_sounds_test`, `snake_feedback_test`). `FeedbackTest` uses `AudioSpy`. `ScenesTest` records a score on a temp path and checks that a failed save still shows game over.
