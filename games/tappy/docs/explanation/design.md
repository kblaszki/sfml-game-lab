---
title: Tappy scene flow
diataxis: explanation
audience: [ai, human]
related_code:
  - games/tappy/app/src/PlayScene.cpp
  - games/tappy/main.cpp
related_docs:
  - ../reference/sim.md
  - ../reference/app.md
  - ../reference/assets.md
  - ../index.md
keywords: [tappy, scenes, design, camera, flap]
last_reviewed: 2026-11-05
---

# Tappy scene flow

The window consumes `TappyGame`. Flap, scroll, gates, score, and death stay in `tappy_sim`. Scenes map pressed flap, call `step`, and draw with camera-relative coordinates.

```mermaid
stateDiagram-v2
  [*] --> mainMenu
  mainMenu --> play: Play
  mainMenu --> quitApp: "Quit or cancel"
  play --> pause: Escape
  pause --> play: "Resume or cancel"
  pause --> mainMenu: Menu
  play --> gameOver: Died
  gameOver --> mainMenu: "Menu or cancel"
  quitApp --> [*]
```

High scores (`tappy.scores`, capacity 5) update on game over. Art prefers atlas sprites and falls back to colored rects.
