---
title: Engine input
diataxis: reference
audience: [ai, human]
related_code:
  - engine/input/include/sgl/input/Key.hpp
  - engine/input/include/sgl/input/MouseButton.hpp
  - engine/input/include/sgl/input/InputEvent.hpp
  - engine/input/include/sgl/input/ActionId.hpp
  - engine/input/include/sgl/input/ActionState.hpp
  - engine/input/include/sgl/input/ActionMap.hpp
  - engine/input/include/sgl/input/InputState.hpp
  - engine/input/src/ActionMap.cpp
  - engine/input/src/InputState.cpp
  - engine/input/CMakeLists.txt
  - tests/engine/input/InputStateTest.cpp
  - tests/engine/input/CMakeLists.txt
related_docs:
  - source-layout.md
  - engine-core.md
keywords: [eng, sgl_input, Key, MouseButton, InputEvent, ActionMap, InputState, ActionId, AxisId, ActionState]
last_reviewed: 2026-09-29
---

# Engine input

SFML-free input types in `namespace sgl`, included as `<sgl/input/X.hpp>`. Target `sgl_input` is a STATIC library that PUBLIC-links `sgl_core`.

## Keys and events

`Key` enumerators include letters used by gameplay (`A`, `D`, `W`, `S`, `Z`, `X`, `C`, `P`), modifiers (`LShift`), navigation (`Left`/`Right`/`Up`/`Down`), and UI (`Enter`, `Escape`, `Backspace`, `Space`), plus `Unknown` as the last enumerator (so `keyIndex` stays valid). `MouseButton` is `Left`, `Right`, `Middle`.

`InputEvent` is a `std::variant` of:

| Alternative | Payload |
|-------------|---------|
| `KeyDown` / `KeyUp` | `Key key` |
| `MouseMove` | `Vec2f pos` |
| `MouseDown` / `MouseUp` | `MouseButton button`, `Vec2f pos` |
| `WindowClosed` | empty |
| `FocusLost` / `FocusGained` | empty |

## Handles and binding

- `ActionId` = `Handle<ActionTag>`; `AxisId` = `Handle<AxisTag>`
- `ActionMap::bind(key, action)` — one key may map to several actions (small vector per key); several keys may bind to one action
- `ActionMap::bindAxis(negative, positive, axis)` — digital axis from two keys
- `ActionMap::actionsFor(key)` — `std::span<const ActionId>` (empty when unbound)
- `ActionMap::keysFor(action)` — all keys bound to that action
- `ActionMap::axes()` — span of `{id, negative, positive}` for `InputState` evaluation

## Frame state

`ActionState` has `pressed`, `held`, and `released` edges/levels.

`InputState`:

- `beginFrame()` — clears `pressed`, `released`, `pointerPressed`, `pointerReleased`, `closeRequested`, and `focusLost` only; `held` stays
- `apply(event, map)` — `std::visit` over `InputEvent`
- `action(id)` / `axis(id)` / `pointer()` / `pointerPressed()` / `pointerReleased()` / `closeRequested()` / `focusLost()`

Semantics:

- Unbound keys leave action state unchanged (still update axis key holds when bound as axis ends)
- `KeyDown` sets `pressed` and `held`
- `KeyUp`: `held` stays true while any other bound key for that action is still down; `released` fires only when the last bound key goes up
- Same-frame `KeyDown` then `KeyUp` (before the next `beginFrame`) yields `pressed && released && !held` — both edges latch for the frame; `held` clears on the release
- Both axis keys held → `0`; only negative → `-1`; only positive → `+1`
- `FocusLost` sets the focus-lost edge and clears all held keys/actions (axes become `0`)
- `WindowClosed` sets `closeRequested` until the next `beginFrame`
- `MouseMove` / `MouseDown` / `MouseUp` set `pointer()`; left `MouseDown` / `MouseUp` also set `pointerPressed` / `pointerReleased` edges
- `FocusGained` is a no-op at this layer
