---
title: Engine render queue and projections
diataxis: reference
audience: [ai, human]
related_code:
  - engine/render/include/sgl/render/AssetIds.hpp
  - engine/render/include/sgl/render/DrawCommand.hpp
  - engine/render/include/sgl/render/RenderQueue.hpp
  - engine/render/src/RenderQueue.cpp
  - engine/render/include/sgl/render/RendererI.hpp
  - engine/render/include/sgl/render/Projection.hpp
  - engine/render/include/sgl/render/Camera.hpp
  - tests/mocks/RendererMock.hpp
  - tests/engine/render/RenderQueueTest.cpp
  - tests/engine/render/ProjectionTest.cpp
related_docs:
  - source-layout.md
  - engine-fx.md
  - ../explanation/architecture.md
keywords: [eng, RenderQueue, RendererI, DrawCommand, SortKey, Layer, Projection, Orthographic, Isometric, Camera, rotationDeg, LineCmd, CircleCmd]
last_reviewed: 2026-09-29
---

# Engine render queue and projections

CPU-side draw list and 2.5D helpers in `namespace sgl`, included as `<sgl/render/X.hpp>`. Target `sgl_render` is a STATIC library that PUBLIC-links `sgl_core`. No SFML dependency.

## Asset ids

`TextureId` / `FontId` are `Handle<TextureTag>` / `Handle<FontTag>` from `AssetIds.hpp`. Resource backends own the tag types; they do not redefine the aliases.

## Draw commands

`DrawCommand` is `std::variant<SpriteCmd, RectCmd, TextCmd, LineCmd, CircleCmd>`.

| Type | Role |
|------|------|
| `SpriteCmd` | textured quad: `texture`, `source`, `position`, `scale`, `tint`, `rotationDeg` |
| `RectCmd` | filled/outlined AABB: `rect`, `fill`, `outline`, `outlineThickness`, `rotationDeg` |
| `TextCmd` | string: `font`, `text`, `size`, `position`, `color`, `anchor` (`TopLeft` / `Center`) |
| `LineCmd` | segment: `from`, `to`, `thickness` (default `1`), `color` |
| `CircleCmd` | filled disk: `center`, `radius`, `fill`, optional `outline` / `outlineThickness`, `points` (default `32`) |

`SpriteCmd::position` and `RectCmd::rect.pos` are the **top-left of the unrotated bounds**. `rotationDeg` rotates around the center of those bounds (default `0` — identical to the previous unrotated draw).

`LineCmd` spans `from` → `to`; thickness is the stroke width. `CircleCmd::center` is the circle center.

`Entry` pairs a `SortKey` with a `DrawCommand`. `SortKey` orders by `Layer` (Background → Overlay), then `depth`, then `seq`.

## RenderQueue

- `push(layer, depth, command)` — assigns `seq` incrementally starting at 0
- `sorted()` — non-const; `std::ranges::stable_sort` by `SortKey` once; returns `std::span<const Entry>`; stays cached until the next `push` or `clear`
- `size()` / `clear()` — `clear` empties entries and resets `seq` to 0

## RendererI

Port for a backend: `begin()`, `submit(RenderQueue&)`, `end()`. `submit` takes a non-const queue so backends can call `sorted()` without casting. Tests use `RendererMock` (`tests/mocks/RendererMock.hpp`).

## Projection and camera

`Projection` requires `project(Vec3f) -> Projected` (`screen`, `depth`).

| Type | Mapping |
|------|---------|
| `Orthographic` | screen `(x, y)`, depth `z` |
| `Isometric` | screen `((x-y)*tileW/2, (x+y)*tileH/2 - z*tileH)`, depth `x+y+z*0.001` |

`Camera::apply(screen)` returns `(screen - offset) * zoom`.
