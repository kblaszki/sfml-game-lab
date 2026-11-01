---
title: Asteroids documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - games/asteroids/CMakeLists.txt
  - games/asteroids/sim/CMakeLists.txt
  - games/asteroids/app/CMakeLists.txt
  - games/asteroids/main.cpp
  - games/asteroids/sim/include/asteroids/sim/Types.hpp
  - games/asteroids/sim/include/asteroids/sim/Tuning.hpp
  - games/asteroids/sim/include/asteroids/sim/Events.hpp
  - games/asteroids/sim/include/asteroids/sim/AsteroidsGame.hpp
  - games/asteroids/sim/src/AsteroidsGame.cpp
related_docs:
  - reference/sim.md
  - reference/app.md
  - explanation/design.md
  - ../../../docs/index.md
  - ../../../docs/explanation/architecture.md
keywords: [asteroids, docs, index, diataxis, sim, app, scenes, AsteroidsGame]
last_reviewed: 2026-11-01
---

# Asteroids documentation

Asteroids on the `sgl` engine: `namespace sgl::asteroids`, targets `asteroids_sim`, `asteroids_app`, and the `asteroids` executable. Rules stay in the headless sim. The app reads that sim and draws it. Engine docs live in [docs/index.md](../../../docs/index.md).

| Quadrant | Document | Keywords |
|----------|----------|----------|
| Tutorials | none yet | |
| How-to | none yet | |
| Reference | [reference/sim.md](reference/sim.md) | AsteroidsGame, rocks, bullets, wrap, Playing, Dead, Wave |
| Reference | [reference/app.md](reference/app.md) | bindAsteroids, scenes, PlayScene, ship triangle, SceneStack |
| Explanation | [explanation/design.md](explanation/design.md) | scene flow, menu, pause, game over |
