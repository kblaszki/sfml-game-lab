---
title: Syndicate app
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/CMakeLists.txt
  - games/syndicate/main.cpp
  - games/syndicate/app/CMakeLists.txt
  - games/syndicate/app/include/syndicate/app/Theme.hpp
  - games/syndicate/app/include/syndicate/app/LayoutView.hpp
  - games/syndicate/app/include/syndicate/app/BoardRender.hpp
  - games/syndicate/app/include/syndicate/app/PawnAnimator.hpp
  - games/syndicate/app/include/syndicate/app/Bindings.hpp
  - games/syndicate/app/include/syndicate/app/HudModel.hpp
  - games/syndicate/app/include/syndicate/app/PlayScene.hpp
  - games/syndicate/app/include/syndicate/app/Scenes.hpp
  - games/syndicate/app/include/syndicate/app/Sounds.hpp
  - games/syndicate/app/include/syndicate/app/Feedback.hpp
  - games/syndicate/app/include/syndicate/app/MainMenuScene.hpp
  - games/syndicate/app/include/syndicate/app/SetupScene.hpp
  - games/syndicate/app/include/syndicate/app/GameOverScene.hpp
  - games/syndicate/app/src/Theme.cpp
  - games/syndicate/app/src/LayoutView.cpp
  - games/syndicate/app/src/BoardRender.cpp
  - games/syndicate/app/src/PawnAnimator.cpp
  - games/syndicate/app/src/Bindings.cpp
  - games/syndicate/app/src/HudModel.cpp
  - games/syndicate/app/src/PlayScene.cpp
  - games/syndicate/app/src/Sounds.cpp
  - games/syndicate/app/src/Feedback.cpp
  - games/syndicate/app/src/Scenes.cpp
  - games/syndicate/app/src/MainMenuScene.cpp
  - games/syndicate/app/src/SetupScene.cpp
  - games/syndicate/app/src/GameOverScene.cpp
  - tests/syndicate/app/CMakeLists.txt
  - tests/syndicate/app/LayoutViewTest.cpp
  - tests/syndicate/app/BoardRenderTest.cpp
  - tests/syndicate/app/PawnAnimatorTest.cpp
  - tests/syndicate/app/HudModelTest.cpp
  - tests/syndicate/app/PlaySceneTest.cpp
  - tests/syndicate/app/ScenesTest.cpp
  - tests/syndicate/app/FeedbackTest.cpp
related_docs:
  - syndicate-sim.md
  - ../explanation/syndicate-design.md
  - engine-render.md
  - engine-ui.md
  - engine-save.md
  - source-layout.md
  - ../explanation/architecture.md
keywords: [syndicate, app, Theme, LayoutView, renderBoard, PawnAnimator, Bindings, HudModel, PlayScene, Scenes, Feedback, Sounds, MainMenuScene, SetupScene, GameOverScene, syndicate executable, 1280x720]
last_reviewed: 2026-09-29
---

# Syndicate app

Presentation layer and playable executable for Syndicate in `namespace sgl::syndicate`. Static library `syndicate_app` links `PUBLIC` `syndicate_sim`, `sgl_render`, `sgl_scene`, `sgl_core`, `sgl_input`, `sgl_ui`, `sgl_audio`, `sgl_fx`, and `sgl_save`. No SFML includes in the library — drawing uses `sgl::RenderQueue` and `CircleCmd` / `LineCmd` / `RectCmd` / `TextCmd`. Include as `<syndicate/app/X.hpp>`. Design size **1280×720**. Executable target `syndicate` (`games/syndicate/main.cpp`) links `syndicate_app` and `sgl_sfml`.

## SyndicateServices

`SyndicateServices` holds references owned by `main()` for the whole `App::run()`: `actions`, `font`, `seed`, `audio`, `sounds`, `scoresPath`, `highScores`. Scene factories in `Scenes.hpp` capture this struct by value (shallow copies of references).

| Factory | Scene |
|---------|--------|
| `mainMenu` | `MainMenuScene` (Play / Quit) |
| `setup` | `SetupScene` (2–4 seats, Human/Bot, Confirm) |
| `play` | `PlayScene` with `MatchConfig` + seats |
| `pause` | `sgl::PauseOverlay` |
| `makeGameOver` | `GameOverScene` with ranking + high-score save |

## Sounds / Feedback

`soundSpecs()` returns PCM from `sgl::blip` (`roll`, `fee`, `auction`, `card`, `market`). `main.cpp` uploads via `SfmlAudio`; the app library only holds `SoundIds` / specs. `Feedback(AudioI&, SoundIds)` plays on `Rolled`, `FeePaid`, `AuctionDone`, `CardPlayed`, `MarketShifted`; other `GameEvent` alternatives are silent.

## Menu / setup / game over

- **MainMenuScene:** `MenuModel` + `renderMenu`. Play → `PushScene{setup}`; Quit / cancel / Escape → `QuitApp`.
- **SetupScene:** cycles seat count 2→3→4, toggles Human/Bot per seat, Confirm → `ReplaceScene{play}` with `MatchConfig{seed, seats, startCash 1500, maxRounds 15, targetNetWorth 6000}`.
- **GameOverScene:** ranking lines (`P1`… by player id, net worth). Menu → pop + replace main menu; Quit → `QuitApp`. Inserts the winner's net worth into `HighScoreTable` and `ScoreStore::save`; save failure still shows the scene.

## Theme / layout / board / HUD

Unchanged from prior slices: `Theme` / `defaultTheme()`, `LayoutView`, `renderBoard`, `PawnAnimator`, `HudModel`. Bindings: **Space** roll, **Enter** end turn / menu confirm, **C** bid, **P** pass, **Escape** pause, **Up/Down** menu, **Backspace** cancel.

## PlayScene

Two constructors: test-friendly `(MatchConfig, seats, FontId, SyndicateActions)` and services `(SyndicateServices, MatchConfig, seats)` which also owns `Feedback`. Pause → `RequestPause`. On `GameOver`, pushes `GameOverScene` (via `makeGameOver` when services are present). `drainEvents` feeds `Feedback` after each applied command.

## main.cpp

`SfmlPlatform` **1280×720** titled **"Syndicate"**, loads `uiFontRelativePath` under `ASSET_DIR`, uploads sounds, resolves `userDataDir("syndicate") / "syndicate.scores"` (temp fallback), `bindSyndicate`, pushes main menu on a `SceneStack` with the pause factory, runs `sgl::App`. Seed from `std::chrono::steady_clock`.

## Tests

| Suite | Focus |
|-------|-------|
| `syndicate_layout_view_test` | Outer spacing, slot 0 above origin, ring 2 closer |
| `syndicate_board_render_test` | 40 circles, line count = unique unordered edges, centers match layout |
| `syndicate_pawn_animator_test` | 400px path at 400 px/s, `busy()`, distinct same-slot positions |
| `syndicate_hud_model_test` | phasePrompt words; note TTL expiry |
| `syndicate_play_scene_test` | 4-bot match reaches GameOver + PushScene; human roll ignored on bot turn at dt 0 |
| `syndicate_scenes_test` | Menu cancel quits; Play pushes setup; setup Confirm replaces with PlayScene |
| `syndicate_feedback_test` | `Rolled` / `FeePaid` each play ≥1 sound via `AudioSpy` |
