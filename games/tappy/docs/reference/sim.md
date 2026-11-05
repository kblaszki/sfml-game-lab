---
title: Tappy sim
diataxis: reference
audience: [ai, human]
related_code:
  - games/tappy/sim/include/tappy/sim/Types.hpp
  - games/tappy/sim/include/tappy/sim/Tuning.hpp
  - games/tappy/sim/include/tappy/sim/Events.hpp
  - games/tappy/sim/include/tappy/sim/TappyGame.hpp
  - games/tappy/sim/src/TappyGame.cpp
  - games/tappy/sim/CMakeLists.txt
  - tests/tappy/sim/TappyGameTest.cpp
related_docs:
  - ../index.md
  - app.md
  - ../../../../docs/how-to/write-headless-tests.md
keywords: [tappy, sim, TappyGame, flap, gates, scroll, Pcg32]
last_reviewed: 2026-11-05
---

# Tappy sim

Headless Flappy-style rules in `namespace sgl::tappy`. Target `tappy_sim` links `PUBLIC` `sgl_core` only. Include `<tappy/sim/X.hpp>`.

World is `kWorldW`×`kWorldH` (1280×720). +Y is up. The player auto-advances in +X; `cameraLeft = player.x - 0.28 * kWorldW`.

```mermaid
stateDiagram-v2
  [*] --> Playing
  Playing --> Playing: "Flapped GateCleared"
  Playing --> Dead: "gate or floor or ceiling"
  Dead --> Dead: "step returns nothing"
```

## Rules in `step`

1. Empty when `over()`.
2. If `flap` → `vy = flapSpeed`, emit `Flapped`.
3. Gravity on `vy`, clamp to `[−maxFall, maxRise]`, integrate Y.
4. `pos.x += scrollSpeed(distance) * dt`; update `distance`.
5. Spawn gates ahead (`gateSpacing`); cull behind camera; clear when player right passes gate right → `GateCleared`, `score++`.
6. AABB vs gate pillars → `Died`. Floor `y < 0` or ceiling `y + h > kWorldH` → `Died`.

Gap height: `max(gapMin, gapStart - gapShrink * gateX)`. Gap center from `Pcg32` within margins.
