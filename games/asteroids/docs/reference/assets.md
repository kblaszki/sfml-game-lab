---
title: Asteroids assets
diataxis: reference
audience: [ai, human]
related_code:
  - games/asteroids/app/include/asteroids/app/SpriteBank.hpp
  - games/asteroids/app/src/SpriteBank.cpp
  - games/asteroids/app/src/PlayScene.cpp
  - games/asteroids/main.cpp
  - assets/asteroids/atlas.png
  - assets/asteroids/atlas.txt
  - assets/asteroids/ATTRIBUTION.md
  - engine/resources/include/sgl/resources/Atlas.hpp
related_docs:
  - app.md
  - ../index.md
  - ../../../../docs/reference/engine-resources.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [asteroids, atlas, SpriteBank, Kenney, CC0, sprites, procedural fallback]
last_reviewed: 2026-11-04
---

# Asteroids assets

Optional raster pack for Asteroids presentation. When both `assets/asteroids/atlas.png` and `atlas.txt` are present, `main` loads them into a `SpriteBank` and `PlayScene` draws `SpriteCmd`s. Missing files or missing region names keep the procedural path (`RectCmd` background, `CircleCmd` rocks/bullets, ship `MeshCmd`). Design size **1280×720**. Art is cropped from Kenney Space Shooter Redux (**CC0**); see [ATTRIBUTION.md](../../../../assets/asteroids/ATTRIBUTION.md).

## Format

- **PNG-32** sRGB with straight alpha.
- Manifest lines: `name x y w h` (`#` comments and blank lines allowed).
- Runtime: `assets/asteroids/atlas.png` + `atlas.txt`.

## Regions

| Name | Role |
|------|------|
| `bg/space` | Full-screen play background |
| `ship/player` | Player ship (Kenney art points up; rotated from heading) |
| `rock/large` | Large meteor |
| `rock/medium` | Medium meteor |
| `rock/small` | Small meteor |
| `bullet` | Shot |

`SpriteBank::sprite(name, pos, size, tint, rotationDeg)` returns `nullopt` when the name is absent. `pos` is the top-left of the destination box; the SFML adapter rotates about the box center. Ship heading `0` (+X in sim) maps to `rotationDeg = 90 - headingDeg` so the up-facing sprite aims along +X on the Y-flipped screen.

## Load path

`main.cpp` loads the atlas only when **both** files exist: `readTextFile` → `parseAtlas` → `SfmlAssets::textureFile("asteroids/atlas", …, smooth=true)` → `SpriteBank`. Any failure leaves an empty bank; the game still runs.
