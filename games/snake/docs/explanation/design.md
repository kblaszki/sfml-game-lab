---
title: Snake scene flow
diataxis: explanation
audience: [ai, human]
related_code:
  - games/snake/app/include/snake/app/Sounds.hpp
  - games/snake/app/include/snake/app/Feedback.hpp
  - games/snake/app/include/snake/app/Scenes.hpp
  - games/snake/app/include/snake/app/MainMenuScene.hpp
  - games/snake/app/include/snake/app/PlayScene.hpp
  - games/snake/app/include/snake/app/PauseScene.hpp
  - games/snake/app/include/snake/app/GameOverScene.hpp
  - games/snake/app/src/PlayScene.cpp
  - games/snake/app/src/PauseScene.cpp
  - games/snake/app/src/GameOverScene.cpp
  - games/snake/main.cpp
related_docs:
  - ../reference/app.md
  - ../reference/sim.md
  - ../index.md
  - ../../../../docs/explanation/architecture.md
  - ../../../../docs/reference/pause-overlay.md
keywords: [snake, scenes, menu, pause, game over, SceneStack, RequestPause]
last_reviewed: 2026-11-01
---

# Snake scene flow

The window is a consumer of `SnakeGame`. Movement, food, score, and death stay in `snake_sim` ([sim](../reference/sim.md)). Scenes map `InputState` to `SimInput`, call `step`, and turn `body()`, `food()`, and `score()` into draw commands. They do not move the snake themselves.

`main` owns the action map, the font id, the seed, the audio device, the sound ids, the scores path, and the high-score table for the whole `App::run()`. Factories copy `AppServices` by value; the references inside it (`SnakeActions`, `AudioI`, `HighScoreTable`) must outlive the stack.

```mermaid
stateDiagram-v2
  [*] --> mainMenu
  mainMenu --> play: "Play"
  mainMenu --> [*]: "Quit or cancel"
  play --> pause: "RequestPause"
  pause --> play: "Resume or cancel"
  pause --> mainMenu: "Menu"
  play --> gameOver: "Died"
  gameOver --> mainMenu: "Menu or cancel"
  gameOver --> [*]: "Quit"
```

Play replaces the menu, so a run is a single scene until pause or death. Pause is an overlay: `RequestPause` (Escape, P, or focus loss) asks the stack to push the pause factory. Resume, the pause key, and cancel pop that overlay and leave the same `SnakeGame` in place. Menu pops the overlay and then replaces play with the main menu, so the abandoned run is not left under the menu.

Death replaces play with game over. The sim has already stopped (`over()` and a `Died` event). `PlayScene` feeds that step's events to `Feedback` before the replace, so the death blip plays on `Died` and the eat blip plays on `Ate`. Game over inserts the sim score into the high-score table (capacity 5), shows the ranking lines, and saves them. A failed save stays on the scene. Menu or cancel returns to the main menu; Quit leaves the app.
