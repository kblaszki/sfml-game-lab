---
title: Tappy docs
diataxis: explanation
audience: [ai, human]
related_code:
  - games/tappy/CMakeLists.txt
  - games/tappy/main.cpp
related_docs:
  - reference/sim.md
  - reference/app.md
  - reference/assets.md
  - explanation/design.md
  - ../../../docs/explanation/architecture.md
  - ../../../docs/how-to/add-a-game.md
keywords: [tappy, flappy, index, docs]
last_reviewed: 2026-11-05
---

# Tappy

Side-scrolling flap game: a plane flies through gates. Headless rules live in `tappy_sim`; scenes and SFX in `tappy_app`; executable `tappy`.

| Doc | Role |
|-----|------|
| [reference/sim.md](reference/sim.md) | `TappyGame` flap, scroll, gates, score, death |
| [reference/app.md](reference/app.md) | Scenes, input, sounds, high scores |
| [reference/assets.md](reference/assets.md) | Atlas / SpriteBank |
| [explanation/design.md](explanation/design.md) | Scene flow and presentation contract |
