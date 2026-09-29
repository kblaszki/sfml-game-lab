---
title: Source layout reference
diataxis: reference
audience: [ai, human]
related_code:
  - CMakeLists.txt
  - cmake/FetchSFML.cmake
  - cmake/FetchGTest.cmake
  - cmake/AddUnitTest.cmake
  - engine/CMakeLists.txt
  - engine/core/CMakeLists.txt
  - engine/core/include/sgl/core/Features.hpp
  - engine/core/include/sgl/core/Overloaded.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Effects.hpp
  - engine/input/CMakeLists.txt
  - engine/input/include/sgl/input/InputEvent.hpp
  - engine/scene/CMakeLists.txt
  - engine/scene/include/sgl/scene/SceneStack.hpp
  - engine/loop/CMakeLists.txt
  - engine/loop/include/sgl/loop/App.hpp
  - engine/render/CMakeLists.txt
  - engine/collision/CMakeLists.txt
  - engine/resources/CMakeLists.txt
  - engine/resources/include/sgl/resources/ResourceCache.hpp
  - engine/resources/include/sgl/resources/Atlas.hpp
  - engine/resources/src/Atlas.cpp
  - engine/sfml/CMakeLists.txt
  - engine/sfml/include/sgl/sfml/SfmlPlatform.hpp
  - engine/audio/CMakeLists.txt
  - engine/audio/include/sgl/audio/ScaledAudio.hpp
  - engine/ui/CMakeLists.txt
  - engine/fx/CMakeLists.txt
  - engine/anim/CMakeLists.txt
  - engine/anim/include/sgl/anim/Easing.hpp
  - engine/save/CMakeLists.txt
  - engine/save/include/sgl/save/Settings.hpp
  - engine/save/src/Settings.cpp
  - games/CMakeLists.txt
  - games/tetris/CMakeLists.txt
  - games/tetris/sim/CMakeLists.txt
  - games/tetris/app/CMakeLists.txt
  - games/syndicate/CMakeLists.txt
  - games/syndicate/sim/CMakeLists.txt
  - games/syndicate/app/CMakeLists.txt
  - cmake/CheckBoundaries.cmake
  - games/arkanoid/CMakeLists.txt
  - games/arkanoid/main.cpp
  - games/arkanoid/sim/CMakeLists.txt
  - games/arkanoid/app/CMakeLists.txt
  - games/arkanoid/app/include/arkanoid/app/Scenes.hpp
  - tests/CMakeLists.txt
  - tests/engine/core/CMakeLists.txt
  - tests/engine/input/CMakeLists.txt
  - tests/engine/scene/CMakeLists.txt
  - tests/engine/loop/CMakeLists.txt
  - tests/engine/render/CMakeLists.txt
  - tests/engine/collision/CMakeLists.txt
  - tests/engine/resources/CMakeLists.txt
  - tests/engine/sfml/CMakeLists.txt
  - tests/arkanoid/sim/CMakeLists.txt
  - tests/arkanoid/app/CMakeLists.txt
  - tests/engine/audio/CMakeLists.txt
  - tests/engine/audio/ScaledAudioTest.cpp
  - tests/engine/ui/CMakeLists.txt
  - tests/engine/fx/CMakeLists.txt
  - tests/engine/anim/CMakeLists.txt
  - tests/engine/save/CMakeLists.txt
  - tests/engine/save/SettingsTest.cpp
  - tests/tetris/sim/CMakeLists.txt
  - tests/tetris/app/CMakeLists.txt
  - tests/syndicate/sim/CMakeLists.txt
  - tests/syndicate/app/CMakeLists.txt
  - tests/mocks/ClockMock.hpp
  - tests/mocks/PlatformMock.hpp
  - tests/mocks/RendererMock.hpp
  - tests/mocks/AudioSpy.hpp
  - .github/workflows/ci.yml
related_docs:
  - engine-core.md
  - engine-input.md
  - engine-scene.md
  - engine-loop.md
  - engine-render.md
  - engine-collision.md
  - engine-resources.md
  - engine-audio.md
  - engine-ui.md
  - engine-fx.md
  - engine-anim.md
  - engine-save.md
  - engine-sfml.md
  - arkanoid-sim.md
  - arkanoid-app.md
  - tetris-sim.md
  - tetris-app.md
  - syndicate-sim.md
  - syndicate-app.md
  - syndicate-assets.md
  - input-and-events.md
  - pause-overlay.md
  - ../how-to/build-and-test.md
  - ../how-to/add-a-game.md
  - ../explanation/architecture.md
  - ../explanation/syndicate-design.md
  - ../explanation/syndicate-ui.md
  - ../../mvp/10-engine-progress.md
keywords: [layout, directories, sgl, arkanoid, tetris, syndicate, targets, cmake, SceneStack, App, SfmlPlatform]
last_reviewed: 2026-09-29
---

# Source layout reference

Running tree after cutover: `engine/` (`namespace sgl`) and `games/arkanoid/` (`namespace sgl::arkanoid`, executable `arkanoid`). Contract: [architecture.md](../explanation/architecture.md).

## Directories

| Path | Responsibility |
|------|----------------|
| `engine/core/` | Header-only value types (`Vec2`, `Rect`, `Handle`, `Image`, `Pcg32`, `Overloaded`); target `sgl_core` |
| `engine/input/` | `InputEvent`, `ActionMap`, `InputState`; target `sgl_input` |
| `engine/scene/` | `SceneI`, `SceneStack`, `SceneRequest`, `SceneTraits`; target `sgl_scene` |
| `engine/loop/` | `FixedStepLoop`, `App`, `PlatformI`, `ClockI`; target `sgl_loop` |
| `engine/render/` | `RenderQueue`, `DrawCommand` (includes `MeshCmd`), `Shapes`, `Projection`, `RendererI`; target `sgl_render` |
| `engine/collision/` | AABB / circle / sweep helpers; target `sgl_collision` |
| `engine/resources/` | `ResourceCache`, `ResourceError`, `Atlas` / `parseAtlas`; target `sgl_resources` |
| `engine/sfml/` | SFML adapters (`SfmlPlatform`, `SfmlAudio`, `EventTranslate`, …); only engine target that links SFML |
| `engine/audio/` | `AudioI`, `ScaledAudio`, PCM `tone` / `blip`, `NullAudio`; target `sgl_audio` |
| `engine/ui/` | Menu kit (`MenuModel`, `renderMenu`, `PauseOverlay`, `SettingsScene`), `TextLayout`, `TooltipSystem`, `HoverAnim`; target `sgl_ui` |
| `engine/fx/` | Fixed-capacity particles; target `sgl_fx` |
| `engine/anim/` | `Easing`, `Tween`, Catmull-Rom `Spline`, `ArcLengthTable`; target `sgl_anim` (links `sgl_core` only) |
| `engine/save/` | High-score table, settings (`Settings` / `SettingsStore`), text format, atomic file write, `ScoreStore`; target `sgl_save` |
| `games/arkanoid/sim/` | Headless breakout (`State`, `step`, levels, `Effects` as timed variants); `arkanoid_sim` |
| `games/arkanoid/app/` | Scenes, HUD, result overlay, sounds, particles, high scores; `arkanoid_app` |
| `games/arkanoid/main.cpp` | Wires `SfmlPlatform`, assets, `SceneStack`, `sgl::App`; executable `arkanoid` |
| `games/tetris/sim/` | Headless Tetris rules (`Bag`, scoring, `TetrisGame`, grid, SRS); `tetris_sim` links `sgl_core` only |
| `games/tetris/app/` | Scenes, HUD, board render, sounds, particles, high scores; `tetris_app` |
| `games/syndicate/sim/` | Headless Syndicate (`BoardGraph`, `Market`/`Economy`, `Cards`, `Auction`/`Trade`, `Match` turn machine); `syndicate_sim` links `sgl_core` only |
| `games/syndicate/app/` | Scenes (incl. `PauseMenuScene`, `HelpScene`, `ControlMap`), board UI, Describe/ActionBar/HandView, MarketHistory/TrendChart/PathHighlighter, DiceAnimator/Presentation/PawnAnimator, SpriteBank, sounds, feedback, high scores; `syndicate_app` (no SFML) |
| `games/syndicate/main.cpp` | Wires `SfmlPlatform`, settings/`ScaledAudio`/`setDisplay`, font/sounds/optional atlas, `SceneStack`, `sgl::App`; executable `syndicate` |
| `assets/fonts/` | UI TTF (`ASSET_DIR` → `assets/`) |
| `tests/engine/` | Per-module engine suites (Debug only) |
| `tests/arkanoid/` | Sim and app suites (Debug only) |
| `tests/tetris/` | Sim and app suites (Debug only) |
| `tests/syndicate/` | Sim and app suites (Debug only) |
| `tests/mocks/` | `ClockMock`, `PlatformMock`, `RendererMock`, `AudioSpy` (engine ports) |

Include layout: `<sgl/<module>/X.hpp>`, `<arkanoid/<sim|app>/X.hpp>`, `<tetris/<sim|app>/X.hpp>`, `<syndicate/<sim|app>/X.hpp>`. Only `engine/sfml` and `games/<game>/main.cpp` may include `<SFML/...>`. The engine never includes a game header.

## Build targets

| Target | Kind | Notes |
|--------|------|-------|
| `sgl_core` | INTERFACE | Headers only |
| `sgl_input` | STATIC | → `sgl_core` |
| `sgl_scene` | STATIC | → `sgl_input` |
| `sgl_loop` | STATIC | → `sgl_scene`, `sgl_render` |
| `sgl_render` | STATIC | → `sgl_core` |
| `sgl_collision` | STATIC | → `sgl_core` |
| `sgl_resources` | STATIC | → `sgl_core`; `ResourceCache` headers + `Atlas` / `parseAtlas` |
| `sgl_audio` | STATIC | → `sgl_core`; no SFML |
| `sgl_ui` | STATIC | → `sgl_input`, `sgl_render`, `sgl_scene`, `sgl_anim`, `sgl_save`; no SFML |
| `sgl_fx` | STATIC | → `sgl_core`, `sgl_render` |
| `sgl_anim` | STATIC | → `sgl_core`; easing, tween, spline; no SFML |
| `sgl_save` | STATIC | → `sgl_core`; no SFML |
| `sgl_sfml` | STATIC | → `sgl_loop`, `sgl_resources`, `sgl_audio`, `sgl_save`; links SFML Graphics/Window/System/Audio |
| `arkanoid_sim` | STATIC | → `sgl_collision`; no SFML |
| `arkanoid_app` | STATIC | → `arkanoid_sim`, `sgl_loop`, `sgl_resources`, `sgl_audio`, `sgl_fx`, `sgl_save`; no SFML includes |
| `arkanoid` | executable | `games/arkanoid/main.cpp` → `arkanoid_app`, `sgl_sfml`; `ASSET_DIR` → `assets/` |
| `tetris_sim` | STATIC | → `sgl_core`; bag, scoring, `TetrisGame`, grid, SRS; no SFML |
| `tetris_app` | STATIC | → `tetris_sim`, `sgl_loop`, `sgl_resources`, `sgl_audio`, `sgl_fx`, `sgl_save`; no SFML includes |
| `tetris` | executable | `games/tetris/main.cpp` → `tetris_app`, `sgl_sfml`; `ASSET_DIR` → `assets/` |
| `syndicate_sim` | STATIC | → `sgl_core`; Money, BoardGraph, economy/cards/auction, Match turn machine; no SFML |
| `syndicate_app` | STATIC | → `syndicate_sim`, `sgl_render`, `sgl_scene`, `sgl_core`, `sgl_input`, `sgl_ui`, `sgl_anim`, `sgl_audio`, `sgl_fx`, `sgl_save`, `sgl_resources`; scenes + board UI + HandView + SpriteBank; no SFML |
| `syndicate` | executable | `games/syndicate/main.cpp` → `syndicate_app`, `sgl_sfml`; `ASSET_DIR` → `assets/` |
| `format` | custom | clang-format in place (Debug configure) |
| `tidy` | custom | `run-clang-tidy` on `engine/` and `games/` (Debug configure) |

Configure presets: `debug`, `release`, `asan` (`SGL_ENABLE_ASAN` on GCC/Clang), `coverage` (`SGL_ENABLE_COVERAGE` on GCC). Details: [build-and-test](../how-to/build-and-test.md).

## Unit test suites (Debug)

Registered with `add_unit_test` under `tests/engine/`, `tests/arkanoid/`, `tests/tetris/`, and `tests/syndicate/`:

| Suite | Leaf |
|-------|------|
| `vec_test`, `rect_test`, `handle_test`, `image_test`, `random_test` | `tests/engine/core/` |
| `input_state_test` | `tests/engine/input/` |
| `scene_stack_test` | `tests/engine/scene/` |
| `fixed_step_loop_test`, `app_test` | `tests/engine/loop/` |
| `render_queue_test`, `projection_test`, `shapes_test` | `tests/engine/render/` |
| `collision_test` | `tests/engine/collision/` |
| `resource_cache_test`, `atlas_test` | `tests/engine/resources/` |
| `particle_system_test` | `tests/engine/fx/` |
| `anim_test` | `tests/engine/anim/` |
| `tone_test`, `blip_test`, `scaled_audio_test` | `tests/engine/audio/` |
| `menu_model_test`, `pause_overlay_test`, `settings_scene_test`, `text_layout_test`, `tooltip_test`, `hover_anim_test` | `tests/engine/ui/` |
| `high_score_table_test`, `save_format_test`, `save_file_test`, `score_store_test`, `settings_test` | `tests/engine/save/` |
| `event_translate_test`, `sfml_draw_test` | `tests/engine/sfml/` (`event_translate_test` links `SFML::Window`; neither opens a window) |
| `arkanoid_sim_test`, `arkanoid_physics_test`, `arkanoid_sim_property_test`, `arkanoid_powerup_test` | `tests/arkanoid/sim/` |
| `tetris_grid_test`, `tetris_srs_test`, `tetris_bag_test`, `tetris_scoring_test`, `tetris_game_test`, `tetris_property_test` | `tests/tetris/sim/` |
| `syndicate_money_test`, `syndicate_board_graph_test`, `syndicate_board_layouts_test`, `syndicate_pathing_test`, `syndicate_market_test`, `syndicate_economy_test`, `syndicate_cards_test`, `syndicate_auction_test`, `syndicate_trade_test`, `syndicate_match_test`, `syndicate_match_property_test` | `tests/syndicate/sim/` |
| `syndicate_layout_view_test`, `syndicate_board_render_test`, `syndicate_hand_view_test`, `syndicate_pawn_animator_test`, `syndicate_dice_animator_test`, `syndicate_presentation_test`, `syndicate_market_history_test`, `syndicate_trend_chart_test`, `syndicate_path_highlighter_test`, `syndicate_hud_model_test`, `syndicate_describe_test`, `syndicate_action_bar_test`, `syndicate_play_scene_test`, `syndicate_scenes_test`, `syndicate_control_map_test`, `syndicate_help_scene_test`, `syndicate_feedback_test`, `syndicate_sprite_bank_test` | `tests/syndicate/app/` |
| `arkanoid_assets_test`, `arkanoid_hud_test`, `arkanoid_scene_render_test`, `arkanoid_scenes_test`, `arkanoid_result_scene_test`, `arkanoid_sounds_test`, `arkanoid_feedback_test` | `tests/arkanoid/app/` |
| `tetris_input_mapping_test`, `tetris_board_render_test`, `tetris_hud_test`, `tetris_scenes_test`, `tetris_sounds_test`, `tetris_feedback_test` | `tests/tetris/app/` |

All suites are headless except `event_translate_test`, which links SFML Window headers only and still opens no window.

## Adding a source file

New `.cpp` files belong in that module’s own `CMakeLists.txt` (`engine/<module>/CMakeLists.txt` or `games/<game>/<sim|app>/CMakeLists.txt`). Header-only files do not need listing. Parallel agents must not edit a shared sources list outside their module.
