---
title: Project direction and status
diataxis: explanation
audience: [ai, human]
related_code:
  - games/arkanoid/main.cpp
  - games/arkanoid/sim/include/arkanoid/sim/State.hpp
  - games/arkanoid/app/include/arkanoid/app/Scenes.hpp
  - engine/scene/include/sgl/scene/SceneStack.hpp
  - engine/loop/include/sgl/loop/App.hpp
related_docs:
  - ../reference/source-layout.md
  - ../../games/arkanoid/docs/reference/sim.md
  - architecture.md
  - ../../mvp/README.md
  - ../../mvp/10-engine-progress.md
keywords: [roadmap, direction, stages, power-ups, sgl, arkanoid, tetris, engine split]
last_reviewed: 2026-09-29
---

# Project direction and status

## Where it is now

A C++23 SFML 3.1 breakout on the landed engine split (`engine/` / `games/arkanoid/`): Start/Quit menu, three stages (Stage1 → Stage2 → Stage3), four power-ups (Wide, MultiBall, Slow, ExtraLife), pause overlay via `SceneStack`, headless tests. Legacy `src/` is removed. The engine namespace is `sgl`.

Facts: [source-layout.md](../reference/source-layout.md), [games/arkanoid/docs/reference/sim.md](../../games/arkanoid/docs/reference/sim.md). Contract: [architecture.md](architecture.md). Prospective notes: [`mvp/01`–`09`](../../mvp/README.md). Landed slices: [`mvp/10-engine-progress.md`](../../mvp/10-engine-progress.md).

## Where it is going

Phase II locks namespace `sgl`, fixes input edges and Arkanoid physics, turns power-ups into data, and adds `sgl_audio`, `sgl_fx`, `sgl_save`, and a headless Tetris under `games/tetris/`. Use `mvp/01`–`09` as prospective design notes, not as a description of the current code. Track landed slices in [`mvp/10-engine-progress.md`](../../mvp/10-engine-progress.md).
