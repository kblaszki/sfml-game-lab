---
title: Syndicate app
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/app/CMakeLists.txt
  - games/syndicate/app/include/syndicate/app/Theme.hpp
  - games/syndicate/app/include/syndicate/app/LayoutView.hpp
  - games/syndicate/app/include/syndicate/app/BoardRender.hpp
  - games/syndicate/app/include/syndicate/app/PawnAnimator.hpp
  - games/syndicate/app/include/syndicate/app/Bindings.hpp
  - games/syndicate/app/include/syndicate/app/HudModel.hpp
  - games/syndicate/app/include/syndicate/app/PlayScene.hpp
  - games/syndicate/app/src/Theme.cpp
  - games/syndicate/app/src/LayoutView.cpp
  - games/syndicate/app/src/BoardRender.cpp
  - games/syndicate/app/src/PawnAnimator.cpp
  - games/syndicate/app/src/Bindings.cpp
  - games/syndicate/app/src/HudModel.cpp
  - games/syndicate/app/src/PlayScene.cpp
  - tests/syndicate/app/CMakeLists.txt
  - tests/syndicate/app/LayoutViewTest.cpp
  - tests/syndicate/app/BoardRenderTest.cpp
  - tests/syndicate/app/PawnAnimatorTest.cpp
  - tests/syndicate/app/HudModelTest.cpp
  - tests/syndicate/app/PlaySceneTest.cpp
related_docs:
  - syndicate-sim.md
  - ../explanation/syndicate-design.md
  - engine-render.md
  - source-layout.md
  - ../explanation/architecture.md
keywords: [syndicate, app, Theme, LayoutView, renderBoard, PawnAnimator, Bindings, HudModel, PlayScene, CircleCmd, LineCmd, 1280x720]
last_reviewed: 2026-09-29
---

# Syndicate app

Headless presentation helpers and play scene in `namespace sgl::syndicate` for the Syndicate board UI. Static library `syndicate_app` links `PUBLIC` `syndicate_sim`, `sgl_render`, `sgl_scene`, `sgl_core`, `sgl_input`, and `sgl_ui`. No SFML includes — drawing uses `sgl::RenderQueue` and `CircleCmd` / `LineCmd` / `RectCmd` / `TextCmd` from `<sgl/render/DrawCommand.hpp>`. Include as `<syndicate/app/X.hpp>`. Design size matches other games: **1280×720** (board origin typically `{640, 360}`).

## Theme

`Theme` holds palette fields: six sector fills (AI … Fintech), `edge`, special tiles (`payday`, `exchange`, `hub`, `audit`, `grant`), `highlight`, and `pawn[4]`. `defaultTheme()` returns distinct, dark-readable colors.

## LayoutView

`LayoutView(origin, outerRadius)` maps `RingSlot` to screen positions:

| Ring | Radius | Slot count |
|------|--------|------------|
| 0 | `outerRadius` | 24 |
| 1 | `outerRadius * 0.62` | 12 |
| 2 | `outerRadius * 0.30` | 4 |

Angle is `slot / count * 2π`; ring 0 slot 0 sits at the top (−Y / smaller screen Y when Y grows down). `nodeRadius()` is `outerRadius * π * 0.7 / 24` (outer circumference / 24 / 2 × 0.7) so neighboring outer nodes do not overlap at 1280×720 with a typical outer radius (~320).

## renderBoard

`renderBoard(queue, board, ownership, layout, theme, highlighted)` pushes on `Layer::World`:

| Command | Count / rule |
|---------|----------------|
| `LineCmd` | One per **unique unordered** edge pair `(min(from,to), max(from,to))` — not one per directed edge |
| `CircleCmd` | One per node (`board.size()`); fill from tile kind (sector or special); outline from highlight / controller pawn / edge; `outlineThickness = level + 1` |

Highlighted destinations (`MoveOption::destination`) use `theme.highlight` as outline. Controlled tiles (no highlight) use that player's `theme.pawn` color.

## PawnAnimator

Constant-speed polyline motion (default **400** pixels/second via `setSpeed`). `start(id, points)` walks the polyline; `busy()` is true while any seat is mid-path. `position(id)` returns the animated point plus a fixed seat offset `Vec2f{(id - 1.5f) * 12.f, 0}` so up to four pawns on the same tile never stack.

## Bindings

`bindSyndicate(ActionMap&)` registers play and menu actions and returns `SyndicateActions` (`roll`, `endTurn`, `bid`, `passBid`, `pause`, plus `sgl::MenuActions`). Keys: **Space** roll, **Enter** end turn (also menu confirm), **C** bid (`sgl::Key` has no `B`), **P** pass bid, **Escape** pause, **Up/Down** menu, **Backspace** menu cancel. Digit keys are not actions (`InputState` has no digit query); path picks use the pointer.

## HudModel

`HudModel` owns a TTL note queue (`pushNote` / `update` drops expired). `phasePrompt(Phase)` returns a non-empty string containing **Roll**, **Path**, **Buy**, **Bid**, **Trade**, **Manage**, or **Over** for the matching phase. `render` pushes one prompt `TextCmd`, one cash `TextCmd` per seat (`Money.v` via `std::to_string`), six market `RectCmd` bars (height clamped into **50…200** from index), and note `TextCmd`s on `Layer::Hud`.

## PlayScene

`PlayScene(MatchConfig, vector<Seat>, FontId, SyndicateActions)` owns a `Match` (via `Match::create`), `PawnAnimator`, `LayoutView` origin `{640, 360}` outer radius **280**, `Theme` from `defaultTheme()`, and `HudModel`. `Seat::bot` marks AI seats; bot personas feed `chooseCommand` after a **0.4 s** think delay when `!animator.busy()`.

Traits: `opaque` true, `blocksUpdate` true, `pausable` true. Pause action → `RequestPause`. Human input maps through free function `commandFromInput` (pointer hit on highlighted nodes within `nodeRadius()*1.5`, Space/Enter/bid/pass as above; purchase: Enter → `DeclinePurchase`, Space → `BuyShares{1}` when legal). `ChooseDestination` starts pawn animation along `option.path` before apply; while `busy()`, further commands are ignored. On `GameOver`, `gameOver()` is true and `update` requests `PushScene` with a temporary nested ranking scene (replaceable by TASK-2.3 `GameOverScene`). Setup failure → single `QuitApp`.

## Tests

| Suite | Focus |
|-------|-------|
| `syndicate_layout_view_test` | Outer spacing, slot 0 above origin, ring 2 closer |
| `syndicate_board_render_test` | 40 circles, line count = unique unordered edges, centers match layout |
| `syndicate_pawn_animator_test` | 400px path at 400 px/s, `busy()`, distinct same-slot positions |
| `syndicate_hud_model_test` | phasePrompt words; note TTL expiry |
| `syndicate_play_scene_test` | 4-bot match reaches GameOver + PushScene; human roll ignored on bot turn at dt 0 |
