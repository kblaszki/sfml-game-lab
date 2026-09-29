---
title: Engine render queue and projections
diataxis: reference
audience: [ai, human]
related_code:
  - engine/render/include/sgl/render/AssetIds.hpp
  - engine/render/include/sgl/render/DrawCommand.hpp
  - engine/render/include/sgl/render/RenderQueue.hpp
  - engine/render/src/RenderQueue.cpp
  - engine/render/include/sgl/render/Shapes.hpp
  - engine/render/src/Shapes.cpp
  - engine/render/include/sgl/render/RendererI.hpp
  - engine/render/include/sgl/render/Projection.hpp
  - engine/render/include/sgl/render/Camera.hpp
  - tests/mocks/RendererMock.hpp
  - tests/engine/render/RenderQueueTest.cpp
  - tests/engine/render/ProjectionTest.cpp
  - tests/engine/render/ShapesTest.cpp
related_docs:
  - source-layout.md
  - engine-fx.md
  - ../explanation/architecture.md
keywords: [eng, RenderQueue, RendererI, DrawCommand, SortKey, Layer, Projection, Orthographic, Isometric, Camera, rotationDeg, LineCmd, CircleCmd, MeshCmd, Shapes, Vertex]
last_reviewed: 2026-09-29
---

# Engine render queue and projections

CPU-side draw list and 2.5D helpers in `namespace sgl`, included as `<sgl/render/X.hpp>`. Target `sgl_render` is a STATIC library that PUBLIC-links `sgl_core`. No SFML dependency.

## Asset ids

`TextureId` / `FontId` are `Handle<TextureTag>` / `Handle<FontTag>` from `AssetIds.hpp`. Resource backends own the tag types; they do not redefine the aliases.

## Draw commands

`DrawCommand` is `std::variant<SpriteCmd, RectCmd, TextCmd, LineCmd, CircleCmd, MeshCmd>`.

| Type | Role |
|------|------|
| `SpriteCmd` | textured quad: `texture`, `source`, `position`, `scale`, `tint`, `rotationDeg` |
| `RectCmd` | filled/outlined AABB: `rect`, `fill`, `outline`, `outlineThickness`, `rotationDeg` |
| `TextCmd` | string: `font`, `text`, `size`, `position`, `color`, `anchor` (`TopLeft` / `Center`) |
| `LineCmd` | segment: `from`, `to`, `thickness` (default `1`), `color` |
| `CircleCmd` | filled disk: `center`, `radius`, `fill`, optional `outline` / `outlineThickness`, `points` (default `32`) |
| `MeshCmd` | triangle mesh: `vertices` (`Vertex` = position + color + uv), `kind` (`Triangles` / `TriangleStrip`), optional `texture` |

`SpriteCmd::position` and `RectCmd::rect.pos` are the **top-left of the unrotated bounds**. `rotationDeg` rotates around the center of those bounds (default `0` — identical to the previous unrotated draw).

`LineCmd` spans `from` → `to`; thickness is the stroke width. `CircleCmd::center` is the circle center.

`MeshCmd` carries a CPU triangle list (or strip). When `texture` is nullopt the mesh is untextured (vertex colors only).

`Entry` pairs a `SortKey` with a `DrawCommand`. `SortKey` orders by `Layer` (Background → Overlay), then `depth`, then `seq`.

## Procedural shapes

`Shapes.hpp` builds `MeshCmd` values (`kind` = `Triangles`) without SFML:

| Function | Result |
|----------|--------|
| `roundedRect(rect, radius, color, segments=6)` | Filled rounded rect. Radius clamped to `[0, min(w,h)/2]`. Degenerate / `radius <= 0` → 6 verts (2 tris). Else `18 + 12 * segments` verts (full-height body inset on X, side strips between corner centers, 4 quarter-circles). |
| `roundedRectOutline(rect, radius, thickness, color, segments=6)` | Ring as perimeter quads (outer minus inner). Empty when `thickness <= 0` or the path degenerates. |
| `verticalGradient(rect, top, bottom)` | Exactly 6 verts; top edge uses `top`, bottom edge uses `bottom`. |
| `radialGlow(center, innerRadius, outerRadius, inner, segments=32)` | `segments` tris (`3 * segments` verts); inner ring keeps `inner` (incl. alpha), outer ring same RGB with alpha 0. Empty if `outerRadius <= innerRadius` or `segments < 3`. |
| `polyline(points, thickness, color)` | One quad (6 verts) per non-zero segment; empty for fewer than 2 points. |
| `arrowHead(tip, direction, size, color)` | Exactly 3 verts; one is `tip`; base width and length are `size` along normalized `direction` (zero → `{1,0}`). |

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
