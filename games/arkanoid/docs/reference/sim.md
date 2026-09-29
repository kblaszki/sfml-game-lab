---
title: Arkanoid sim
diataxis: reference
audience: [ai, human]
related_code:
  - games/arkanoid/sim/include/arkanoid/sim/Tuning.hpp
  - games/arkanoid/sim/include/arkanoid/sim/State.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Effects.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Events.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Levels.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Physics.hpp
  - games/arkanoid/sim/src/Effects.cpp
  - games/arkanoid/sim/src/Levels.cpp
  - games/arkanoid/sim/src/Physics.cpp
  - games/arkanoid/sim/CMakeLists.txt
  - tests/arkanoid/sim/SimTest.cpp
  - tests/arkanoid/sim/PhysicsTest.cpp
  - tests/arkanoid/sim/SimPropertyTest.cpp
  - tests/arkanoid/sim/PowerUpTest.cpp
  - tests/arkanoid/sim/CMakeLists.txt
related_docs:
  - ../../../../docs/reference/engine-collision.md
  - ../../../../docs/reference/engine-core.md
  - app.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [arkanoid, sim, State, Effects, step, SimInput, SimEvent, makeState, PowerUpKind, StageId, sweep, TOI, Wide, MultiBall, Slow, ExtraLife, PaddleHit, WallHit, BallLaunched, BallLost]
last_reviewed: 2026-09-29
---

# Arkanoid sim

Headless breakout rules in `namespace sgl::arkanoid`. Static library target `arkanoid_sim` links `PUBLIC` `sgl_collision` (which pulls `sgl_core`). No SFML. Include as `<arkanoid/sim/X.hpp>`.

Design space is **1280×720** with **+y down**.

## Types

| Header | Contents |
|--------|----------|
| `Tuning.hpp` | Inline constexpr design size, paddle/ball/brick/capsule sizes and speeds, `paddleNormalWidth` 120 / `paddleWideWidth` 200, `ballSpeed` (length of `launchVelocity`), effect duration 8 s, slow factor 0.6, multiball spread 120, score 10, lives 3, grid 10×6 |
| `State.hpp` | `Ball` / `Paddle` / `Brick` / `Capsule` / `State`; enums `BallMode`, `PowerUpKind`, `StageId`; includes `Effects.hpp` |
| `Effects.hpp` | `Wide` / `Slow` (`Seconds remaining`), `TimedEffect` variant, `Effects { vector<TimedEffect> active }`; pure `paddleWidth(effects)`, `ballSpeedMultiplier(effects)` |
| `Events.hpp` | `BrickDestroyed`, `LifeLost`, `StageCleared`, `GameOver`, `PowerUpCaught`, `PaddleHit`, `WallHit`, `BallLaunched`, `BallLost`; `SimEvent` variant |
| `Levels.hpp` | `LevelData`, `level(StageId)`, `makeState(StageId)`, `makeState(span<const Brick>)` |
| `Physics.hpp` | `SimInput`, `step`, `applyPowerUp`, `ballSpeedMultiplier(const State&)` (delegates to Effects) |

`Ball::pos` is the top-left of a 16×16 box (radius 8). Paddle y is fixed at 680.

## Effects

`Effects::active` holds at most one `Wide` and one `Slow`. Catching the same kind again refreshes `remaining` to `effectDuration` (8 s). Wide and Slow stack independently.

| Pure function | Result |
|---------------|--------|
| `paddleWidth(effects)` | `paddleWideWidth` if Wide is active, else `paddleNormalWidth` |
| `ballSpeedMultiplier(effects)` | `slowFactor` (0.6) if Slow is active, else `1` |

Ball speed is always `ballSpeed * ballSpeedMultiplier` via normalization (`setBallSpeed` / launch / bounce). Slow never multiplies then divides velocities. Instant power-ups (MultiBall, ExtraLife) are dispatched with `std::visit` and `sgl::Overloaded`; capsules still store `PowerUpKind`.

## Levels and donors

`level(id)` returns Stage1/2/3 masks and row colors. Unknown ids fall back to Stage1.

`makeState(StageId)` builds a centered paddle, one **Stuck** ball on the paddle, and bricks only where the mask is `1`:

| Stage | Mask | Brick count |
|-------|------|-------------|
| Stage1 | all 1s | 60 |
| Stage2 | `(row+col)%2==0` | 30 |
| Stage3 | border frame | 28 |

`makeState(span<const Brick>)` copies the given bricks onto the same paddle/stuck-ball setup (Stage1 id) for custom test boards, including abutting bricks with no gap.

**Donor rule:** live-brick index `i` (mask-1 cells only, row-major, starting at 0). If `i % 4 == 0`, `drop = PowerUpKind((i / 4) % 4)` in enum order Wide, MultiBall, Slow, ExtraLife. Killing a brick with `drop` spawns a capsule at the brick position.

## `step` order

When `cleared` or `over`, `step` returns an empty event list and moves nothing.

Otherwise, with `SimInput { paddleAxis, launch }` and `dt`:

1. **Paddle** — `x += paddleSpeed * axis * dt`, clamp to `[0, designWidth - width]`. Then stick Stuck balls; **depenetrate** any Live ball overlapping the paddle (center above paddle midline → place on top; else push sideways in the paddle move direction). A sideways push is clamped so the ball sprite stays inside `[0, designWidth]`.
2. **Launch** — first Stuck ball becomes Live with direction of `launchVelocity` scaled to `ballSpeed * ballSpeedMultiplier(state)`; emit `BallLaunched`.
3. **Live balls (TOI)** — per ball, `remaining = 1`, up to 4 iterations. Candidates via `sgl::sweep`: alive bricks, three exterior wall AABBs (left/right/top), paddle AABB. Move to earliest `t`, resolve, continue with `remaining *= (1 - t)`.
   - **Brick seam:** all alive bricks with `|t - tBest| < 1e-4` die together (+10 and `BrickDestroyed` each, maybe capsule). Reflect once with the normalized sum of their normals (fallback: best normal). Renormalize speed.
   - **Wall:** reflect off every exterior wall the circle still overlaps (or the sweep normal when it only touches), push the center out by the overlap plus 0.01, renormalize, `WallHit`. A zero-time contact still separates before the TOI loop stops.
   - **Paddle:** `angle = clamp(hitOffset, -1, 1) * 60°` from vertical; `vel = speed * (sin a, -cos a)`; `PaddleHit`.
   - After every bounce: speed = `ballSpeed * ballSpeedMultiplier(state)`; enforce `|vy| >= speed * sin(15°)`.
4. **Capsules** — fall at 180 px/s; paddle AABB catch → `applyPowerUp` + `PowerUpCaught`; `pos.y >= 720` erases.
5. **Effects** — decrement each active `remaining`; erase when ≤0. Wide expiry restores `paddleNormalWidth` and clamps; Slow expiry renormalizes live balls to `ballSpeed` (absolute speed, no ÷0.6).
6. **Lives** — if every ball has `pos.y >= 720`: `LifeLost`, decrement lives; if lives remain, one Stuck ball; else `over` + `GameOver`. If some but not all exit: one `BallLost` per fallen ball, then erase them.
7. **Clear** — if bricks non-empty and none alive: set `cleared` and emit `StageCleared` once. An empty brick vector does **not** clear.

Stuck balls stay glued to the paddle each step. A 1200 px/s ball must still destroy a 28 px brick via sweep (no tunneling miss).

## `applyPowerUp`

| Kind | Behavior |
|------|----------|
| Wide | Refresh or add `Wide` timer 8 s; set paddle width from `paddleWidth(effects)` and clamp |
| Slow | Refresh or add `Slow` timer 8 s; renormalize live balls to `ballSpeed * slowFactor` |
| ExtraLife | `lives + 1`; does not touch timers |
| MultiBall | Two extras at the first ball. Direction is `launchVelocity` (if stuck) or the live velocity, with `x±120`, then both are renormalized to the current target speed |

Wide and Slow stack. MultiBall and ExtraLife do not clear timed effects.

## Tests

- `arkanoid_sim_test` — paddle clamp, wall bounce, brick score/events, bottom-face reflect, life loss/restick, StageCleared vs empty bricks, stage brick counts, Wide/ExtraLife, missed capsule, Slow expire restore, multiball count 3, high-speed sweep hit.
- `arkanoid_physics_test` — paddle side-sweep/depenetration, center/edge paddle angles, constant speed, min vertical angle, brick seams, remaining TOI time, corner normals, exterior walls (faces and top corners, including a ball already inside the right wall), `BallLost` vs `LifeLost`, launch/wall/paddle events.
- `arkanoid_sim_property_test` — 20 `Pcg32` seeds × 10 000 ticks: arena bounds, speed, no deep brick overlap, min vertical angle.
- `arkanoid_powerup_test` — Wide+Slow stack, recatch refresh, Slow expiry exact speed (50 cycles), Wide expiry width+clamp, MultiBall under Slow, ExtraLife leaves timers alone.
