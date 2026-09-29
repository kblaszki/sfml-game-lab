---
name: add-scene
description: >-
  Adds a SceneI implementation under games/<name>/app, wires it into that
  game's app CMakeLists and SceneStack factories. Use when introducing or
  extending game scenes (menu, gameplay, pause, overlays) for arkanoid, tetris,
  syndicate, or a new game.
---

# Add a scene

Source of truth: [docs/how-to/add-a-scene.md](../../../docs/how-to/add-a-scene.md), [docs/reference/engine-scene.md](../../../docs/reference/engine-scene.md), [docs/reference/pause-overlay.md](../../../docs/reference/pause-overlay.md), and the scene lifecycle section of [docs/explanation/architecture.md](../../../docs/explanation/architecture.md). For game-specific scenes, read `games/<name>/docs/reference/app.md`.

Concrete game scenes live in `games/<name>/app/` (`namespace sgl::<name>`), not in `engine/scene/`. The engine provides `SceneI`, `SceneStack`, `SceneRequest`, `SceneContext`, and `SceneTraits`, plus reusable scenes in `sgl_ui` (`PauseOverlay`, `SettingsScene`). A brand-new title follows [create_new_game.md](../../../docs/tutorials/create_new_game.md).

## Checklist

```
- [ ] Create FooScene.hpp / FooScene.cpp under games/<name>/app/
- [ ] Implement sgl::SceneI (update, render const, traits)
- [ ] Add src/FooScene.cpp to games/<name>/app/CMakeLists.txt
- [ ] Expose a SceneFactory from Scenes.hpp / Scenes.cpp if other scenes create it
- [ ] Cover with a windowless test under tests/<name>/app/ (SceneStack + InputState)
- [ ] Update games/<name>/docs/reference/app.md (+ state diagram in explanation if flow changed)
- [ ] Update docs/reference/source-layout.md if a new suite or file was added
```

## Steps

1. **Traits.** Set `opaque`, `blocksUpdate` and `pausable`. A pause or result overlay uses `opaque` false, `blocksUpdate` true, `pausable` false. Gameplay is `pausable` only while playing.

2. **Requests.** Use `ctx.request(PushScene|PopScene|ReplaceScene|RequestPause|QuitApp)`. Requests apply after the update walk, in order. To leave an overlay to the menu, send `PopScene` and then `ReplaceScene`. Never close the platform from a scene.

3. **Pause.** Only `RequestPause` (or focus loss) pushes the stack's pause factory. Do not push a pause scene ad hoc.

4. **CMake.** List the new `.cpp` only in `games/<name>/app/CMakeLists.txt`.

5. **Tests.** Extend that game's scenes suite or add a suite with `add_unit_test`. No window. See [write-headless-tests.md](../../../docs/how-to/write-headless-tests.md).

Do not invent `MenuScreen`, `ScreenStack`, `ScreenI` or `SceneManager`; those names do not exist. Use `SceneI` / `SceneStack`.
