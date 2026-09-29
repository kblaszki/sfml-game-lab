---
name: add-scene
description: >-
  Adds a SceneI implementation under games/<name>/app, wires it into that
  game's app CMakeLists and SceneStack factories. Use when introducing or
  extending game scenes (menu, gameplay, pause, overlays) for arkanoid or tetris.
---

# Add a scene

Source of truth: [docs/reference/engine-scene.md](../../../docs/reference/engine-scene.md), [games/arkanoid/docs/reference/app.md](../../../games/arkanoid/docs/reference/app.md), [games/tetris/docs/reference/app.md](../../../games/tetris/docs/reference/app.md), [docs/how-to/add-a-game.md](../../../docs/how-to/add-a-game.md), [docs/explanation/architecture.md](../../../docs/explanation/architecture.md).

Concrete game scenes live in `games/<name>/app/` (`namespace sgl::arkanoid` or `sgl::tetris`), not in `engine/scene/`. A brand-new title follows [add-a-game](../../../docs/how-to/add-a-game.md). Engine provides `SceneI`, `SceneStack`, `SceneRequest`, `SceneTraits`.

## Checklist

```
- [ ] Create FooScene.hpp / FooScene.cpp under games/<name>/app/
- [ ] Implement sgl::SceneI (update, render, traits)
- [ ] Add FooScene.cpp to games/arkanoid/app/CMakeLists.txt
- [ ] Expose a SceneFactory from Scenes.hpp / Scenes.cpp if the stack needs it
- [ ] Cover with a windowless test under tests/<name>/app/
- [ ] Update that game's docs/reference page and source-layout.md if structure changed
```

## Steps

1. **Traits** — set `opaque`, `blocksUpdate`, `pausable` appropriately. Pause overlay: `opaque` false, `blocksUpdate` true, `pausable` false. Gameplay: `pausable` only while playing.

2. **Requests** — use `ctx.request(PushScene|PopScene|ReplaceScene|RequestPause|QuitApp)`. Never close the platform from a scene.

3. **Pause** — only `RequestPause` pushes the pause factory; do not push `PauseScene` ad hoc. See [pause-overlay.md](../../../docs/reference/pause-overlay.md).

4. **CMake** — list the new `.cpp` in `games/<name>/app/CMakeLists.txt` only.

5. **Tests** — extend that game's scenes suite or add a suite; no window.

Do not invent `MenuScreen` / `ScreenStack` / `ScreenI` — those names are gone. Use `SceneI` / `SceneStack`.
