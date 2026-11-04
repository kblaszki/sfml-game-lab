---
title: Asteroids app
diataxis: reference
audience: [ai, human]
related_code:
  - games/asteroids/app/include/asteroids/app/Theme.hpp
  - games/asteroids/app/include/asteroids/app/Bindings.hpp
  - games/asteroids/app/include/asteroids/app/InputMapping.hpp
  - games/asteroids/app/include/asteroids/app/Sounds.hpp
  - games/asteroids/app/include/asteroids/app/Feedback.hpp
  - games/asteroids/app/include/asteroids/app/SpriteBank.hpp
  - games/asteroids/app/include/asteroids/app/Scenes.hpp
  - games/asteroids/app/include/asteroids/app/MainMenuScene.hpp
  - games/asteroids/app/include/asteroids/app/PlayScene.hpp
  - games/asteroids/app/include/asteroids/app/PauseScene.hpp
  - games/asteroids/app/include/asteroids/app/GameOverScene.hpp
  - games/asteroids/app/src/Bindings.cpp
  - games/asteroids/app/src/InputMapping.cpp
  - games/asteroids/app/src/Sounds.cpp
  - games/asteroids/app/src/Feedback.cpp
  - games/asteroids/app/src/SpriteBank.cpp
  - games/asteroids/app/src/Scenes.cpp
  - games/asteroids/app/src/MainMenuScene.cpp
  - games/asteroids/app/src/PlayScene.cpp
  - games/asteroids/app/src/PauseScene.cpp
  - games/asteroids/app/src/GameOverScene.cpp
  - games/asteroids/app/CMakeLists.txt
  - games/asteroids/main.cpp
  - games/asteroids/CMakeLists.txt
  - tests/asteroids/app/ScenesTest.cpp
  - tests/asteroids/app/PlaySceneTest.cpp
  - tests/asteroids/app/SoundsTest.cpp
  - tests/asteroids/app/FeedbackTest.cpp
  - tests/asteroids/app/SpriteBankTest.cpp
  - tests/asteroids/app/CMakeLists.txt
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
keywords: [asteroids, app, bindAsteroids, AsteroidsActions, PlayScene, PauseScene, GameOverScene, MainMenuScene, SceneStack, Sounds, Feedback, HighScoreTable, SpriteBank]
last_reviewed: 2026-11-04
---

# Asteroids app

Presentation shell in `namespace sgl::asteroids`. Static library `asteroids_app` links `PUBLIC` `asteroids_sim`, `sgl_scene`, `sgl_render`, `sgl_input`, `sgl_ui`, `sgl_loop`, `sgl_resources`, `sgl_audio`, and `sgl_save` (`sgl_core` comes in through those). The link list matches `snake_app`. No SFML includes. Include as `<asteroids/app/X.hpp>`.

Executable `asteroids` (`games/asteroids/main.cpp`) links `asteroids_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`. The window is **1280×720**, title **"Asteroids"**. `main` loads `fonts/upheavtt.ttf`, optionally loads `asteroids/atlas.png` + `atlas.txt` into `SpriteBank` (see [assets.md](assets.md)), uploads `soundSpecs()`, loads the high-score table, calls `bindAsteroids`, and pushes `mainMenu` on a `SceneStack` whose pause factory is `pause`.

## Theme

`Theme.hpp` holds design size **1280×720** (the sim world `kWorldW` × `kWorldH`), menu and space colors, the HUD score anchor, and `uiFontRelativePath` (`"fonts/upheavtt.ttf"`).

## Bindings

`bindAsteroids(ActionMap&)` assigns `AsteroidsActions` ids **1–7** and binds them:

| Keys | Action |
|------|--------|
| Up, W | thrust |
| Right, D | rotateCw |
| Left, A | rotateCcw |
| Space | fire |
| Enter | confirm |
| Backspace | cancel |
| Escape, P | pause |

`menuActions(actions)` builds `sgl::MenuActions` with `up = rotateCcw`, `down = rotateCw`, plus `confirm` / `cancel`. Left / A moves the highlight to the previous button. Right / D moves it to the next. Enter activates the focused button.

## Input mapping

`toSimInput` copies **held** thrust, rotateCw, rotateCcw, and fire into `SimInput`. A key that stays down keeps turning, thrusting, and firing. The sim still applies `fireCooldown` and cancels rotation when both turn flags are set. Pause and menus read the pressed edge themselves.

## Sounds

`soundSpecs()` returns a span of `{key, Pcm}` for **fire**, **hit**, and **die**. Each buffer is built with `sgl::blip`. `SoundIds` holds the uploaded `SoundId` handles (`fire`, `hit`, `die`). `main.cpp` uploads every spec through `platform.audio().upload` (`SfmlAudio&`) and stores the ids in `AppServices`.

## Feedback

`Feedback(AudioI&, SoundIds)` maps `AsteroidsEvent`s to playback. `onEvents` plays **fire** for `ShotFired`, **hit** for `RockHit`, and **die** for `Died` at volume `1`. `WaveCleared` is silent. `PlayScene` owns `Feedback` and feeds it the `game_.step` events each tick, including the tick that replaces the scene on `Died`.

## High scores

Capacity **5** (`kHighScoreCapacity`). Path is `userDataDir("asteroids") / "asteroids.scores"`, or `temp_directory_path() / "asteroids.scores"` if `userDataDir` fails (the process does not abort). `main.cpp` loads with `ScoreStore::load`. `GameOverScene` inserts the final sim score (`int` stored as `uint32_t`), lists the table as ranking lines `"#N score"` (1-based, best first), shows **"New record"** when the score ranks, and saves with `ScoreStore::save`. The inserted line uses the title color. A save failure shows `"Save failed: …"` and the scene stays up. A score that does not qualify is not written.

## Scenes

Factories in `Scenes.hpp`: `mainMenu`, `play`, `pause`, `gameOver(services, score)`. Each closes over `AppServices`:

`{const AsteroidsActions& actions; FontId font; uint64_t seed; AudioI& audio; SoundIds sounds; path scoresPath; HighScoreTable& highScores; Tuning tuning; SpriteBank sprites}`

References are owned by `main()` for the whole `App::run()`; scenes must not outlive that. `SpriteBank` is a value (empty when the atlas is absent). `PlayScene` constructs `AsteroidsGame` with that seed and tuning, and `Feedback` from `audio` and `sounds`. The default seed in `main` is **42** and the default `Tuning` is the sim default.

| Scene | Traits | Requests |
|-------|--------|----------|
| `MainMenuScene` | opaque, blocks update, not pausable | Play → `ReplaceScene` `play`. Quit or cancel → `QuitApp` |
| `PlayScene` | opaque, blocks update, pausable | Escape / P → `RequestPause` (no step that frame). After `step`, `Feedback::onEvents`. `Died` → `ReplaceScene` `gameOver` with `score()` |
| `PauseScene` | not opaque, blocks update, not pausable | Resume, pause, or cancel → `PopScene`. Menu → `PopScene` then `ReplaceScene` `mainMenu` |
| `GameOverScene` | opaque, blocks update, not pausable | Menu or cancel → `ReplaceScene` `mainMenu` |

`RequestPause` is the only way onto `PauseScene`. The stack's pause factory supplies it. Focus loss pauses only while the top scene is pausable, so the menu, pause overlay, and game-over screen stay put.

Buttons are `MenuModel` rows: main menu **Play** / **Quit**, pause **Resume** / **Menu**, game over **Menu**.

`main.cpp` opens `SfmlPlatform` **1280×720** titled `"Asteroids"`, loads the UI font, optionally loads the atlas into `SpriteBank`, uploads `soundSpecs()`, loads scores via `ScoreStore` (temp fallback), builds `AppServices` with constant default seed **42**, constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Playfield

`PlayScene::render` reads `ship()`, `rocks()`, `bullets()`, and `score()`. It does not move them. Draw space flips Y so sim **+Y is up**: `screen = (sim.x, kWorldH - sim.y)`. Heading **0** points along **+X**. Positive heading turns the nose toward sim +Y, which is up on screen.

When `SpriteBank` has regions, play draws `SpriteCmd`s: `bg/space`, `rock/{large|medium|small}`, `bullet`, and `ship/player` (rotated with `rotationDeg = 90 - heading * 180/π`). Missing regions fall back to the procedural path: full-screen `RectCmd`, rock/bullet `CircleCmd`s, and a ship `MeshCmd` triangle (nose `1.6` radii along heading; base `0.9` back and `0.95` to each side). Score is a `TextCmd` (`"Score N"`) on `Layer::Hud`.

`PlayScene::game()` exposes the sim for headless tests. Those tests drive `SceneStack` with `InputState` and open no window (`asteroids_scenes_test`, `asteroids_play_scene_test`, `asteroids_sounds_test`, `asteroids_feedback_test`, `asteroids_sprite_bank_test`). `FeedbackTest` uses `AudioSpy`. `ScenesTest` records a score on a temp path and checks that a failed save still shows game over.
