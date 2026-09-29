---
title: Documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - CMakeLists.txt
  - games/arkanoid/main.cpp
related_docs:
  - tutorials/getting-started.md
  - how-to/build-and-test.md
  - how-to/add-a-game.md
  - reference/source-layout.md
  - reference/engine-core.md
  - reference/engine-audio.md
  - reference/engine-ui.md
  - reference/engine-input.md
  - reference/engine-scene.md
  - reference/engine-loop.md
  - reference/engine-render.md
  - reference/engine-fx.md
  - reference/engine-anim.md
  - reference/engine-collision.md
  - reference/engine-resources.md
  - reference/engine-save.md
  - reference/engine-sfml.md
  - reference/arkanoid-sim.md
  - reference/tetris-sim.md
  - reference/syndicate-sim.md
  - reference/syndicate-economy.md
  - reference/syndicate-cards.md
  - reference/syndicate-auction.md
  - reference/syndicate-app.md
  - reference/arkanoid-app.md
  - reference/tetris-app.md
  - reference/input-and-events.md
  - reference/pause-overlay.md
  - explanation/roadmap.md
  - explanation/architecture.md
  - explanation/syndicate-design.md
  - ../mvp/README.md
  - ../mvp/10-engine-progress.md
keywords: [documentation, diataxis, index, map, navigation, docs]
last_reviewed: 2026-09-29
---

# Documentation map

This directory documents **sfml-game-lab** following the [Diataxis](https://diataxis.fr/) framework. Every document starts with a YAML front-matter block; agents should search by the `related_code` and `keywords` fields to find the right page.

## How the docs are organized

```mermaid
flowchart TD
  Index[index.md] --> Tut[tutorials/ learning]
  Index --> How[how-to/ tasks]
  Index --> Ref[reference/ facts]
  Index --> Exp[explanation/ rationale]
  Ref -->|related_code| Code[engine/ and games/]
  How -->|related_code| Code
```

| Quadrant | When to read | Purpose |
|----------|--------------|---------|
| Tutorials | First time with the repo | Learn by doing, end to end |
| How-to | You have a specific task | Steps to reach a goal |
| Reference | You need a precise fact | Describe what exists, mirrors code |
| Explanation | You want the "why" | Design rationale and direction |

## All documents

| Document | Type | Keywords |
|----------|------|----------|
| [tutorials/getting-started.md](tutorials/getting-started.md) | tutorial | setup, toolchain, build, run, msys2 |
| [how-to/build-and-test.md](how-to/build-and-test.md) | how-to | build, test, ctest, format, presets |
| [how-to/add-a-game.md](how-to/add-a-game.md) | how-to | game, tetris, cmake, check_boundaries, sim, app |
| [reference/source-layout.md](reference/source-layout.md) | reference | layout, directories, sgl, arkanoid, tetris, targets |
| [reference/engine-core.md](reference/engine-core.md) | reference | sgl, Vec2, Rect, Handle, Image, Overloaded |
| [reference/engine-audio.md](reference/engine-audio.md) | reference | sgl, AudioI, tone, Pcm, SfmlAudio |
| [reference/engine-ui.md](reference/engine-ui.md) | reference | sgl, MenuModel, renderMenu, PauseOverlay |
| [reference/engine-input.md](reference/engine-input.md) | reference | InputEvent, ActionMap, InputState |
| [reference/engine-scene.md](reference/engine-scene.md) | reference | SceneStack, SceneRequest, pause |
| [reference/engine-loop.md](reference/engine-loop.md) | reference | FixedStepLoop, App, PlatformI |
| [reference/engine-render.md](reference/engine-render.md) | reference | RenderQueue, SortKey, Projection |
| [reference/engine-fx.md](reference/engine-fx.md) | reference | sgl, ParticleSystem, BurstParams, Pcg32 |
| [reference/engine-anim.md](reference/engine-anim.md) | reference | sgl, Easing, Tween, Spline, ArcLengthTable |
| [reference/engine-collision.md](reference/engine-collision.md) | reference | intersect, sweep, reflect |
| [reference/engine-resources.md](reference/engine-resources.md) | reference | ResourceCache, ResourceError |
| [reference/engine-save.md](reference/engine-save.md) | reference | sgl, HighScoreTable, SaveError, userDataDir |
| [reference/engine-sfml.md](reference/engine-sfml.md) | reference | SfmlPlatform, EventTranslate, letterbox |
| [reference/arkanoid-sim.md](reference/arkanoid-sim.md) | reference | arkanoid sim, StageId, power-ups |
| [reference/tetris-sim.md](reference/tetris-sim.md) | reference | tetris sim, Grid, SRS, kicks |
| [reference/syndicate-sim.md](reference/syndicate-sim.md) | reference | syndicate sim, Money, BoardGraph, pathing |
| [reference/syndicate-economy.md](reference/syndicate-economy.md) | reference | syndicate, Market, landing fee, dividends |
| [reference/syndicate-cards.md](reference/syndicate-cards.md) | reference | syndicate, Deck, CardKind, CardEffect |
| [reference/syndicate-auction.md](reference/syndicate-auction.md) | reference | syndicate, Auction, TradeOffer |
| [reference/syndicate-app.md](reference/syndicate-app.md) | reference | syndicate app, scenes, Feedback, executable syndicate |
| [reference/arkanoid-app.md](reference/arkanoid-app.md) | reference | Theme, Assets, HudModel, Sounds, Feedback, ResultScene, high score |
| [reference/tetris-app.md](reference/tetris-app.md) | reference | Theme, Bindings, BoardRender, Sounds, Feedback, high score |
| [reference/input-and-events.md](reference/input-and-events.md) | reference | InputEvent, ActionMap, defaultBindings |
| [reference/pause-overlay.md](reference/pause-overlay.md) | reference | PauseScene, RequestPause, FocusLost, pausable |
| [explanation/roadmap.md](explanation/roadmap.md) | explanation | roadmap, stages, power-ups, engine split |
| [explanation/architecture.md](explanation/architecture.md) | explanation | ADR, sgl, arkanoid, tetris, boundaries, ECS |
| [explanation/syndicate-design.md](explanation/syndicate-design.md) | explanation | syndicate rules, rings, economy, cards, phases |
| [../mvp/README.md](../mvp/README.md) | design notes (not Diátaxis) | prospective engine plan |
| [../mvp/10-engine-progress.md](../mvp/10-engine-progress.md) | living tracker (not Diátaxis) | slices, sgl modules, arkanoid, tetris |

`mvp/01`–`09` are prospective. Living engine progress is [`mvp/10-engine-progress.md`](../mvp/10-engine-progress.md). Current facts live in the Diátaxis pages above; prefer `engine/` and `games/` over chapter prose if they disagree.

## Maintenance

Documentation currency is enforced by the always-on rule `.cursor/rules/documentation.mdc` and the `update-docs` skill. When code changes, update the docs whose `related_code` lists the touched files, refresh their `last_reviewed`, and keep this table in sync.
