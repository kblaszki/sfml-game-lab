---
title: C++23, ownership and zero-cost abstractions in sgl
diataxis: explanation
audience: [ai, human]
related_code:
  - engine/core/include/sgl/core/Handle.hpp
  - engine/core/include/sgl/core/Overloaded.hpp
  - engine/core/include/sgl/core/Random.hpp
  - engine/core/include/sgl/core/Time.hpp
  - engine/render/include/sgl/render/AssetIds.hpp
  - engine/render/include/sgl/render/DrawCommand.hpp
  - engine/resources/include/sgl/resources/ResourceCache.hpp
  - engine/scene/include/sgl/scene/SceneRequest.hpp
  - engine/scene/include/sgl/scene/SceneStack.hpp
  - engine/scene/src/SceneStack.cpp
  - engine/sfml/include/sgl/sfml/SfmlPlatform.hpp
  - engine/sfml/src/SfmlRenderer.cpp
  - games/syndicate/sim/include/syndicate/sim/Money.hpp
related_docs:
  - architecture.md
  - sfml-integration.md
  - ../reference/engine-core.md
  - ../reference/engine-resources.md
  - ../reference/engine-scene.md
keywords: [C++23, RAII, ownership, unique_ptr, references, expected, variant, visit, concepts, move_only_function, zero-cost, Handle, strong types]
last_reviewed: 2026-09-29
---

# C++23, ownership and zero-cost abstractions in sgl

[architecture.md](architecture.md) lists the allowed and forbidden C++23 features. This page explains why, using the code that already follows those rules.

## Ownership is visible in the type

There is no raw `new` or `delete` anywhere in `engine/` or `games/`. Each kind of relationship has a single spelling:

| Relationship | Spelling | Example |
|--------------|----------|---------|
| Exclusive ownership of a polymorphic object | `std::unique_ptr<T>` | `SceneStack` holds `std::vector<std::unique_ptr<SceneI>>` |
| Ownership of a value | member by value | `App` holds `InputState`, `FixedStepLoop` and `ActionMap` |
| Borrow for the object's lifetime | reference member `T&` | `App` holds `PlatformI&`, `ClockI&`, `SceneStack&`, `RenderQueue&` |
| Optional, non-owning lookup | `const T*` that may be null | `ResourceCache::get(id)`, `SceneStack::top()` |
| Deferred construction | `std::move_only_function<std::unique_ptr<SceneI>()>` | `SceneFactory` in `PushScene` / `ReplaceScene` |

A raw pointer in this codebase never owns anything. It means "may be absent, do not delete". Anything that would need a nullable owner uses `std::optional<T>` or `std::unique_ptr<T>` instead.

`main.cpp` is the composition root. It creates the platform, clock, stack and queue on its own stack frame, then hands references to `sgl::App`. Since `App::run()` returns before `main` does, every borrow is outlived by its owner, with no shared ownership and no reference counting.

## RAII and destruction order

Resource lifetime follows C++ scope. The sharpest case is `SfmlPlatform` ([SfmlPlatform.hpp](../../engine/sfml/include/sgl/sfml/SfmlPlatform.hpp)). Its members are declared in this order: window, assets, audio, then renderer. Members are destroyed in reverse declaration order, so the renderer (which references the window and the assets) goes first, then the audio voices, then the textures and fonts. The window and its OpenGL context are destroyed last, after every GPU resource that needs them. Declaration order is the lifetime contract, so do not reorder these members.

The same reasoning is behind the ban on function-local `static` textures or sound buffers. Those would be destroyed after `main` returns, when the GL context or audio device is already gone.

`SceneStack` is move-only: copying is deleted and moving is defaulted. Duplicating a stack of uniquely owned scenes has no sensible meaning, so the type does not allow it.

## Errors are values

Loading can fail, and the failure belongs to the caller. `ResourceCache::load`, `SfmlAssets::font` and `SfmlAudio::upload` return `std::expected<Id, ResourceError>`. `main` checks the result and exits with a message; nothing throws. Absence that is not an error (a cache miss, or an optional atlas texture) is `std::optional`.

Exceptions are not used for control flow. The simulation has no failure path beyond "the command is not legal", and that is data: Syndicate's `legalCommands()` lists what may be applied.

## Closed sets are variants

When the set of alternatives is closed and owned by the engine, the code uses `std::variant` rather than a class hierarchy:

- `InputEvent`: key, mouse, focus and window events.
- `SceneRequest`: `PushScene`, `PopScene`, `ReplaceScene`, `RequestPause`, `QuitApp`.
- `DrawCommand`: `SpriteCmd`, `RectCmd`, `TextCmd`, `LineCmd`, `CircleCmd`, `MeshCmd`.
- Game events and timed effects in each sim.

Variants are values. They are stored contiguously in a `std::vector`, sorted, copied into tests and compared, with no heap allocation per command. Dispatch is `std::visit`, either with `sgl::Overloaded` ([Overloaded.hpp](../../engine/core/include/sgl/core/Overloaded.hpp)) or with an `if constexpr` chain, as in `SfmlRenderer::submit`. Adding an alternative then makes every non-generic visitor fail to compile until it handles the new case, which is the review we want.

Virtual functions are reserved for **ports**, where the implementation lives outside the headless code: `SceneI`, `RendererI`, `PlatformI`, `ClockI`, `AudioI`. That gives one virtual call per scene per tick, and one per frame into the renderer. It is never paid per sprite.

## Strong types cost nothing

`sgl::Handle<Tag>` ([Handle.hpp](../../engine/core/include/sgl/core/Handle.hpp)) is a `std::uint32_t` wrapped in a tagged struct. `TextureId` and `FontId` are distinct types with the same layout, so passing a font where a texture is expected fails to compile. The generated code is identical to passing a bare integer. `ActionId`, `AxisId` and `SoundId` follow the same idea.

Syndicate's `Money` ([Money.hpp](../../games/syndicate/sim/include/syndicate/sim/Money.hpp)) is a `std::int64_t` with `constexpr` arithmetic operators and a defaulted `<=>`. Integer money makes the economy exact and identical on every compiler. The wrapper stops a share count from being added to a price by accident.

## Concepts document template contracts

`ResourceCache<Id, T>::load` accepts any loader that satisfies `ResourceLoader<Loader, T>`: a callable returning `std::expected<T, ResourceError>`. A wrong loader is rejected at the call site with a readable diagnostic rather than deep inside the template. The cache keeps each `T` in a `std::unique_ptr` so addresses stay stable while the vector grows. `SfmlRenderer` can then hold `const sf::Texture*` results across frames.

## Determinism is a design constraint

Fixed `kTick` steps, integer or plain-float state, and `sgl::Pcg32` ([Random.hpp](../../engine/core/include/sgl/core/Random.hpp)) make a sim a pure function of (seed, input sequence). `Pcg32` is header-only, `constexpr`-friendly and `noexcept`. Its `uniformBelow` uses rejection sampling, so it is unbiased and gives the same stream under GCC, Clang and MSVC. That is why the standard distributions are banned in sim code: their output is implementation-defined.

## What "zero-cost" means here

The engine avoids costs that buy nothing in a small 2D game:

- no per-object heap allocation in the hot loop
- no virtual dispatch per draw
- no hidden reference counting
- no exceptions unwinding across frames
- no type erasure in the sim

`std::move_only_function` appears only where a callable really has to be stored: the `SceneFactory` in scene requests and pause overlays. `std::span` passes contiguous views without copying, for example `RenderQueue::sorted()` and `ActionMap::actionsFor()`.
