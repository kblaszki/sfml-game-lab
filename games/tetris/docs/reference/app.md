---
title: Tetris app
diataxis: reference
audience: [ai, human]
related_code:
  - games/tetris/app/include/tetris/app/Theme.hpp
  - games/tetris/app/include/tetris/app/Bindings.hpp
  - games/tetris/app/include/tetris/app/InputMapping.hpp
  - games/tetris/app/include/tetris/app/BoardRender.hpp
  - games/tetris/app/include/tetris/app/HudModel.hpp
  - games/tetris/app/include/tetris/app/Sounds.hpp
  - games/tetris/app/include/tetris/app/Feedback.hpp
  - games/tetris/app/include/tetris/app/Scenes.hpp
  - games/tetris/app/include/tetris/app/MainMenuScene.hpp
  - games/tetris/app/include/tetris/app/PlayScene.hpp
  - games/tetris/app/include/tetris/app/PauseScene.hpp
  - games/tetris/app/include/tetris/app/GameOverScene.hpp
  - games/tetris/app/src/Bindings.cpp
  - games/tetris/app/src/InputMapping.cpp
  - games/tetris/app/src/BoardRender.cpp
  - games/tetris/app/src/HudModel.cpp
  - games/tetris/app/src/Sounds.cpp
  - games/tetris/app/src/Feedback.cpp
  - games/tetris/app/src/Scenes.cpp
  - games/tetris/app/src/MainMenuScene.cpp
  - games/tetris/app/src/PlayScene.cpp
  - games/tetris/app/src/PauseScene.cpp
  - games/tetris/app/src/GameOverScene.cpp
  - games/tetris/main.cpp
  - games/tetris/app/CMakeLists.txt
  - tests/tetris/app/InputMappingTest.cpp
  - tests/tetris/app/BoardRenderTest.cpp
  - tests/tetris/app/HudTest.cpp
  - tests/tetris/app/ScenesTest.cpp
  - tests/tetris/app/SoundsTest.cpp
  - tests/tetris/app/FeedbackTest.cpp
related_docs:
  - sim.md
  - ../../../../docs/reference/engine-core.md
  - ../../../../docs/reference/engine-input.md
  - ../../../../docs/reference/engine-resources.md
  - ../../../../docs/reference/engine-scene.md
  - ../../../../docs/reference/engine-audio.md
  - ../../../../docs/reference/engine-fx.md
  - ../../../../docs/reference/engine-save.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [tetris, app, Theme, Bindings, InputMapping, BoardRender, HudModel, Sounds, Feedback, Scenes, PlayScene, GameOverScene, MenuModel]
last_reviewed: 2026-09-29
---

# Tetris app

Presentation shell in `namespace sgl::tetris` for the playable Tetris window. Static library target `tetris_app` links `PUBLIC` `tetris_sim`, `sgl_loop`, `sgl_resources`, `sgl_audio`, `sgl_fx`, `sgl_save`, and `sgl_ui`. No SFML includes — cells are `RectCmd`, input uses `sgl::ActionMap`, playback goes through `AudioI`, menus use `sgl::MenuModel` / `sgl::renderMenu`. Include as `<tetris/app/X.hpp>`. Executable target `tetris` (`games/tetris/main.cpp`) links `tetris_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root, and depends on `check_boundaries`.

## Theme

`Theme.hpp` holds design size **1280×720**, cell size **30**, ghost alpha **70**, guideline piece colors, HUD anchors, menu/overlay colors, and `uiFontRelativePath` (`"fonts/upheavtt.ttf"`).

## Bindings

`makeActions()` builds `Actions` with explicit ids **1–10** (`confirm`, `cancel`, `pause`, `left`, `right`, `softDrop`, `hardDrop`, `rotateCw`, `rotateCcw`, `hold`). `defaultBindings` maps:

| Keys | Action |
|------|--------|
| Enter | confirm |
| Backspace | cancel |
| Escape, P | pause |
| Left / Right | left / right |
| Down | softDrop |
| Space | hardDrop |
| Up, X | rotateCw |
| Z | rotateCcw |
| C, LShift | hold |

`menuActions(actions)` builds `sgl::MenuActions` with `up = rotateCw`, `down = softDrop`, plus `confirm` / `cancel` — so Up/Down navigate menus while play still uses those keys for rotate / soft drop.

## Input mapping

`toTetrisInput(const InputState&, const Actions&)` copies `pressed`/`held` edges into `TetrisInput`. Soft drop uses `held`; hard drop, rotates, and hold use `pressed`. A tap that is pressed and released before the next sim step still sets `pressed` because `InputState` keeps the edge until `beginFrame`.

## Board render

`Layout` places a centered well, hold panel (left), and next panel (right). `defaultLayout()` uses cell **30** and preview cell **20**. `renderBoard` draws with **`RectCmd` only**: background, well, locked tiles for the **20** visible rows (`std::views::take(kVisibleHeight)`), ghost (alpha 70) under the active piece on `Layer::Actors`, next queue of **5**, and hold. Buffer rows above 19 are skipped.

## HUD

`makeHud(const Score&)` fills `HudModel` with `"Score N"`, `"Lines N"`, and `"Level N"`. `PlayScene` pushes those strings as `TextCmd`.

## Sounds

`soundSpecs()` returns a span of `{key, Pcm}` for **move**, **rotate**, **lock**, **line1**, **line2**, **line3**, **tetris**, **levelUp**, and **gameOver**. Each buffer is built with `sgl::blip`. `SoundIds` holds the uploaded `SoundId` handles. `main.cpp` uploads every spec through `platform.audio().upload` (`SfmlAudio&`) and stores the ids in `AppServices`.

## Feedback

`Feedback(AudioI&, SoundIds, Pcg32)` maps `TetrisEvent`s to playback and particles (`ParticleSystem` capacity **512**, **12** particles per cleared row). `onEvents` plays one sound per relevant event (`PieceShifted` → move, `PieceRotated` → rotate, `PieceLocked` → lock, `LinesCleared` → line1/2/3 or tetris, `LevelUp`, `GameOver`) and emits one burst per cleared **row index** from `LinesCleared::rows` (well Y from `defaultLayout()`, not `0..count-1`). `update(Seconds)` and `render(RenderQueue&)` draw on `Layer::Overlay`. `PlayScene` owns `Feedback` and feeds it `game_.step` events each tick.

## High scores

Capacity **5**. Path is `userDataDir("sfml-game-lab") / "tetris.scores"`, or `temp_directory_path() / "tetris.scores"` if `userDataDir` fails (process does not abort). `main.cpp` loads with `ScoreStore::load`. `GameOverScene` inserts the final score, shows **"New record"** when it ranks, and saves with `ScoreStore::save`; a save failure shows `"Save failed: …"` and is not fatal. Menu and game over both show the current best (`"Best N"` or `"Best --"`).

## Scenes

`AppServices` is `{const Actions& actions; FontId font; uint64_t seed; AudioI& audio; SoundIds sounds; path scoresPath; HighScoreTable& highScores}`. Comment on the type: references are owned by `main()` for the whole `App::run()`; scenes must not outlive that. Factories return `sgl::SceneFactory`:

| Factory | Scene | Traits |
|---------|-------|--------|
| `mainMenu(services)` | `MainMenuScene` | opaque, blocksUpdate, not pausable |
| `play(services)` | `PlayScene` | opaque, blocksUpdate, pausable while `!over()` |
| `pause(services)` | `PauseScene` | not opaque, blocksUpdate, not pausable |
| `gameOver(services, score)` | `GameOverScene` | not opaque, blocksUpdate, not pausable |

`MainMenuScene` title `"Tetris"`, best score, then `MenuModel` Start/Quit via `renderMenu`. Activate Start → `ReplaceScene{play}`; Quit or cancel → `QuitApp`.

`PlayScene` owns `TetrisGame(services.seed)` and `Feedback`. Each update maps input, calls `step`, runs feedback, and on game over `PushScene{gameOver(...)}`. Pause → `RequestPause`. Exposes `game()`, `hud()`, and `feedback()`.

`PauseScene` keeps its type; navigation uses `MenuModel` (Resume / Menu) plus pause-key resume. Pause or cancel → `PopScene`; Resume → `PopScene`; Menu → `PopScene` then `ReplaceScene{mainMenu}`.

`GameOverScene` shows the final score, best, optional new-record / save-error lines, and Retry/Menu through `MenuModel` + `renderMenu`. Retry → `PopScene` then `ReplaceScene{play}`. Menu or cancel → `PopScene` then `ReplaceScene{mainMenu}`. Traits align with arkanoid `ResultScene`: `opaque = false`.

`main.cpp` opens `SfmlPlatform` **1280×720** titled `"Tetris"`, loads the UI font, uploads `soundSpecs()`, loads scores via `ScoreStore` (temp fallback), builds `AppServices` with constant default seed **42** (not from the clock), constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Tests

Headless suites under `tests/tetris/app/`: `InputMappingTest`, `BoardRenderTest`, `HudTest`, `ScenesTest` (including `gameOverRecordsHighScore` with a temp scores path), `SoundsTest`, `FeedbackTest` (`AudioSpy`, including burst origins for cleared rows 5 and 19).
