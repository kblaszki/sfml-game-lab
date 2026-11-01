---
title: Snake documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - games/snake/CMakeLists.txt
  - games/snake/sim/CMakeLists.txt
  - games/snake/app/CMakeLists.txt
  - games/snake/main.cpp
related_docs:
  - reference/sim.md
  - reference/app.md
  - explanation/design.md
  - ../../../docs/index.md
  - ../../../docs/explanation/architecture.md
keywords: [snake, docs, index, diataxis, app, scenes]
last_reviewed: 2026-11-01
---

# Snake documentation

Snake on the `sgl` engine: `namespace sgl::snake`, targets `snake_sim`, `snake_app`, and the `snake` executable. Rules stay in the headless sim. The app reads that sim and draws it. Engine docs live in [docs/index.md](../../../docs/index.md).

| Quadrant | Document | Keywords |
|----------|----------|----------|
| Tutorials | none yet | |
| How-to | none yet | |
| Reference | [reference/sim.md](reference/sim.md) | SnakeGame, food, move interval, death, Playing, Dead |
| Reference | [reference/app.md](reference/app.md) | bindSnake, scenes, PlayScene, 40px board, SceneStack |
| Explanation | [explanation/design.md](explanation/design.md) | scene flow, menu, pause, game over |
