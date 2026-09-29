---
title: Engine API map (concept to real type)
diataxis: reference
audience: [ai, human]
related_code:
  - engine/core/include/sgl/core/Vec2.hpp
  - engine/core/include/sgl/core/Rect.hpp
  - engine/core/include/sgl/core/Color.hpp
  - engine/core/include/sgl/core/Handle.hpp
  - engine/core/include/sgl/core/Random.hpp
  - engine/core/include/sgl/core/Time.hpp
  - engine/input/include/sgl/input/InputEvent.hpp
  - engine/input/include/sgl/input/ActionMap.hpp
  - engine/input/include/sgl/input/InputState.hpp
  - engine/scene/include/sgl/scene/SceneI.hpp
  - engine/scene/include/sgl/scene/SceneStack.hpp
  - engine/scene/include/sgl/scene/SceneRequest.hpp
  - engine/scene/include/sgl/scene/SceneContext.hpp
  - engine/loop/include/sgl/loop/App.hpp
  - engine/render/include/sgl/render/RenderQueue.hpp
  - engine/resources/include/sgl/resources/ResourceCache.hpp
  - engine/sfml/include/sgl/sfml/SfmlAssets.hpp
related_docs:
  - engine-core.md
  - engine-input.md
  - engine-scene.md
  - engine-loop.md
  - engine-render.md
  - engine-resources.md
  - engine-sfml.md
  - ../explanation/architecture.md
keywords: [api map, glossary, SceneManager, InputManager, ResourceManager, EventManager, Core, SceneStack, InputState, ResourceCache]
last_reviewed: 2026-09-29
---

# Engine API map

Generic engine terms and the real `sgl` types that fill each role. Types without a namespace below are in `sgl`. Search the code for the real names; the generic ones do not exist.

| Generic term | Real type(s) | Header | Reference |
|--------------|--------------|--------|-----------|
| Core | `Vec2<T>` / `Vec2f`, `Rect<T>`, `Color`, `Handle<Tag>`, `Pcg32`, `Seconds`, `kTick`, `Overloaded`, `Image` | `<sgl/core/*.hpp>` (target `sgl_core`, header only) | [engine-core.md](engine-core.md) |
| Scene | `SceneI` (`update`, `render`, `traits`) and `SceneTraits` (`opaque`, `blocksUpdate`, `pausable`) | `<sgl/scene/SceneI.hpp>` | [engine-scene.md](engine-scene.md) |
| Scene manager | `SceneStack`, changed only through `SceneRequest` values sent with `SceneContext::request` | `<sgl/scene/SceneStack.hpp>`, `<sgl/scene/SceneRequest.hpp>` | [engine-scene.md](engine-scene.md) |
| Scene transition | `PushScene{factory}`, `PopScene{}`, `ReplaceScene{factory}`, `RequestPause{}`, `QuitApp{}` | `<sgl/scene/SceneRequest.hpp>` | [engine-scene.md](engine-scene.md) |
| Input manager | `InputState` (per-frame `ActionState` and axes, pointer) plus `ActionMap` (key to action bindings) | `<sgl/input/InputState.hpp>`, `<sgl/input/ActionMap.hpp>` | [engine-input.md](engine-input.md) |
| Raw input event | `InputEvent` variant: `KeyDown`, `KeyUp`, `MouseMove`, `MouseDown`, `MouseUp`, `WindowClosed`, `FocusLost`, `FocusGained` | `<sgl/input/InputEvent.hpp>` | [engine-input.md](engine-input.md) |
| Event manager / event bus | None. See [Events](#events) below | | [input-and-events.md](input-and-events.md) |
| Resource manager | `ResourceCache<Id, T>` (keyed, stable addresses, `std::expected` loads), owned by `sfml::SfmlAssets` (textures, fonts) and `sfml::SfmlAudio` (sound buffers) | `<sgl/resources/ResourceCache.hpp>`, `<sgl/sfml/SfmlAssets.hpp>` | [engine-resources.md](engine-resources.md) |
| Sprite sheet | `Atlas` / `parseAtlas` (text manifest of named rects) | `<sgl/resources/Atlas.hpp>` | [engine-resources.md](engine-resources.md) |
| Application / main loop | `App` (`run`, `runFrames`) with `FixedStepLoop` | `<sgl/loop/App.hpp>` | [engine-loop.md](engine-loop.md) |
| Window / platform | `PlatformI`, implemented by `sfml::SfmlPlatform` | `<sgl/loop/PlatformI.hpp>` | [engine-loop.md](engine-loop.md), [engine-sfml.md](engine-sfml.md) |
| Renderer | `RendererI` consuming a `RenderQueue` of `DrawCommand` values | `<sgl/render/RendererI.hpp>`, `<sgl/render/RenderQueue.hpp>` | [engine-render.md](engine-render.md) |
| Sound manager | `AudioI::play(SoundId, volume)`, `ScaledAudio`, and PCM via `tone` / `blip` | `<sgl/audio/*.hpp>` | [engine-audio.md](engine-audio.md) |
| UI widgets | `MenuModel` + `renderMenu`, `TooltipSystem`, `HoverAnim`, `TextLayout`, `PauseOverlay`, `SettingsScene` | `<sgl/ui/*.hpp>` | [engine-ui.md](engine-ui.md) |
| Tweening | `Easing`, `Tween`, Catmull-Rom spline, `ArcLengthTable` | `<sgl/anim/*.hpp>` | [engine-anim.md](engine-anim.md) |
| Persistence | `HighScoreTable` / `ScoreStore`, `Settings` / `SettingsStore` | `<sgl/save/*.hpp>` | [engine-save.md](engine-save.md) |

## Events

There is no central event manager or observer registry. An event is a value in one of three channels, and each channel has exactly one producer and one consumer:

| Channel | Type | Producer to consumer | Lifetime |
|---------|------|----------------------|----------|
| Platform input | `InputEvent` | `PlatformI::poll()` to `InputState::apply` inside `App` | folded into `InputState` for the frame |
| Scene changes | `SceneRequest` | a scene's `ctx.request(...)` to `SceneStack` | queued, applied after the update walk |
| Game events | the sim's own type, for example Arkanoid `SimEvent` or Syndicate `GameEvent` | the sim step or `Match::apply` to the game's app layer | returned from `step`, or drained with `drainEvents()`, every tick |

Game events stay game-specific on purpose. The engine never needs to know what a brick or a dividend is, so it offers no generic event type for them.
