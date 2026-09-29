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
  - games/syndicate/app/include/syndicate/app/Describe.hpp
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
  - games/syndicate/app/src/Theme.cpp
  - games/syndicate/app/src/LayoutView.cpp
  - games/syndicate/app/src/BoardRender.cpp
  - games/syndicate/app/src/PawnAnimator.cpp
  - games/syndicate/app/src/DiceAnimator.cpp
  - games/syndicate/app/src/Presentation.cpp
  - games/syndicate/app/src/Bindings.cpp
  - games/syndicate/app/src/Describe.cpp
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
  - tests/syndicate/app/ActionBarTest.cpp
  - tests/syndicate/app/PlaySceneTest.cpp
  - tests/syndicate/app/ScenesTest.cpp
  - tests/syndicate/app/FeedbackTest.cpp
  - tests/syndicate/app/SpriteBankTest.cpp
related_docs:
  - syndicate-sim.md
  - syndicate-assets.md
  - ../explanation/syndicate-design.md
  - ../explanation/syndicate-ui.md
  - engine-render.md
  - engine-ui.md
  - engine-save.md
  - source-layout.md
  - ../explanation/architecture.md
keywords: [syndicate, app, Theme, LayoutView, renderBoard, HandView, MarketHistory, TrendChart, PathHighlighter, PawnAnimator, DiceAnimator, Presentation, Bindings, Describe, ActionBar, HudModel, PlayScene, tooltip, Scenes, Feedback, Sounds, SpriteBank, MainMenuScene, SetupScene, GameOverScene, syndicate executable, 1280x720]
last_reviewed: 2026-09-29
---

# Syndicate app

Presentation layer and playable executable for Syndicate in `namespace sgl::syndicate`. Static library `syndicate_app` links `PUBLIC` `syndicate_sim`, `sgl_render`, `sgl_scene`, `sgl_core`, `sgl_input`, `sgl_ui`, `sgl_anim`, `sgl_audio`, `sgl_fx`, `sgl_save`, and `sgl_resources`. No SFML includes in the library — drawing uses `sgl::RenderQueue` and `CircleCmd` / `LineCmd` / `RectCmd` / `TextCmd` / optional `SpriteCmd`. Include as `<syndicate/app/X.hpp>`. Design size **1280×720**. Executable target `syndicate` (`games/syndicate/main.cpp`) links `syndicate_app` and `sgl_sfml`.

## SyndicateServices

`SyndicateServices` holds references owned by `main()` for the whole `App::run()`: `actions`, `font`, `seed`, `audio`, `sounds`, `scoresPath`, `highScores`, `sprites` (`SpriteBank`, default empty), and `bodyFont` (body / tooltip TTF; id 0 means fall back to `font`). Scene factories in `Scenes.hpp` capture this struct by value (shallow copies of references). Font path constants: `uiFontRelativePath`, `bodyFontRelativePath`.

| Factory | Scene |
|---------|--------|
| `mainMenu` | `MainMenuScene` (Play / Quit) |
| `setup` | `SetupScene` (2–4 seats, Human/Bot, Confirm) |
| `play` | `PlayScene` with `MatchConfig` + seats |
| `pause` | `sgl::PauseOverlay` |
| `makeGameOver` | `GameOverScene` with ranking + high-score save |

## SpriteBank

`SpriteBank` maps atlas region names to `SpriteCmd` (texture + source rect + position/scale/tint). Default / empty manifest: `sprite(...)` returns `nullopt` so callers keep procedural fallbacks. Optional load in `main.cpp` when both `assets/syndicate/atlas.png` and `atlas.txt` exist.

## Sounds / Feedback

`soundSpecs()` returns PCM from `sgl::blip` (`roll`, `fee`, `auction`, `card`, `market`). `main.cpp` uploads via `SfmlAudio`; the app library only holds `SoundIds` / specs. `Feedback(AudioI&, SoundIds)` plays on `Rolled`, `FeePaid`, `AuctionDone`, `CardPlayed`, `MarketShifted`; other `GameEvent` alternatives are silent.

## Menu / setup / game over

- **MainMenuScene:** `MenuModel` + `renderMenu`. Play → `PushScene{setup}`; Quit / cancel / Escape → `QuitApp`.
- **SetupScene:** cycles seat count 2→3→4, toggles Human/Bot per seat, Confirm → `ReplaceScene{play}` with `MatchConfig{seed, seats, startCash 1500, maxRounds 15, targetNetWorth 6000}`.
- **GameOverScene:** ranking lines (`P1`… by player id, net worth). Menu → pop + replace main menu; Quit → `QuitApp`. Inserts the winner's net worth into `HighScoreTable` and `ScoreStore::save`; save failure still shows the scene.

## Theme / layout / board / HUD

`Theme` / `defaultTheme()`, `LayoutView`, `renderBoard`, `PawnAnimator`, `DiceAnimator`, `Presentation`, `HudModel`, `HandView`, `MarketHistory`, `TrendChart`, `PathHighlighter`. Bindings: **Space** roll, **Enter** end turn / menu confirm, **C** bid, **P** pass, **Escape** pause, **Num1–Num9** action-bar slots, **Up/Down** menu, **Backspace** cancel.

`renderBoard` draws 40 node circles, unique undirected edges, and a size-11 short label per node (`AI`/`CLN`/`BIO`/`CYB`/`LOG`/`FIN`, `PAY`/`EXC`/`HUB`/`AUD`/`GRA`) just below the node. Optional trailing `FontId` + `SpriteBank*` + `HighlightModel*` (defaults) emit centered atlas sprites when present (`sector/*`, `tile/*`); when `highlights` is non-null, destination nodes use a thicker outline.

`HudModel` draws one rounded player panel per seat (`playerPanelRect`, x=16, y=48+i×36, 300×32) with `P{n} {formatMoney(cash)}`; the acting seat gets a brighter outline. Phase prompt and note TTL are unchanged. Market bars are replaced by `renderTrendChart` in the right panel (`marketChartRect`, x=940, y=16, w=324, h=284): six stacked sparklines from an internal `MarketHistory` (ring of 32 samples per sector). `recordMarket` appends the current `Market` index; `chartHotspots()` returns one hotspot per sector row.

`PathHighlighter` builds legal-move destinations and junction arrows from `MoveOption` paths (arrow tip at 60% along a diverging edge). `update` pulses radius in `[0.88, 1.12]` and sets `hovered` when the pointer is within `1.5 * nodeRadius` of a destination. `render` draws `radialGlow`, `arrowHead`, and a thickness-5 path polyline for the hovered option.

## Dice / pawn presentation

Match rolls are **d4+d4** (faces 1..4, total 2..8). `DiceAnimator` shows two procedural dice (rounded rect + pip circles) with tumble intervals `0.04s * 1.35^k`, then an `easeOutBack` scale pop. It owns a cosmetic `sgl::Pcg32` seeded with `seed ^ 0xD1CE` — **not** `GameState::rng`, so match determinism is unchanged.

`PawnAnimator` moves along a Catmull-Rom sample (`catmullRom(points, 8)` + `ArcLengthTable`) with distance `length * easeInOutCubic(elapsed/T)`, `T = min(0.25s * hops, 1.6s)`. `Presentation` sequences dice then path (queues path while dice busy); `fastForward` (click or roll key) finishes both. Human/bot commands are gated on `!presentation.busy()`.

## HandView

`HandView` mirrors the acting player's `Hand` (cap 5). Cards are 96×134; layout is a column at x=24 starting y=420 (8px gap) because five row strides do not fit in the left panel width. Newly appeared cards tween from x=−120 with `easeOutBack` over 0.35s; removed cards slide toward x=400 and fade out over 0.35s. `hit` / hotspots use settled slot rects; `render` uses the animated position. Hotspot body is `describeCard`.

## Describe helpers

`Describe.hpp` formats English UI copy for money, sectors, tiles, cards, and legal `Command`s:

- `formatMoney` — 1 sim unit = $1k (`$0`, `$150k`, `$1M`, `$1.5M`); integer math only.
- `sectorName` / `describeCard` / `describeTile` — sector labs include share price, holdings, level, and `landingFee`.
- `describeCommand` — button label + one-sentence tooltip (costs via `sharePrice`, `upgradeCost`, buy-card `$100k`, momentum `$25k`).

## ActionBar

`ActionBar` rebuilds from `Match::legalCommands`, drops `ChooseDestination`, prefers commands for `selectedTile` when capping at 9, and lays out up to nine buttons in y 652..708 / x 16..1264. Slots bind to **Num1–Num9**; pointer release on a button applies that command. Enter is not treated as a bar click (legacy `EndTurn` path stays on Enter). Hotspots feed tooltips (`title` = label, `body` = tooltip).

## PlayScene

Two constructors: test-friendly `(MatchConfig, seats, FontId, SyndicateActions)` and services `(SyndicateServices, MatchConfig, seats)` which also owns `Feedback`. Owns `ActionBar`, `HandView`, `HoverAnim`, `TooltipSystem`, `Presentation` (seed from `MatchConfig::seed` or `services.seed`), and `PathHighlighter`. Records the market into `HudModel` at startup and after each successful `apply`. On a human turn while presentation is idle: rebuilds the bar, optionally selects a Manage/AwaitPurchase tile from a board click (not on a button), then applies `actionBar.update` before falling back to `commandFromInput` (Space/Enter/C/P/path click). While presentation is busy, pointer press or roll key calls `presentation.fastForward()` and does not apply a command. In `AwaitPath`, builds/updates the path highlighter and passes `&highlighter.model()` into `renderBoard`, then `highlighter.render` after the board. Each update collects hotspots for board tiles (`describeTile`), action-bar buttons, hand cards, market chart rows, and player panels, then drives `TooltipSystem` (font = `bodyFont` when set, else scene `font_`). Hover does not consume clicks. Renders board/highlights/pawns, HUD, dice at layout origin, hand, action bar, then tooltip last. Pause → `RequestPause`. On `GameOver`, pushes `GameOverScene` (via `makeGameOver` when services are present). `drainEvents` feeds `Presentation::onRolled` and `Feedback` after each applied command.

## main.cpp

`SfmlPlatform` **1280×720** titled **"Syndicate"**, loads `uiFontRelativePath` then `bodyFontRelativePath` under `ASSET_DIR` (same `fail()` on either), optionally loads the syndicate atlas into `SpriteBank`, uploads sounds, resolves `userDataDir("syndicate") / "syndicate.scores"` (temp fallback), `bindSyndicate`, pushes main menu on a `SceneStack` with the pause factory, runs `sgl::App`. Seed from `std::chrono::steady_clock`.

## Tests

| Suite | Focus |
|-------|-------|
| `syndicate_layout_view_test` | Outer spacing, slot 0 above origin, ring 2 closer |
| `syndicate_board_render_test` | 40 circles, 40 short labels, line count = unique unordered edges, centers match layout |
| `syndicate_hand_view_test` | Appear tween: x off-screen at dt=0, at slot x after 0.35s |
| `syndicate_pawn_animator_test` | Catmull-Rom ease-in-out path, `busy()`, distinct same-slot positions |
| `syndicate_dice_animator_test` | d4+d4 tumble intervals non-decreasing, settle on roll 7, fast-forward |
| `syndicate_presentation_test` | Path waits for dice; fast-forward ends both |
| `syndicate_market_history_test` | Ring capacity 32; oldest dropped; series match recorded index |
| `syndicate_trend_chart_test` | Mesh vertices inside chart rect; ≥6 labels; hotspot body |
| `syndicate_path_highlighter_test` | Pulse range; single option → 0 arrows; multi-option destinations + hover |
| `syndicate_hud_model_test` | phasePrompt words; note TTL expiry |
| `syndicate_describe_test` | `formatMoney` scales; Biotech `BuyShares{1}` label facts |
| `syndicate_action_bar_test` | legal−destination commands for AwaitRoll/Purchase/Auction/Manage; Num1 returns first slot |
| `syndicate_play_scene_test` | 4-bot GameOver; human roll ignored on bot turn; human Num buy + Manage Upgrade/PlayCard; tile hover tooltip |
| `syndicate_scenes_test` | Menu cancel quits; Play pushes setup; setup Confirm replaces with PlayScene |
| `syndicate_feedback_test` | `Rolled` / `FeePaid` each play ≥1 sound via `AudioSpy` |
| `syndicate_sprite_bank_test` | Empty bank nullopt; manifest entry → matching `SpriteCmd` source |
