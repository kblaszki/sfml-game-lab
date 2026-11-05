---
title: Tappy assets
diataxis: reference
audience: [ai, human]
related_code:
  - games/tappy/app/include/tappy/app/SpriteBank.hpp
  - games/tappy/app/src/SpriteBank.cpp
  - games/tappy/app/src/PlayScene.cpp
  - games/tappy/main.cpp
  - assets/tappy/atlas.png
  - assets/tappy/atlas.txt
  - assets/tappy/ATTRIBUTION.md
related_docs:
  - app.md
  - ../index.md
keywords: [tappy, atlas, SpriteBank, Kenney, procedural]
last_reviewed: 2026-11-05
---

# Tappy assets

Optional atlas at `assets/tappy/atlas.png` + `atlas.txt`. `main` loads both into `SpriteBank` when present.

| Region | Role |
|--------|------|
| `bg/sky` | Full-screen sky |
| `bg/hills` | Parallax mid ground |
| `player/blue` | Plane |
| `gate/rockTop` | Upper pillar |
| `gate/rockBottom` | Lower pillar |

See [ATTRIBUTION.md](../../../../assets/tappy/ATTRIBUTION.md).
