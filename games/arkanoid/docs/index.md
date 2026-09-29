---
title: Arkanoid documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - games/arkanoid/CMakeLists.txt
  - games/arkanoid/main.cpp
related_docs:
  - reference/sim.md
  - reference/app.md
  - how-to/add-stage-or-powerup.md
  - explanation/physics-and-states.md
  - ../../../docs/index.md
  - ../../../docs/explanation/architecture.md
keywords: [arkanoid, breakout, docs, index, diataxis]
last_reviewed: 2026-09-29
---

# Arkanoid documentation

Breakout on the `sgl` engine: `namespace sgl::arkanoid`, targets `arkanoid_sim`, `arkanoid_app`, and executable `arkanoid`. Engine docs live in [docs/index.md](../../../docs/index.md).

| Quadrant | Document | Keywords |
|----------|----------|----------|
| Tutorials | none yet | |
| How-to | [how-to/add-stage-or-powerup.md](how-to/add-stage-or-powerup.md) | StageId, LevelData, PowerUpKind, applyPowerUp, CMakeLists |
| Reference | [reference/sim.md](reference/sim.md) | State, step, SimEvent, power-ups, StageId |
| Reference | [reference/app.md](reference/app.md) | scenes, HUD, sounds, feedback, high scores |
| Explanation | [explanation/physics-and-states.md](explanation/physics-and-states.md) | TOI, sweep, reflect, paddle bounce, BallMode, SimEvent |
