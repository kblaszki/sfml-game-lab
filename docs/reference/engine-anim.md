---
title: Engine animation easing, tween, and splines
diataxis: reference
audience: [ai, human]
related_code:
  - engine/anim/include/sgl/anim/Easing.hpp
  - engine/anim/include/sgl/anim/Tween.hpp
  - engine/anim/include/sgl/anim/Spline.hpp
  - engine/anim/src/Spline.cpp
  - engine/anim/CMakeLists.txt
  - tests/engine/anim/AnimTest.cpp
  - tests/engine/anim/CMakeLists.txt
related_docs:
  - source-layout.md
  - engine-core.md
keywords: [eng, Easing, EaseFn, Tween, Spline, cubicBezier, catmullRom, ArcLengthTable, Seconds]
last_reviewed: 2026-09-29
---

# Engine animation easing, tween, and splines

Headless motion helpers in `namespace sgl`, included as `<sgl/anim/X.hpp>`. Target `sgl_anim` is a STATIC library that PUBLIC-links `sgl_core`. No SFML dependency. Intended for pawn (and similar) motion that eases along a curve without blocking the game loop.

## Easing

`EaseFn` is `float (*)(float)`. Each easing clamps `t` to `[0, 1]` before evaluating, and satisfies `f(0) == 0`, `f(1) == 1`.

| Function | Formula (after clamp) |
|----------|------------------------|
| `linear` | `t` |
| `easeInQuad` | `t*t` |
| `easeOutCubic` | `1 - (1-t)^3` |
| `easeInOutCubic` | `t < 0.5 ? 4*t^3 : 1 - (-2*t+2)^3 / 2` |
| `easeOutBack` | `1 + c3*(t-1)^3 + c1*(t-1)^2` with `c1 = 1.70158`, `c3 = c1+1` |

`easeOutBack` may overshoot slightly above `1` for `t` in `(0, 1)`; that is intentional.

## Tween

Header-only `Tween<T>`. `T` must support `T + (T - T) * float` (`float` and `Vec2f` do). Duration uses `Seconds` from `<sgl/core/Time.hpp>`.

- `start(from, to, duration, ease)` — begins interpolation; `ease == nullptr` means `linear`
- `duration <= 0` snaps to `to` and finishes immediately
- `update(dt)` advances elapsed time; past the end clamps to `to` and stays done
- Before `start()`, `value()` is `T{}` and `done()` is true
- `running()` is true only while a positive-duration tween is in progress

## Spline

`cubicBezier(p0, p1, p2, p3, t)` — standard cubic Bernstein polynomial; `t` clamped to `[0, 1]`.

`catmullRom(points, samplesPerSegment)` — uniform Catmull-Rom polyline. Returns empty if fewer than 2 points or `samplesPerSegment < 1`. Endpoints are duplicated for tangents. Each segment emits `samplesPerSegment` samples after its start (`t = 1/N … 1`), so joints appear once; output starts at `points.front()` and ends at `points.back()`, and every control point appears in the sample list.

## ArcLengthTable

Cumulative chord-length table over a polyline:

- `build(points)` — rebuilds; fewer than 2 points yields an empty table (`length() == 0`, `at` returns `{}`)
- `length()` — total path length
- `at(distance)` — position along the path: `distance <= 0` → first point; `distance >= length()` → last point; otherwise binary-searches cumulative lengths (`std::lower_bound`) and lerps the segment

Pair with a `Tween<float>` on `[0, length()]` and an easing function to move a pawn along a curve at a non-blocking, eased pace.
