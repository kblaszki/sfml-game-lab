---
title: Syndicate UI presentation
diataxis: explanation
audience: [ai, human]
related_code:
  - games/syndicate/app/include/syndicate/app/PlayScene.hpp
  - games/syndicate/app/include/syndicate/app/SpriteBank.hpp
  - games/syndicate/app/src/SpriteBank.cpp
  - engine/render/include/sgl/render/Shapes.hpp
  - engine/render/include/sgl/render/DrawCommand.hpp
  - engine/sfml/include/sgl/sfml/SfmlPlatform.hpp
related_docs:
  - ../reference/syndicate-assets.md
  - ../reference/syndicate-app.md
  - syndicate-design.md
  - architecture.md
keywords: [syndicate, UI, procedural, MeshCmd, atlas, hybrid, presentation, animators]
last_reviewed: 2026-09-29
---

# Syndicate UI presentation

C++/SFML can draw UI chrome **procedurally**. The engine already wraps `sf::VertexArray` meshes as `sgl::MeshCmd`, and `Shapes` builds rounded rects, gradients, radial glows, polylines, and arrows. `SfmlPlatform` opens the window with MSAA (`antiAliasingLevel` 4), so soft edges stay acceptable without shaders. Shaders are unnecessary for this phase.

Raster PNG is reserved for **icons**, **card art**, and **backgrounds**. Everything else (panels, buttons, dice pips, board chrome) stays mesh-based until an optional atlas supplies a region.

## Hybrid model

1. Procedural chrome is always available.
2. An optional atlas (`atlas.png` + `atlas.txt`) may be loaded into `SpriteBank`.
3. Missing keys return `nullopt` from `SpriteBank::sprite` → fall back to procedural draw.

The game stays playable with zero PNGs.

## Presentation stays outside the sim

`Match::apply` is instant and headless. Animators (`PawnAnimator`, feedback FX, future card flips) live in the app layer and interpret `GameEvent` streams after each command. The sim never knows about sprites or meshes.

```mermaid
flowchart TD
  inputEvt[Input] --> playScene[PlayScene]
  playScene --> matchApply[Match]
  matchApply --> gameEvents[events]
  gameEvents --> presentation[presentation]
```
