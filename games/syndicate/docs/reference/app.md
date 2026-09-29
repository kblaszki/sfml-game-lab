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
  - games/syndicate/app/include/syndicate/app/DiceAnimator.hpp
  - games/syndicate/app/include/syndicate/app/Presentation.hpp
  - games/syndicate/app/include/syndicate/app/Bindings.hpp
  - games/syndicate/app/include/syndicate/app/ControlMap.hpp
  - games/syndicate/app/include/syndicate/app/Describe.hpp
  - games/syndicate/app/include/syndicate/app/ActivityLog.hpp
  - games/syndicate/app/include/syndicate/app/ActionBar.hpp
  - games/syndicate/app/include/syndicate/app/HandView.hpp
  - games/syndicate/app/include/syndicate/app/MarketHistory.hpp
  - games/syndicate/app/include/syndicate/app/TrendChart.hpp
  - games/syndicate/app/include/syndicate/app/PathHighlighter.hpp
  - games/syndicate/app/include/syndicate/app/HudModel.hpp
  - games/syndicate/app/include/syndicate/app/PlayScene.hpp
  - games/syndicate/app/include/syndicate/app/Scenes.hpp
  - games/syndicate/app/include/syndicate/app/Sounds.hpp
  - games/syndicate/app/include/syndicate/app/Feedback.hpp
  - games/syndicate/app/include/syndicate/app/SpriteBank.hpp
  - games/syndicate/app/include/syndicate/app/MainMenuScene.hpp
  - games/syndicate/app/include/syndicate/app/SetupScene.hpp
  - games/syndicate/app/include/syndicate/app/GameOverScene.hpp
  - games/syndicate/app/include/syndicate/app/HelpScene.hpp
  - games/syndicate/app/include/syndicate/app/PauseMenuScene.hpp
  - games/syndicate/app/src/Theme.cpp
  - games/syndicate/app/src/LayoutView.cpp
  - games/syndicate/app/src/BoardRender.cpp
  - games/syndicate/app/src/PawnAnimator.cpp
  - games/syndicate/app/src/DiceAnimator.cpp
  - games/syndicate/app/src/Presentation.cpp
  - games/syndicate/app/src/Bindings.cpp
  - games/syndicate/app/src/ControlMap.cpp
  - games/syndicate/app/src/Describe.cpp
  - games/syndicate/app/src/ActivityLog.cpp
  - games/syndicate/app/src/ActionBar.cpp
  - games/syndicate/app/src/HandView.cpp
  - games/syndicate/app/src/MarketHistory.cpp
  - games/syndicate/app/src/TrendChart.cpp
  - games/syndicate/app/src/PathHighlighter.cpp
  - games/syndicate/app/src/HudModel.cpp
  - games/syndicate/app/src/PlayScene.cpp
  - games/syndicate/app/src/Sounds.cpp
  - games/syndicate/app/src/Feedback.cpp
  - games/syndicate/app/src/SpriteBank.cpp
  - games/syndicate/app/src/Scenes.cpp
  - games/syndicate/app/src/MainMenuScene.cpp
  - games/syndicate/app/src/SetupScene.cpp
  - games/syndicate/app/src/GameOverScene.cpp
  - games/syndicate/app/src/HelpScene.cpp
  - games/syndicate/app/src/PauseMenuScene.cpp
  - tests/syndicate/app/CMakeLists.txt
  - tests/syndicate/app/LayoutViewTest.cpp
  - tests/syndicate/app/BoardRenderTest.cpp
  - tests/syndicate/app/HandViewTest.cpp
  - tests/syndicate/app/PawnAnimatorTest.cpp
  - tests/syndicate/app/DiceAnimatorTest.cpp
  - tests/syndicate/app/PresentationTest.cpp
  - tests/syndicate/app/MarketHistoryTest.cpp
  - tests/syndicate/app/TrendChartTest.cpp
  - tests/syndicate/app/PathHighlighterTest.cpp
  - tests/syndicate/app/HudModelTest.cpp
  - tests/syndicate/app/DescribeTest.cpp
  - tests/syndicate/app/ActivityLogTest.cpp
  - tests/syndicate/app/ActionBarTest.cpp
  - tests/syndicate/app/PlaySceneTest.cpp
  - tests/syndicate/app/ScenesTest.cpp
  - tests/syndicate/app/ControlMapTest.cpp
  - tests/syndicate/app/HelpSceneTest.cpp
  - tests/syndicate/app/FeedbackTest.cpp
  - tests/syndicate/app/SpriteBankTest.cpp
related_docs:
  - sim.md
  - assets.md
  - dice-animation.md
  - ../explanation/design.md
  - ../explanation/ui.md
  - ../explanation/board-graph.md
  - ../how-to/navigate-ui-settings-help.md
  - ../../../../docs/reference/engine-render.md
  - ../../../../docs/reference/engine-ui.md
  - ../../../../docs/reference/engine-save.md
  - ../../../../docs/reference/pause-overlay.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [syndicate, app, Theme, LayoutView, renderBoard, HandView, MarketHistory, TrendChart, PathHighlighter, PawnAnimator, DiceAnimator, Presentation, Bindings, ControlMap, HelpScene, PauseMenuScene, Settings, Describe, ActivityLog, ActionBar, HudModel, PlayScene, tooltip, Scenes, Feedback, Sounds, SpriteBank, MainMenuScene, SetupScene, GameOverScene, syndicate executable, 1280x720]
last_reviewed: 2026-09-29
---

# Syndicate app

Presentation layer and playable executable for Syndicate in `namespace sgl::syndicate`. Static library `syndicate_app` links `PUBLIC` `syndicate_sim`, `sgl_render`, `sgl_scene`, `sgl_core`, `sgl_input`, `sgl_ui`, `sgl_anim`, `sgl_audio`, `sgl_fx`, `sgl_save`, and `sgl_resources`. No SFML includes in the library — drawing uses `sgl::RenderQueue` and `CircleCmd` / `LineCmd` / `RectCmd` / `TextCmd` / optional `SpriteCmd`. Include as `<syndicate/app/X.hpp>`. Design size **1280×720**. Executable target `syndicate` (`games/syndicate/main.cpp`) links `syndicate_app` and `sgl_sfml`.

## SyndicateServices

`SyndicateServices` holds references owned by `main()` for the whole `App::run()`: `actions`, `font`, `seed`, `audio`, `sounds`, `scoresPath`, `highScores`, `sprites` (`SpriteBank`, default empty), `bodyFont` (body / tooltip TTF; id 0 means fall back to `font`), `settings` (`sgl::Settings`), and `onSettingsChanged` (optional; empty is a no-op). Scene factories in `Scenes.hpp` capture this struct by value (shallow copies of references). Font path constants: `uiFontRelativePath`, `bodyFontRelativePath`.

| Factory | Scene |
|---------|--------|
| `mainMenu` | `MainMenuScene` (Play / Settings / Help / Quit) |
| `setup` | `SetupScene` (2–4 seats, Human/Bot, Confirm) |
| `play` | `PlayScene` with `MatchConfig` + seats |
| `pause` | `PauseMenuScene` (Resume / Settings / Help / Quit; not `sgl::PauseOverlay`) |
| `makeGameOver` | `GameOverScene` with ranking + high-score save |

## ControlMap / Help / Pause

`controlMap()` returns phase / input / effect rows (Roll, Path, Purchase, Auction, Trade, Manage, Global). `hintLine(phase)` returns a non-empty hint ≤ 90 characters for the current `Phase`.

`HelpScene(phaseIndex, MenuActions, FontId)` shows tabs for unique phase names in control-map order; starts on `phaseIndex` clamped to the tab count; Left/Right or tab clicks change tabs; cancel/Back → `PopScene`.

`PauseMenuScene`: Resume or cancel → `PopScene`; Settings → `PushScene` `sgl::SettingsScene` using `services.settings` / `onSettingsChanged`; Help → `HelpScene` at tab 0; Quit → `QuitApp`. Traits match pause overlay (`opaque` false, `blocksUpdate` true).

## SpriteBank

`SpriteBank` maps atlas region names to `SpriteCmd` (texture + source rect + position/scale/tint). Default / empty manifest: `sprite(...)` returns `nullopt` so callers keep procedural fallbacks. Optional load in `main.cpp` when both `assets/syndicate/atlas.png` and `atlas.txt` exist.

## Sounds / Feedback

`soundSpecs()` returns PCM from `sgl::blip` (`roll`, `fee`, `auction`, `card`, `market`). `main.cpp` uploads via `SfmlAudio`; the app library only holds `SoundIds` / specs. `Feedback(AudioI&, SoundIds)` plays on `Rolled`, `FeePaid`, `AuctionDone`, `CardPlayed`, `MarketShifted`; other `GameEvent` alternatives are silent.

## Menu / setup / game over

- **MainMenuScene:** `MenuModel` + `renderMenu`. Play → `PushScene{setup}`; Settings → `SettingsScene`; Help → `HelpScene` (tab 0); Quit / cancel / Escape → `QuitApp`.
- **SetupScene:** cycles seat count 2→3→4, toggles Human/Bot per seat, Confirm → `ReplaceScene{play}` with `MatchConfig{seed, seats, startCash 1500, maxRounds 15, targetNetWorth 6000}`.
- **GameOverScene:** ranking lines (`P1`… by player id, net worth). Menu → pop + replace main menu; Quit → `QuitApp`. Inserts the winner's net worth into `HighScoreTable` and `ScoreStore::save`; save failure still shows the scene.

## Theme / layout / board / HUD

`Theme` / `defaultTheme()`, `LayoutView`, shared layout rects (`kBoardOrigin`/`kBoardRadius`, `kLeftPanel`, `kRightChart`, `kRightLog`, `kActionBar`, `kBoardBounds`), `renderBoard`, `PawnAnimator`, `DiceAnimator`, `Presentation`, `HudModel`, `ActivityLog`, `HandView`, `MarketHistory`, `TrendChart`, `PathHighlighter`. Bindings: **Space** roll, **Enter** end turn / menu confirm, **C** bid, **P** pass, **Escape** pause, **H** help, **Left/Right** settings nudges, **Num1–Num9** action-bar slots, **Up/Down** menu, **Backspace** cancel.

`renderBoard` draws 40 node circles, unique undirected edges, and a size-11 short label per node (`AI`/`CLN`/`BIO`/`CYB`/`LOG`/`FIN`, `PAY`/`EXC`/`HUB`/`AUD`/`GRA`) just below the node. Optional trailing `FontId` + `SpriteBank*` + `HighlightModel*` (defaults) emit centered atlas sprites when present (`sector/*`, `tile/*`); when `highlights` is non-null, destination nodes use a thicker outline.

`HudModel` draws one rounded player panel per seat (`playerPanelRect` inside `kLeftPanel`) with `P{n} {formatMoney(cash)}`; the acting seat gets a brighter outline. Phase prompt and note TTL are unchanged. Market bars are replaced by `renderTrendChart` in `kRightChart` (via `marketChartRect()`): six stacked sparklines from an internal `MarketHistory` (ring of 32 samples per sector). `recordMarket` appends the current `Market` index; `chartHotspots()` returns one hotspot per sector row.

`ActivityLog` (cap 64) stores turn lines from drained `GameEvent`s (`describeEvent`). `visible(n)` returns the newest `n` entries oldest-first. Render fills `kRightLog` (rounded panel, 4px player stripe, wrapped text, newest at the bottom). Round changes push a `— Round N —` separator entry.

`PathHighlighter` builds legal-move destinations and junction arrows from `MoveOption` paths (arrow tip at 60% along a diverging edge). `update` pulses radius in `[0.88, 1.12]` and sets `hovered` when the pointer is within `1.5 * nodeRadius` of a destination. `render` draws `radialGlow`, `arrowHead`, and a thickness-5 path polyline for the hovered option.

## Dice / pawn presentation

Match rolls are **d4+d4** (faces 1..4, total 2..8). `DiceAnimator` shows two procedural dice (rounded rect + pip circles) with tumble intervals `0.04s * 1.35^k`, then an `easeOutBack` scale pop. It owns a cosmetic `sgl::Pcg32` seeded with `seed ^ 0xD1CE` — **not** `GameState::rng`, so match determinism is unchanged.

`PawnAnimator` moves along a Catmull-Rom sample (`catmullRom(points, 8)` + `ArcLengthTable`) with distance `length * easeInOutCubic(elapsed/T)`, `T = min(0.25s * hops, 1.6s)`. `Presentation` sequences dice then path (queues path while dice busy); `fastForward` (click or roll key) finishes both. Human/bot commands are gated on `!presentation.busy()` — animation never blocks the update loop; `busy` only gates input.

## HandView

`HandView` mirrors the acting player's `Hand` (cap 5). Cards are 96×134; layout is a column at x=24 starting y=420 (8px gap) because five row strides do not fit in `kLeftPanel` width. Newly appeared cards tween from x=−120 with `easeOutBack` over 0.35s; removed cards slide toward x=400 and fade out over 0.35s. `hit` / hotspots use settled slot rects; `render` uses the animated position. Hotspot body is `describeCard`.

## Describe helpers

`Describe.hpp` formats English UI copy for money, sectors, tiles, cards, legal `Command`s, and `GameEvent`s:

- `formatMoney` — 1 sim unit = $1k (`$0`, `$150k`, `$1M`, `$1.5M`); integer math only.
- `sectorName` / `describeCard` / `describeTile` — sector labs include share price, holdings, level, and `landingFee`.
- `describeCommand` — button label + one-sentence tooltip (costs via `sharePrice`, `upgradeCost`, buy-card `$100k`, momentum `$25k`).
- `describeEvent` — non-empty English for every `GameEvent` alternative (player tags `P1`…`P4`, money via `formatMoney`).

## ActionBar

`ActionBar` rebuilds from `Match::legalCommands`, drops `ChooseDestination`, prefers commands for `selectedTile` when capping at 9, and lays out up to nine buttons in `kActionBar` (y 652..708 / x 16..1264). Slots bind to **Num1–Num9**; pointer release on a button applies that command. Enter is not treated as a bar click (legacy `EndTurn` path stays on Enter). Hotspots feed tooltips (`title` = label, `body` = tooltip).

## PlayScene

Two constructors: test-friendly `(MatchConfig, seats, FontId, SyndicateActions)` and services `(SyndicateServices, MatchConfig, seats)` which also owns `Feedback`. Owns `ActionBar`, `HandView`, `HoverAnim`, `TooltipSystem`, `Presentation` (seed from `MatchConfig::seed` or `services.seed`), `PathHighlighter`, and `ActivityLog`. Board layout uses `LayoutView{kBoardOrigin, kBoardRadius}`. Records the market into `HudModel` at startup and after each successful `apply`. On a human turn while presentation is idle: **H** (help) pushes `HelpScene` on the tab matching the current phase when presentation is not busy; otherwise rebuilds the bar, optionally selects a Manage/AwaitPurchase tile from a board click (not on a button), then applies `actionBar.update` before falling back to `commandFromInput` (Space/Enter/C/P/path click). While presentation is busy, pointer press or roll key calls `presentation.fastForward()` and does not apply a command. In `AwaitPath`, builds/updates the path highlighter and passes `&highlighter.model()` into `renderBoard`, then `highlighter.render` after the board. Each update collects hotspots for board tiles (`describeTile`), action-bar buttons, hand cards, market chart rows, and player panels, then drives `TooltipSystem` (font = `bodyFont` when set, else scene `font_`). Hover does not consume clicks. Renders board/highlights/pawns, HUD, `hintLine(phase)` at y=40 under the phase prompt, dice at layout origin, hand, action bar, activity log in `kRightLog`, then tooltip last. Pause → `RequestPause`. On `GameOver`, pushes `GameOverScene` (via `makeGameOver` when services are present). `drainEvents` feeds `Presentation::onRolled`, `Feedback`, and the activity log (`describeEvent` + round separators) after each applied command. `log()` exposes the log for tests.

## main.cpp

`SfmlPlatform` **1280×720** titled **"Syndicate"**. Loads settings from `userDataDir("syndicate") / "settings.txt"` (format header `sgl-settings 1`; defaults if the dir fails — save skipped). Applies `SfmlPlatform::setDisplay` and wraps `platform.audio()` in `ScaledAudio` (`setMaster(volume/100)`), passing the scaled audio into `SyndicateServices`. Loads `uiFontRelativePath` then `bodyFontRelativePath` under `ASSET_DIR` (same `fail()` on either), optionally loads the syndicate atlas into `SpriteBank`, uploads sounds to the raw SFML audio, resolves scores path, `bindSyndicate`, wires `onSettingsChanged` (setDisplay, setMaster, `SettingsStore::save` — save errors go to `std::cerr`, no quit), pushes main menu on a `SceneStack` with the pause factory, runs `sgl::App`. Seed from `std::chrono::steady_clock`.

## Tests

| Suite | Focus |
|-------|-------|
| `syndicate_layout_view_test` | Outer spacing, slot 0 above origin, ring 2 closer; play layout rects pairwise non-overlapping |
| `syndicate_board_render_test` | 40 circles, 40 short labels, line count = unique unordered edges, centers match layout |
| `syndicate_hand_view_test` | Appear tween: x off-screen at dt=0, at slot x after 0.35s |
| `syndicate_pawn_animator_test` | Catmull-Rom ease-in-out path, `busy()`, distinct same-slot positions |
| `syndicate_dice_animator_test` | d4+d4 tumble intervals non-decreasing, settle on roll 7, fast-forward |
| `syndicate_presentation_test` | Path waits for dice; fast-forward ends both |
| `syndicate_market_history_test` | Ring capacity 32; oldest dropped; series match recorded index |
| `syndicate_trend_chart_test` | Mesh vertices inside chart rect; ≥6 labels; hotspot body |
| `syndicate_path_highlighter_test` | Pulse range; single option → 0 arrows; multi-option destinations + hover |
| `syndicate_hud_model_test` | phasePrompt words; note TTL expiry |
| `syndicate_describe_test` | `formatMoney` scales; Biotech `BuyShares{1}` label facts; `describeEvent` all 11 alternatives |
| `syndicate_activity_log_test` | Cap 64; `visible(10)` newest 10 chronological |
| `syndicate_action_bar_test` | legal−destination commands for AwaitRoll/Purchase/Auction/Manage; Num1 returns first slot |
| `syndicate_play_scene_test` | 4-bot GameOver + non-empty activity log with Round; human roll ignored on bot turn; human Num buy + Manage Upgrade/PlayCard; tile hover tooltip |
| `syndicate_scenes_test` | Menu cancel quits; Play/Settings/Help; setup Confirm → Play; pause Resume/Settings/Quit |
| `syndicate_control_map_test` | Phase coverage, hint bounds, bound key names in map text |
| `syndicate_help_scene_test` | Manage tab selected; Right moves to next tab |
| `syndicate_feedback_test` | `Rolled` / `FeePaid` each play ≥1 sound via `AudioSpy` |
| `syndicate_sprite_bank_test` | Empty bank nullopt; manifest entry → matching `SpriteCmd` source |
