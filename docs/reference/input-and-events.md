---
title: Input and events
diataxis: reference
audience: [ai, human]
related_code:
  - engine/input/include/sgl/input/InputEvent.hpp
  - engine/input/include/sgl/input/ActionMap.hpp
  - engine/input/include/sgl/input/InputState.hpp
  - engine/input/include/sgl/input/ActionId.hpp
  - engine/input/include/sgl/input/Key.hpp
  - engine/input/src/ActionMap.cpp
  - engine/input/src/InputState.cpp
  - games/arkanoid/app/include/arkanoid/app/Bindings.hpp
  - games/arkanoid/app/src/Bindings.cpp
  - tests/engine/input/InputStateTest.cpp
related_docs:
  - engine-input.md
  - engine-loop.md
  - engine-sfml.md
  - pause-overlay.md
  - ../../games/arkanoid/docs/reference/app.md
  - source-layout.md
  - ../tutorials/getting-started.md
  - ../../mvp/03-events-and-input.md
keywords: [input, InputEvent, ActionMap, InputState, defaultBindings, confirm, pause, cancel, paddle, Key, Num1, H]
last_reviewed: 2026-09-29
---

# Input and events

Facts about the running tree. Engine types: [engine-input.md](engine-input.md). Frame pump: [engine-loop.md](engine-loop.md) (`App` drains `PlatformI::poll` into `InputState`). SFML → `InputEvent` mapping: [engine-sfml.md](engine-sfml.md). Overlay pause: [pause-overlay.md](pause-overlay.md). Prospective extras stay in [`mvp/03-events-and-input.md`](../../mvp/03-events-and-input.md).

## Engine types (summary)

| Type | Role |
|------|------|
| `sgl::InputEvent` | Variant: `KeyDown` / `KeyUp`, mouse move/down/up, `WindowClosed`, `FocusLost` / `FocusGained` |
| `sgl::Key` | Includes `H`, `Num1`–`Num9` (top-row digits), gameplay letters, arrows, Enter/Escape/Backspace/Space/LShift; `Unknown` last |
| `sgl::ActionMap` | Binds keys to `ActionId`s and key pairs to an `AxisId`; `keysFor` lists keys per action |
| `sgl::InputState` | Per-frame edges (`pressed` / `released`, pointer press/release), holds, axes, pointer, `closeRequested`, `focusLost` |

Scenes read `SceneContext::input()`; they never poll the window and never call `close()`. Action `held` stays true while any bound key is down; `released` fires when the last bound key goes up. `App` keeps edges across zero-step frames (see [engine-loop.md](engine-loop.md)).

## Arkanoid default bindings

`sgl::arkanoid::makeActions()` builds ids 1–4. `sgl::arkanoid::defaultBindings` fills the map:

| Physical input | Binding |
|----------------|---------|
| Enter | `confirm` |
| Escape | `pause` |
| Backspace | `cancel` |
| Left / Right | `paddle` axis (−1 / +1) |
| A / D | `paddle` axis (−1 / +1) |

Menu: confirm starts Stage1; cancel quits the app (`QuitApp`). Gameplay: confirm launches; paddle axis moves the paddle; pause requests the overlay. Overlay: pause/cancel resume; confirm quits to menu. See [games/arkanoid/docs/reference/app.md](../../games/arkanoid/docs/reference/app.md).
