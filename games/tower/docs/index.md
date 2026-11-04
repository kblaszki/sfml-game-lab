---
title: Tower documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - games/tower/CMakeLists.txt
  - games/tower/main.cpp
  - games/tower/sim/CMakeLists.txt
  - games/tower/app/CMakeLists.txt
  - games/tower/sim/include/tower/sim/Types.hpp
  - games/tower/sim/include/tower/sim/Tuning.hpp
  - games/tower/sim/include/tower/sim/Events.hpp
  - games/tower/sim/include/tower/sim/TowerGame.hpp
  - games/tower/sim/src/TowerGame.cpp
related_docs:
  - reference/sim.md
  - reference/app.md
  - reference/assets.md
  - explanation/design.md
  - ../../../docs/index.md
  - ../../../docs/explanation/architecture.md
keywords: [tower, docs, index, diataxis, sim, app, TowerGame, platforms, camera, Landed, Died, scenes, sounds, high scores, assets, SpriteBank]
last_reviewed: 2026-11-04
---

# Tower documentation

Tower on the `sgl` engine: `namespace sgl::tower`, targets `tower_sim`, `tower_app`, and the `tower` executable. Rules stay in the headless sim. The app reads that sim and draws it. Engine docs live in [docs/index.md](../../../docs/index.md).

| Quadrant | Document | Keywords |
|----------|----------|----------|
| Tutorials | none yet | |
| How-to | none yet | |
| Reference | [reference/sim.md](reference/sim.md) | TowerGame, platforms, Landed, Died, camera, Playing, Dead |
| Reference | [reference/app.md](reference/app.md) | bindTower, scenes, PlayScene, sounds, high scores, SpriteBank, screenY, SceneStack |
| Reference | [reference/assets.md](reference/assets.md) | atlas, Kenney CC0, procedural fallback |
| Explanation | [explanation/design.md](explanation/design.md) | scene flow, menu, pause, game over, sounds |
