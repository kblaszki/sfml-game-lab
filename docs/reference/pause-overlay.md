---
title: Pause overlay
diataxis: reference
audience: [ai, human]
related_code:
  - engine/scene/include/sgl/scene/SceneTraits.hpp
  - engine/scene/include/sgl/scene/SceneRequest.hpp
  - engine/scene/include/sgl/scene/SceneStack.hpp
  - engine/scene/src/SceneStack.cpp
  - games/arkanoid/app/include/arkanoid/app/PauseScene.hpp
  - games/arkanoid/app/src/PauseScene.cpp
  - games/arkanoid/app/include/arkanoid/app/GameplayScene.hpp
  - games/arkanoid/app/src/GameplayScene.cpp
  - games/arkanoid/app/include/arkanoid/app/ResultScene.hpp
  - games/arkanoid/app/src/ResultScene.cpp
  - games/arkanoid/app/include/arkanoid/app/Scenes.hpp
  - tests/engine/scene/SceneStackTest.cpp
  - tests/arkanoid/app/ScenesTest.cpp
  - tests/arkanoid/app/ResultSceneTest.cpp
related_docs:
  - engine-scene.md
  - engine-input.md
  - input-and-events.md
  - arkanoid-app.md
  - source-layout.md
  - ../../mvp/05-pause.md
keywords: [pause, overlay, PauseScene, PauseMenuScene, PauseOverlay, ResultScene, RequestPause, SceneTraits, pausable, FocusLost, SceneStack]
last_reviewed: 2026-09-29
---

# Pause overlay

Facts about the running tree. Engine stack rules: [engine-scene.md](engine-scene.md). Prospective extras stay in [`mvp/05-pause.md`](../../mvp/05-pause.md).

## What pause is

Paused state **is** “a pause scene is top of `SceneStack`”. It is not a `bool` on the sim `State`, not a flag on gameplay, and not `timeScale = 0`.

Arkanoid uses `PauseScene` / shared `sgl::PauseOverlay` patterns; Tetris uses its own `PauseScene`. **Syndicate** pause factory returns `PauseMenuScene` (Resume / Settings / Help / Quit), not `sgl::PauseOverlay`.

| Trait / query | Pause overlay | Meaning |
|---------------|---------------|---------|
| `opaque` | `false` | Gameplay still renders underneath |
| `blocksUpdate` | `true` | Stack stops the update walk; gameplay `update` (and sim step) do not run |
| `pausable` | `false` | A second pause does not stack |

`ResultScene` uses the same overlay traits (`opaque` false, `blocksUpdate` true, not pausable) for win/lose; see [arkanoid-app.md](arkanoid-app.md).

## How it is requested

`SceneRequest::RequestPause` is the only path that pushes the pause-overlay factory. `SceneStack::update` enqueues it when:

- `input.focusLost()` is set, or a scene explicitly `request(RequestPause)`
- top scene `traits().pausable` is true
- a pause was not already queued this frame

`GameplayScene` is pausable only while playing (`!cleared && !over`). Menu, pause, and result are not pausable. No-op when the stack is empty or top is not pausable.

## Resume vs quit

| Input on Arkanoid/Tetris overlay | Requests |
|----------------------------------|----------|
| Pause or Cancel | `PopScene` → back to gameplay |
| Confirm | `PopScene` then `ReplaceScene{mainMenu}` (game-specific) |

Syndicate `PauseMenuScene`: Resume / cancel → `PopScene`; Settings → `PushScene` `SettingsScene`; Help → `PushScene` `HelpScene`; Quit → `QuitApp`.

`FocusGained` does not resume. Focus lost is an `InputEvent`, not a remappable action; see [engine-input.md](engine-input.md).
