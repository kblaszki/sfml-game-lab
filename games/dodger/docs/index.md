---
title: Dodger documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - games/dodger/CMakeLists.txt
  - games/dodger/main.cpp
  - games/dodger/sim/CMakeLists.txt
  - games/dodger/app/CMakeLists.txt
  - games/dodger/sim/include/dodger/sim/Types.hpp
  - games/dodger/sim/include/dodger/sim/Tuning.hpp
  - games/dodger/sim/include/dodger/sim/Events.hpp
  - games/dodger/sim/include/dodger/sim/DodgerGame.hpp
  - games/dodger/sim/src/DodgerGame.cpp
related_docs:
  - reference/sim.md
  - reference/app.md
  - explanation/design.md
  - ../../../docs/index.md
  - ../../../docs/explanation/architecture.md
keywords: [dodger, docs, index, diataxis, sim, app, scenes, sounds, high scores, DodgerGame, hazards, paddle]
last_reviewed: 2026-11-01
---

# Dodger documentation

Dodger on the `sgl` engine: `namespace sgl::dodger`, targets `dodger_sim`, `dodger_app`, and the `dodger` executable. Rules stay in the headless sim. The app reads that sim and draws it. Engine docs live in [docs/index.md](../../../docs/index.md).

| Quadrant | Document | Keywords |
|----------|----------|----------|
| Tutorials | none yet | |
| How-to | none yet | |
| Reference | [reference/sim.md](reference/sim.md) | DodgerGame, paddle, hazards, HazardCleared, Died, Playing, Dead |
| Reference | [reference/app.md](reference/app.md) | bindDodger, scenes, PlayScene, sounds, high scores, RectCmd, SceneStack |
| Explanation | [explanation/design.md](explanation/design.md) | scene flow, menu, pause, game over |
