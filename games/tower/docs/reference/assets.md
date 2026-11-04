---
title: Tower assets
diataxis: reference
audience: [ai, human]
related_code:
  - games/tower/app/include/tower/app/SpriteBank.hpp
  - games/tower/app/src/SpriteBank.cpp
  - games/tower/app/src/PlayScene.cpp
  - games/tower/main.cpp
  - assets/tower/atlas.png
  - assets/tower/atlas.txt
  - assets/tower/ATTRIBUTION.md
  - engine/resources/include/sgl/resources/Atlas.hpp
related_docs:
  - app.md
  - ../index.md
  - ../../../../docs/reference/engine-resources.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [tower, atlas, SpriteBank, Kenney, CC0, sprites, procedural fallback, ice, jumper]
last_reviewed: 2026-11-04
---

# Tower assets

Optional raster pack for Tower presentation. When both `assets/tower/atlas.png` and `atlas.txt` are present, `main` loads them into a `SpriteBank` and `PlayScene` draws `SpriteCmd`s. Missing files or missing region names keep the procedural path (`RectCmd` sky, platforms, and player). Design size **1280×720**. Art is cropped from Kenney Platformer Art Ice World and Jumper Pack (**CC0**); see [ATTRIBUTION.md](../../../../assets/tower/ATTRIBUTION.md).

## Format

- **PNG-32** sRGB with straight alpha.
- Manifest lines: `name x y w h` (`#` comments and blank lines allowed).
- Runtime: `assets/tower/atlas.png` + `atlas.txt`.

## Regions

| Name | Role | Source |
|------|------|--------|
| `bg/sky` | Full-screen play background, 1280×720 crop of sky plus clouds | Jumper Pack `bg_layer1.png` + `bg_layer2.png` |
| `player/idle` | Grounded jumper | Jumper Pack `bunny1_stand.png` |
| `player/jump` | Airborne jumper | Jumper Pack `bunny1_jump.png` |
| `plat/ice` | Ice platform tile, scaled to each platform box | Ice World `iceBlock.png` |

`SpriteBank::sprite(name, pos, size, tint, rotationDeg)` returns `nullopt` when the name is absent. `rotationDeg` defaults to **0**. `pos` is the top-left of the destination box; the SFML adapter rotates about the box center.

`PlayScene` uses `player/idle` while `player().grounded` and `player/jump` otherwise. When `vel.x < 0`, it negates `scale.x` and shifts `position.x` by the destination width. `toSprite` places the center at `position + halfSize * scale`, so that shift keeps the mirrored sprite in the same box. The packed bunny faces the camera, so the mirror is slight.

## Load path

`main.cpp` loads the atlas only when **both** files exist: `readTextFile` → `parseAtlas` → `SfmlAssets::textureFile("tower/atlas", …, smooth=true)` → `SpriteBank`. Any failure leaves an empty bank; the game still runs.
