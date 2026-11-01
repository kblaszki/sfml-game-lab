---
title: Dodger app
diataxis: reference
audience: [ai, human]
related_code:
  - games/dodger/app/include/dodger/app/Theme.hpp
  - games/dodger/app/include/dodger/app/Bindings.hpp
  - games/dodger/app/include/dodger/app/InputMapping.hpp
  - games/dodger/app/include/dodger/app/Sounds.hpp
  - games/dodger/app/include/dodger/app/Feedback.hpp
  - games/dodger/app/include/dodger/app/Scenes.hpp
  - games/dodger/app/include/dodger/app/MainMenuScene.hpp
  - games/dodger/app/include/dodger/app/PlayScene.hpp
  - games/dodger/app/include/dodger/app/PauseScene.hpp
  - games/dodger/app/include/dodger/app/GameOverScene.hpp
  - games/dodger/app/src/Bindings.cpp
  - games/dodger/app/src/InputMapping.cpp
  - games/dodger/app/src/Sounds.cpp
  - games/dodger/app/src/Feedback.cpp
  - games/dodger/app/src/Scenes.cpp
  - games/dodger/app/src/MainMenuScene.cpp
  - games/dodger/app/src/PlayScene.cpp
  - games/dodger/app/src/PauseScene.cpp
  - games/dodger/app/src/GameOverScene.cpp
  - games/dodger/app/CMakeLists.txt
  - games/dodger/main.cpp
  - games/dodger/CMakeLists.txt
  - tests/dodger/app/ScenesTest.cpp
  - tests/dodger/app/PlaySceneTest.cpp
  - tests/dodger/app/SoundsTest.cpp
  - tests/dodger/app/FeedbackTest.cpp
  - tests/dodger/app/CMakeLists.txt
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
keywords: [dodger, app, bindDodger, DodgerActions, PlayScene, PauseScene, GameOverScene, MainMenuScene, SceneStack, RectCmd, Sounds, Feedback, HighScoreTable]
last_reviewed: 2026-11-01
---

# Dodger app

Presentation shell in `namespace sgl::dodger`. Static library `dodger_app` links `PUBLIC` `dodger_sim`, `sgl_scene`, `sgl_render`, `sgl_input`, `sgl_ui`, `sgl_loop`, `sgl_audio`, and `sgl_save` (`sgl_core` comes in through those). No SFML includes. Include as `<dodger/app/X.hpp>`.

Executable `dodger` (`games/dodger/main.cpp`) links `dodger_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`. The window is **1280×720**, title **"Dodger"**. `main` loads `fonts/upheavtt.ttf`, uploads the death blip, loads the high-score table, calls `bindDodger`, and pushes `mainMenu` on a `SceneStack` whose pause factory is `pause`.

## Theme

`Theme.hpp` holds design size **1280×720** (the sim world `kWorldW` × `kWorldH`), menu and field colors, the HUD score anchor, and `uiFontRelativePath` (`"fonts/upheavtt.ttf"`).

## Bindings

`bindDodger(ActionMap&)` assigns `DodgerActions` ids **1–5** and binds them:

| Keys | Action |
|------|--------|
| Left, A | left |
| Right, D | right |
| Enter | confirm |
| Backspace | cancel |
| Escape, P | pause |

`menuActions(actions)` builds `sgl::MenuActions` with `up = left`, `down = right`, plus `confirm` / `cancel`. Left / A moves the highlight to the previous button. Right / D moves it to the next. Enter activates the focused button.

## Input mapping

`toSimInput` copies **held** left and right into `SimInput`. A key that stays down keeps the paddle moving. Both together cancel inside the sim. Pause and menus read the pressed edge themselves.

## Sounds

`soundSpecs()` returns a span of `{key, Pcm}` for **die** only. The buffer is built with `sgl::blip`. `SoundIds` holds the uploaded `SoundId` handle (`die`). `main.cpp` uploads every spec through `platform.audio().upload` (`SfmlAudio&`) and stores the id in `AppServices`. There is no move blip and no near-miss blip.

## Feedback

`Feedback(AudioI&, SoundIds)` maps `DodgerEvent`s to playback. `onEvents` plays **die** for `Died` at volume `1`. `HazardCleared` is silent. `PlayScene` owns `Feedback` and feeds it the `game_.step` events each tick, including the tick that replaces the scene on `Died`.

## High scores

Capacity **5** (`kHighScoreCapacity`). Path is `userDataDir("dodger") / "dodger.scores"`, or `temp_directory_path() / "dodger.scores"` if `userDataDir` fails (the process does not abort). `main.cpp` loads with `ScoreStore::load`. `GameOverScene` inserts the final sim score (`int` stored as `uint32_t`), lists the table as ranking lines `"#N score"` (1-based, best first), shows **"New record"** when the score ranks, and saves with `ScoreStore::save`. The inserted line uses the title color. A save failure shows `"Save failed: …"` and the scene stays up. A score that does not qualify is not written.

## Scenes

Factories in `Scenes.hpp`: `mainMenu`, `play`, `pause`, `gameOver(services, score)`. Each closes over `AppServices`:

`{const DodgerActions& actions; FontId font; uint64_t seed; AudioI& audio; SoundIds sounds; path scoresPath; HighScoreTable& highScores; Tuning tuning}`

References are owned by `main()` for the whole `App::run()`; scenes must not outlive that. `PlayScene` constructs `DodgerGame` with that seed and tuning, and `Feedback` from `audio` and `sounds`. The default seed in `main` is **42** and the default `Tuning` is the sim default. A test can change `tuning` before it builds `play` (a full-width paddle and a hazard speed that lands on `kPaddleY` in one step).

| Scene | Traits | Requests |
|-------|--------|----------|
| `MainMenuScene` | opaque, blocks update, not pausable | Play → `ReplaceScene` `play`. Quit or cancel → `QuitApp` |
| `PlayScene` | opaque, blocks update, pausable | Escape / P → `RequestPause` (no step that frame). After `step`, `Feedback::onEvents`. `Died` → `ReplaceScene` `gameOver` with `score()` |
| `PauseScene` | not opaque, blocks update, not pausable | Resume, pause, or cancel → `PopScene`. Menu → `PopScene` then `ReplaceScene` `mainMenu` |
| `GameOverScene` | opaque, blocks update, not pausable | Menu or cancel → `ReplaceScene` `mainMenu` |

`RequestPause` is the only way onto `PauseScene`. The stack's pause factory supplies it. Focus loss pauses only while the top scene is pausable, so the menu, pause overlay, and game-over screen stay put.

Buttons are `MenuModel` rows: main menu **Play** / **Quit**, pause **Resume** / **Menu**, game over **Menu**.

`main.cpp` opens `SfmlPlatform` **1280×720** titled `"Dodger"`, loads the UI font, uploads `soundSpecs()`, loads scores via `ScoreStore` (temp fallback), builds `AppServices` with constant default seed **42**, constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Playfield

`PlayScene::render` reads `paddle()`, `hazards()`, and `score()`. It does not move them. Sim **+Y is down**, the same direction as the screen, so a view position is the draw position. There is no Y flip.

The paddle and each hazard are `RectCmd`s on `Layer::Actors`, using the view `pos` and `size`. The field is a `RectCmd` on `Layer::Background`. Score is a `TextCmd` (`"Score N"`) on `Layer::Hud`. The number is the sim's live count of cleared hazards for this run.

`PlayScene::game()` exposes the sim for headless tests. Those tests drive `SceneStack` with `InputState` and open no window (`dodger_scenes_test`, `dodger_play_scene_test`, `dodger_sounds_test`, `dodger_feedback_test`). `FeedbackTest` uses `AudioSpy`. `ScenesTest` records a score on a temp path and checks that a failed save still shows game over.
