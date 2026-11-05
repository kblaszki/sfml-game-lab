---
title: Tappy app
diataxis: reference
audience: [ai, human]
related_code:
  - games/tappy/app/include/tappy/app/Theme.hpp
  - games/tappy/app/include/tappy/app/Bindings.hpp
  - games/tappy/app/include/tappy/app/InputMapping.hpp
  - games/tappy/app/include/tappy/app/Sounds.hpp
  - games/tappy/app/include/tappy/app/Feedback.hpp
  - games/tappy/app/include/tappy/app/SpriteBank.hpp
  - games/tappy/app/include/tappy/app/Scenes.hpp
  - games/tappy/app/src/PlayScene.cpp
  - games/tappy/main.cpp
  - tests/tappy/app/PlaySceneTest.cpp
  - tests/tappy/app/ScenesTest.cpp
related_docs:
  - sim.md
  - assets.md
  - ../explanation/design.md
keywords: [tappy, app, SceneStack, flap, Feedback, SpriteBank]
last_reviewed: 2026-11-05
---

# Tappy app

Presentation for `TappyGame`. No SFML outside `main.cpp`.

## Input

| Keys | Action |
|------|--------|
| Space, Up, W | flap (pressed edge) |
| Left, A / Right, D | menu up / down |
| Enter | confirm |
| Backspace | cancel |
| Escape, P | pause |

## Sounds

`flap`, `clear`, `die` via `sgl::blip`. `Feedback` plays them for `Flapped`, `GateCleared`, `Died`.

## Playfield

`screenX = worldX - cameraLeft`, `screenY = kWorldH - (worldY + height)`. Parallax hills use `cameraLeft * 0.35`. Sprites when atlas regions exist; else Theme rects.
