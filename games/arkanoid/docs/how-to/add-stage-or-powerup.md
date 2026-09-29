---
title: Add an Arkanoid stage or power-up
diataxis: how-to
audience: [ai, human]
related_code:
  - games/arkanoid/sim/include/arkanoid/sim/State.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Levels.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Effects.hpp
  - games/arkanoid/sim/include/arkanoid/sim/Physics.hpp
  - games/arkanoid/sim/src/Levels.cpp
  - games/arkanoid/sim/src/Effects.cpp
  - games/arkanoid/sim/src/Physics.cpp
  - games/arkanoid/sim/CMakeLists.txt
  - games/arkanoid/app/include/arkanoid/app/Assets.hpp
  - games/arkanoid/app/src/Assets.cpp
  - games/arkanoid/app/include/arkanoid/app/SceneRender.hpp
  - games/arkanoid/app/src/SceneRender.cpp
  - games/arkanoid/app/src/Sounds.cpp
  - games/arkanoid/app/src/Feedback.cpp
  - games/arkanoid/app/src/GameplayScene.cpp
  - games/arkanoid/app/src/HudModel.cpp
  - games/arkanoid/app/CMakeLists.txt
  - tests/arkanoid/sim/CMakeLists.txt
related_docs:
  - ../reference/sim.md
  - ../reference/app.md
  - ../explanation/physics-and-states.md
  - ../index.md
  - ../../../../docs/how-to/add-a-game.md
  - ../../../../docs/reference/source-layout.md
keywords: [arkanoid, StageId, LevelData, PowerUpKind, applyPowerUp, add_unit_test, CMakeLists]
last_reviewed: 2026-09-29
---

# Add an Arkanoid stage or power-up

Concrete steps against the current tree. Keep `arkanoid_sim` headless: no SFML includes; any randomness must go through `sgl::Pcg32` (the sim itself is deterministic today). New `.cpp` files must be listed in `games/arkanoid/sim/CMakeLists.txt` or `games/arkanoid/app/CMakeLists.txt`. Update [games/arkanoid/docs/reference/sim.md](../reference/sim.md) in the same change.

## Add a stage

1. Extend `enum class StageId` in `games/arkanoid/sim/include/arkanoid/sim/State.hpp` (e.g. `Stage4`).
2. In `games/arkanoid/sim/src/Levels.cpp`, add a `consteval` mask builder (same `levelColumns`×`levelRows` grid as Stage1–3), a `constexpr` mask, and a `LevelData` constant with `id`, dimensions, `stageColors` (or a new row-color span), and the mask.
3. Handle the new id in `level(StageId)` (today unknown ids fall through to Stage1).
4. Wire progression in `GameplayScene::advanceFrom` and the `StageCleared` branch in `GameplayScene::update` (`games/arkanoid/app/src/GameplayScene.cpp`): decide which clear advances to the new stage and which clear is a final win (`finishRound` / `Outcome::Won`). Today Stage3 is the win terminal.
5. Adjust HUD win copy if needed: `makeHud` in `HudModel.cpp` treats `"You win"` when `cleared && stage == Stage3`.
6. Add or extend a test under `tests/arkanoid/sim/` that calls `makeState(StageId::…)` and asserts brick count / mask occupancy (see stage brick counts in `SimTest.cpp`). Register a new `.cpp` with `add_unit_test(...)` in `tests/arkanoid/sim/CMakeLists.txt`, or extend an existing suite there.
7. Document the mask and brick count in [sim.md](../reference/sim.md).

`makeState(StageId)` already places bricks, donor drops (`liveIndex % 4 == 0`), centered paddle, and one Stuck ball. Prefer that path over hand-built boards unless you need custom bricks via `makeState(span<const Brick>)`.

## Add a power-up kind

1. Add a enumerator to `enum class PowerUpKind` in `State.hpp` (order matters for the donor formula `(liveIndex / 4) % 4` unless you change that rule in `Levels.cpp`).
2. Decide the effect:
   - **Timed** (like Wide / Slow): add a struct in `Effects.hpp`, extend `TimedEffect`, implement behavior in `Effects.cpp` (`paddleWidth` / `ballSpeedMultiplier` or new pure helpers), refresh/add in `applyPowerUp` (`Physics.cpp`), and expiry side effects in `tickTimedEffects`.
   - **Instant** (like MultiBall / ExtraLife): add a capsule tag type in the anonymous `PowerUp` variant in `Physics.cpp`, map it in `powerUpFromKind`, and handle it in the `std::visit` inside `applyPowerUp`.
3. Ensure catching still emits `PowerUpCaught{.kind = kind}` (already done when a capsule overlaps the paddle in `step`).
4. App visuals:
   - `AssetKeys` + builder in `Assets.hpp` / `Assets.cpp`, and an entry in `textureSpecs()`.
   - Grow `TextureIds::capsule` (`SceneRender.hpp`) and the upload/mapping in `main.cpp` / asset load path so `textures.capsule[static_cast<size_t>(kind)]` stays in range (`SceneRender.cpp`).
5. App audio: either reuse the shared `powerUp` blip (`Feedback.cpp` already plays `sounds_.powerUp` for every `PowerUpCaught`) or add a dedicated `SoundSpec` in `Sounds.cpp` / `SoundIds` and branch in `Feedback::onEvents`.
6. HUD: if timed, extend `makeHud` effect string formatting in `HudModel.cpp`.
7. Tests: cover apply, stack/refresh, and expiry under `tests/arkanoid/sim/` (pattern in `PowerUpTest.cpp`). Register new sources with `add_unit_test` in `tests/arkanoid/sim/CMakeLists.txt`.
8. Update the power-up tables in [sim.md](../reference/sim.md) and capsule notes in [app.md](../reference/app.md).

## Checklist reminders

- List every new `.cpp` in the owning module `CMakeLists.txt` (`sim` or `app`).
- Sim stays SFML-free; link only through existing `arkanoid_sim` → `sgl_collision` / `sgl_core`.
- Diátaxis: keep this how-to and the [physics explanation](../explanation/physics-and-states.md) consistent with code after behavioral changes.
