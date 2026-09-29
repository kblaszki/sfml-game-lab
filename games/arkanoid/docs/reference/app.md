---
title: Arkanoid app
diataxis: reference
audience: [ai, human]
related_code:
  - games/arkanoid/app/include/arkanoid/app/Theme.hpp
  - games/arkanoid/app/include/arkanoid/app/Assets.hpp
  - games/arkanoid/app/include/arkanoid/app/Bindings.hpp
  - games/arkanoid/app/include/arkanoid/app/HudModel.hpp
  - games/arkanoid/app/include/arkanoid/app/SceneRender.hpp
  - games/arkanoid/app/include/arkanoid/app/Sounds.hpp
  - games/arkanoid/app/include/arkanoid/app/Feedback.hpp
  - games/arkanoid/app/include/arkanoid/app/Scenes.hpp
  - games/arkanoid/app/include/arkanoid/app/MainMenuScene.hpp
  - games/arkanoid/app/include/arkanoid/app/GameplayScene.hpp
  - games/arkanoid/app/include/arkanoid/app/PauseScene.hpp
  - games/arkanoid/app/include/arkanoid/app/ResultScene.hpp
  - games/arkanoid/app/src/Assets.cpp
  - games/arkanoid/app/src/SceneRender.cpp
  - games/arkanoid/app/src/Bindings.cpp
  - games/arkanoid/app/src/HudModel.cpp
  - games/arkanoid/app/src/Sounds.cpp
  - games/arkanoid/app/src/Feedback.cpp
  - games/arkanoid/app/src/Scenes.cpp
  - games/arkanoid/app/src/MainMenuScene.cpp
  - games/arkanoid/app/src/GameplayScene.cpp
  - games/arkanoid/app/src/PauseScene.cpp
  - games/arkanoid/app/src/ResultScene.cpp
  - games/arkanoid/main.cpp
  - tests/arkanoid/app/AssetsTest.cpp
  - tests/arkanoid/app/HudModelTest.cpp
  - tests/arkanoid/app/SceneRenderTest.cpp
  - tests/arkanoid/app/ScenesTest.cpp
  - tests/arkanoid/app/ResultSceneTest.cpp
  - tests/arkanoid/app/SoundsTest.cpp
  - tests/arkanoid/app/FeedbackTest.cpp
related_docs:
  - sim.md
  - ../../../../docs/reference/engine-core.md
  - ../../../../docs/reference/engine-input.md
  - ../../../../docs/reference/engine-resources.md
  - ../../../../docs/reference/engine-save.md
  - ../../../../docs/reference/engine-scene.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/pause-overlay.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [arkanoid, app, Theme, Assets, Bindings, HudModel, textureSpecs, makeHud, Sounds, Feedback, Scenes, MainMenuScene, GameplayScene, PauseScene, ResultScene, HighScoreTable, MenuModel, ScoreStore, blip]
last_reviewed: 2026-09-29
---

# Arkanoid app

Presentation helpers in `namespace sgl::arkanoid` for the playable shell. Static library target `arkanoid_app` (links `arkanoid_sim`, `sgl_loop`, `sgl_resources`, `sgl_audio`, `sgl_fx`, `sgl_save`, `sgl_ui`). No SFML includes — pixel art is `sgl::Image`, input uses `sgl::ActionMap`. Include as `<arkanoid/app/X.hpp>`. Executable target `arkanoid` (`games/arkanoid/main.cpp`) links `arkanoid_app` and `sgl_sfml`, defines `ASSET_DIR` to the repo `assets/` root.

## Assets

`Assets.hpp` / `Assets.cpp` expose procedural textures and the UI font path:

| Symbol | Role |
|--------|------|
| `AssetKeys` | `string_view` keys: `background`, `brick`, `paddle`, `ball`, `capsuleWide` / `Multi` / `Slow` / `Extra` |
| `uiFontRelativePath` | `"fonts/upheavtt.ttf"` (relative to the assets root) |
| `TextureSpec` | `{key, build}` where `build` returns an `sgl::Image` |
| `textureSpecs()` | Span of all eight specs |

Image sizes match the breakout art: background **1280×720** (dark blue with horizontal stripes), brick **110×28**, paddle **120×20**, ball **16×16** (circle with transparent outside), capsules **32×16** (ellipse, transparent outside). Capsule tints: gold / cyan / blue / green with a white highlight band.

`Theme.hpp` holds inline constexpr colors (title gold, HUD text, banner fill, menu button fills/labels) and HUD anchor positions: score `(32,16)`, power `(560,16)`, lives `(1040,16)`. Design size is `designSize` from `sgl::arkanoid::designWidth` / `designHeight`.

## Bindings

`makeActions()` builds `Actions` with explicit ids **1–6** (`confirm`, `cancel`, `pause`, `paddle` axis id **4**, `up` **5**, `down` **6`). `defaultBindings` maps Enter→confirm, Escape→pause, Backspace→cancel, Up/Down→menu focus, and paddle axis Left/A (negative) with Right/D (positive). `menuActions(actions)` returns `sgl::MenuActions` for the shared menu kit.

## HUD

`makeHud(const State&)` fills `HudModel`:

| Field | Content |
|-------|---------|
| `score` / `lives` | `"Score N"` / `"Lives N"` |
| `effect` | Space-separated list of every active timed effect with whole seconds remaining, e.g. `"Wide 4 Slow 8"` (floor of `remaining`); empty when none |
| `banner` | `"You win"` only when `cleared && stage == Stage3`; `"You lose"` when `over`; otherwise nullopt (including cleared Stage1/2) |
| `hint` | `"Enter - retry   Backspace - menu"` when `banner` is set |

`GameplayScene` draws only `score`, `lives`, and `effect` on `Layer::Hud`. Win/lose outcome UI is `ResultScene`, not the HUD banner path. `banner` / `hint` remain filled by `makeHud` but are unused by the draw path.

## Sounds

`soundSpecs()` returns a span of `{key, Pcm}` for **paddle**, **brick**, **wall**, **launch**, **powerUp**, **ballLost**, **lifeLost**, **stageClear**, and **gameOver**. Each buffer is built with `sgl::blip` (gain `0.35`). `SoundIds` holds the uploaded `SoundId` handles. `main.cpp` uploads every spec through `SfmlAudio::upload` (`platform.audio()` is already `SfmlAudio&`) and stores the ids plus `AudioI&` in `AppServices`.

## Feedback

`Feedback(AudioI&, SoundIds, Pcg32)` maps every `SimEvent` to exactly one sound (`BrickDestroyed` → brick, `LifeLost` → lifeLost, `StageCleared` → stageClear, `GameOver` → gameOver, `PowerUpCaught` → powerUp, `PaddleHit` → paddle, `WallHit` → wall, `BallLaunched` → launch, `BallLost` → ballLost). `BrickDestroyed` also emits **12** particles in that brick’s `tint` (`ParticleSystem` capacity **1024**). `update(Seconds)` and `render(RenderQueue&)` draw on `Layer::Overlay`. `GameplayScene` owns `Feedback` and feeds it `step` events each tick.

## Render

`renderState` pushes draw commands from a `State` snapshot. Background is `Layer::Background`. Alive bricks are `Layer::World` with depth `box.pos.y`. Paddle, balls, and capsules are `Layer::Actors` with depth equal to their y. Dead bricks are skipped. A wide paddle uses `scale.x = width / 120`. Capsule texture index follows `PowerUpKind`.

## Scenes

`AppServices` bundles `{const Actions& actions; const TextureIds& textures; sgl::FontId font; sgl::AudioI& audio; SoundIds sounds; sgl::HighScoreTable& highScores; std::filesystem::path scoresPath}` for scene factories. References are owned by `main()` for the whole `App::run()`; scenes must not outlive that. High-score capacity is `kHighScoreCapacity` (**5**). Factories return `sgl::SceneFactory`:

| Factory | Scene | Traits |
|---------|-------|--------|
| `mainMenu(services)` | `MainMenuScene` | opaque, blocksUpdate, not pausable |
| `gameplay(services, StageId)` | `GameplayScene` | opaque, blocksUpdate, pausable while playing (`!cleared && !over`) |
| `pause(services)` | `PauseScene` | not opaque, blocksUpdate, not pausable |
| `result(services, Outcome, score, rank, saveFailed)` | `ResultScene` | not opaque, blocksUpdate, not pausable |

`MainMenuScene` and `ResultScene` drive `sgl::MenuModel` + `sgl::renderMenu` for their buttons (Start/Quit and Retry/Menu). Titles and score copy stay custom draws. Confirm activates the focused row; cancel quits (menu) or returns to the main menu (result). Pointer activation requires press and release inside the same button.

`GameplayScene` owns `sim::State` from `makeState` and `Feedback`. Each update builds `SimInput` from paddle axis + confirm, calls `step`, runs feedback, then handles stage/outcome. Stage1/2 `StageCleared` advances via `makeState(next, score, lives)`. On `GameOver` or Stage3 `StageCleared`: `highScores.insert(score)`, `ScoreStore::save(scoresPath, ...)`, then `PushScene{result(...)}`. Save failure sets `saveFailed` on the result overlay; play continues. While playing, pause → `RequestPause`. Exposes `state()` (const and mutable) and `hud()`.

`ResultScene` is the win/lose overlay (`Outcome::Won` / `Lost`). Shows title, score, optional `"New record"` when `rank` has a value, and `"Could not save high score"` when `saveFailed`. Retry (confirm on focused Retry, or Retry click) → `PopScene` then `ReplaceScene{gameplay(..., Stage1)}` with default score/lives. Menu (cancel or Menu activate) → `PopScene` then `ReplaceScene{mainMenu}`.

`PauseScene` keeps the `PauseScene` type and uses `MenuModel` + `renderMenu` (Resume / Menu). Pause or cancel → `PopScene`; confirm → `PopScene` then `ReplaceScene{mainMenu}` (menu-kit confirm is unbound so Enter keeps the prior quit-to-menu path; pointer still activates the focused row).

`main.cpp` loads every `textureSpecs()` entry via a key→slot table and the UI font through `SfmlPlatform::assets()`, uploads `soundSpecs()`, resolves `userDataDir("sfml-game-lab") / "arkanoid.scores"` (temp-directory fallback when `userDataDir` fails), loads the table with `ScoreStore::load`, builds `AppServices`, constructs `SceneStack` with the pause factory, pushes the main menu, and runs `sgl::App`.

## Tests

Headless suites under `tests/arkanoid/app/`: `AssetsTest`, `HudModelTest`, `SceneRenderTest`, `ScenesTest`, `ResultSceneTest`, `SoundsTest`, `FeedbackTest` (`AudioSpy`).
