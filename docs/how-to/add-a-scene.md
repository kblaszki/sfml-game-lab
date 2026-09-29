---
title: Add a scene and scene transitions
diataxis: how-to
audience: [ai, human]
related_code:
  - engine/scene/include/sgl/scene/SceneI.hpp
  - engine/scene/include/sgl/scene/SceneTraits.hpp
  - engine/scene/include/sgl/scene/SceneRequest.hpp
  - engine/scene/include/sgl/scene/SceneContext.hpp
  - engine/scene/include/sgl/scene/SceneStack.hpp
  - engine/scene/src/SceneStack.cpp
  - engine/ui/include/sgl/ui/MenuModel.hpp
  - engine/ui/include/sgl/ui/RenderMenu.hpp
  - engine/ui/include/sgl/ui/PauseOverlay.hpp
  - games/arkanoid/app/src/PauseScene.cpp
  - games/arkanoid/app/src/ResultScene.cpp
  - games/tetris/app/src/MainMenuScene.cpp
related_docs:
  - ../tutorials/create_new_game.md
  - write-headless-tests.md
  - ../reference/engine-scene.md
  - ../reference/pause-overlay.md
  - ../explanation/architecture.md
keywords: [scene, SceneI, SceneTraits, PushScene, PopScene, ReplaceScene, RequestPause, QuitApp, SceneFactory, menu, overlay, transition]
last_reviewed: 2026-09-29
---

# Add a scene and scene transitions

Game scenes live in `games/<game>/app/` in `namespace sgl::<game>`, never in `engine/scene/`. The engine supplies `SceneI`, `SceneTraits`, `SceneRequest`, `SceneContext` and `SceneStack`. For background, read the scene lifecycle section of [architecture.md](../explanation/architecture.md#scene-lifecycle).

## 1. Choose the traits

| Scene kind | `opaque` | `blocksUpdate` | `pausable` |
|------------|----------|----------------|------------|
| Menu, setup, full-screen scene | true | true | false |
| Gameplay | true | true | true while playing, false after game over |
| Overlay over gameplay (pause, result, help, settings) | false | true | false |
| HUD-like layer that lets the scene below keep running | false | false | false |

`opaque` false lets the scenes below keep drawing. `blocksUpdate` true freezes them. `pausable` is checked on the **top** scene when a `RequestPause` arrives or the window loses focus.

## 2. Implement `SceneI`

1. Add `FooScene.hpp` under `games/<game>/app/include/<game>/app/` and `FooScene.cpp` under `games/<game>/app/src/`.
2. Implement `update(SceneContext&, Seconds)`, `render(RenderQueue&) const` and `traits() const`.
3. List `src/FooScene.cpp` in `games/<game>/app/CMakeLists.txt`.
4. Expose a factory so other scenes can create it, following `mainMenu(services)` / `play(services)` in each game's `Scenes.hpp`. The factory returns `sgl::SceneFactory`, which is `std::move_only_function<std::unique_ptr<SceneI>()>`.

This menu scene is compiled and tested against the engine. It uses `MenuModel` for focus and keyboard or mouse activation, and `renderMenu` for drawing:

```cpp
class MenuScene final : public sgl::SceneI
{
public:
    MenuScene(Actions actions, sgl::FontId font)
        : actions_{actions}
        , font_{font}
        , model_{{sgl::Button{.rect = {{540.f, 300.f}, {200.f, 48.f}}, .label = "Play"},
                  sgl::Button{.rect = {{540.f, 364.f}, {200.f, 48.f}}, .label = "Quit"}}}
    {
    }

    void update(sgl::SceneContext& ctx, sgl::Seconds /*dt*/) override
    {
        if(const auto chosen = model_.update(ctx.input(), menuActions(actions_)))
        {
            if(*chosen == 0)
            {
                ctx.request(sgl::ReplaceScene{play(actions_, font_, 42)});
            }
            else
            {
                ctx.request(sgl::QuitApp{});
            }
            return;
        }
        if(model_.cancelled())
        {
            ctx.request(sgl::QuitApp{});
        }
    }

    void render(sgl::RenderQueue& queue) const override
    {
        const sgl::MenuStyle style{.font = font_,
                                   .textSize = 32,
                                   .idle = sgl::Color{40, 44, 60, 255},
                                   .focused = sgl::Color{90, 200, 255, 255},
                                   .text = sgl::Color{240, 240, 240, 255},
                                   .panel = sgl::Color{16, 16, 32, 255},
                                   .cornerRadius = 8.f};
        sgl::renderMenu(queue, model_, style, sgl::Layer::Hud);
    }

    [[nodiscard]] sgl::SceneTraits traits() const override
    {
        return sgl::SceneTraits{.opaque = true, .blocksUpdate = true, .pausable = false};
    }

private:
    Actions actions_;
    sgl::FontId font_;
    sgl::MenuModel model_;
};
```

## 3. Pick the transition

Always send a request through `ctx.request(...)`. Requests queue during the update walk and are applied afterwards, in the order they were sent.

| Goal | Request(s) | Example in the repo |
|------|------------|---------------------|
| Go from menu to gameplay | `ReplaceScene{play(...)}` | `games/tetris/app/src/MainMenuScene.cpp` |
| Open an overlay (result, help, settings) | `PushScene{factory}` | `GameplayScene` pushes `result(...)` in Arkanoid |
| Close the overlay | `PopScene{}` | `PauseOverlay` Resume |
| Leave an overlay to the menu | `PopScene{}` then `ReplaceScene{mainMenu(...)}` | `games/arkanoid/app/src/PauseScene.cpp` |
| Restart gameplay from an overlay | `PopScene{}` then `ReplaceScene{play(...)}` | `games/arkanoid/app/src/ResultScene.cpp` |
| Pause | `RequestPause{}` (ignored unless the top scene is `pausable`) | gameplay scenes on the pause action |
| Quit | `QuitApp{}` | menu cancel |

Rules:

- A scene never calls `platform.close()`. Quitting is `QuitApp`.
- Do not push a pause scene yourself. The stack's pause factory, passed to `SceneStack{...}` in `main.cpp`, is the only pause, and `RequestPause` or focus loss triggers it. See [pause-overlay.md](../reference/pause-overlay.md).
- Menu cancel quits the app. Overlay cancel resumes (`PopScene`).
- Capture what the next scene needs by value in the factory lambda. The requesting scene may be destroyed before the factory runs.

## 4. Test it headlessly

Build a `SceneStack`, push the scene, feed `InputState` with `apply(KeyDown{...}, map)`, call `stack.update(input, sgl::kTick)`, then assert `stack.size()`, `stack.top()` or `stack.quitRequested()`. Recipes are in [write-headless-tests.md](write-headless-tests.md).

## 5. Update docs

Add the scene to `games/<game>/docs/reference/app.md`, including its `related_code` list. If it changes the game's flow, update the state diagram in that game's explanation page.
