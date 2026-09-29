---
title: Write headless GoogleTest tests
diataxis: how-to
audience: [ai, human]
related_code:
  - cmake/AddUnitTest.cmake
  - tests/CMakeLists.txt
  - tests/mocks/PlatformMock.hpp
  - tests/mocks/ClockMock.hpp
  - tests/mocks/RendererMock.hpp
  - tests/mocks/AudioSpy.hpp
  - tests/engine/scene/fakes/SceneSpy.hpp
  - tests/engine/loop/AppTest.cpp
  - tests/tetris/app/ScenesTest.cpp
  - engine/loop/include/sgl/loop/App.hpp
  - engine/input/include/sgl/input/InputState.hpp
  - engine/render/include/sgl/render/RenderQueue.hpp
related_docs:
  - build-and-test.md
  - add-a-scene.md
  - add-a-ui-component.md
  - ../tutorials/create_new_game.md
  - ../reference/source-layout.md
keywords: [tests, gtest, gmock, headless, add_unit_test, PlatformMock, ClockMock, RendererMock, AudioSpy, SceneSpy, runFrames, determinism, temp directory]
last_reviewed: 2026-09-29
---

# Write headless GoogleTest tests

Tests exist only in the Debug preset. They use GoogleTest and GoogleMock 1.18, and never open a window. The one suite that touches SFML, `event_translate_test`, only links `SFML::Window` to translate events.

```sh
cmake --preset debug
cmake --build --preset debug --target build_ut
ctest --preset debug
```

## 1. Register a suite

Put the test next to the module it covers: `tests/engine/<module>/`, `tests/<game>/sim/` or `tests/<game>/app/`. Register it in that leaf's `CMakeLists.txt` with `add_unit_test` ([cmake/AddUnitTest.cmake](../../cmake/AddUnitTest.cmake)). This adds the executable, links `gtest`, `gmock` and `gtest_main`, registers it with CTest, and adds it to `build_ut`:

```cmake
add_unit_test(tetris_scoring_test
    SOURCES
        ScoringTest.cpp
    DEPENDENCIES
        tetris_sim
)
```

A new leaf directory also needs an `add_subdirectory(...)` in [tests/CMakeLists.txt](../../tests/CMakeLists.txt). Name fixtures `<Thing>Should` and tests after the behavior, for example `TEST(DodgerSimShould, beDeterministicForSeed)`.

## 2. Test a simulation

Sims are pure: construct state from a seed, call `step` with a scripted input, and assert on the state or the returned events. Always include a determinism test that runs two states with the same seed and inputs and compares them. It catches hidden clocks, statics and unseeded randomness.

## 3. Test a scene with a real SceneStack

Scenes need no mocks. Build the real stack and feed real input:

```cpp
sgl::SceneStack stack{sgl::dodger::pause(actions, sgl::FontId{})};
stack.push(sgl::dodger::play(actions, sgl::FontId{}, 42)());
sgl::InputState input;

input.apply(sgl::KeyDown{.key = sgl::Key::Escape}, map);
stack.update(input, sgl::kTick);

EXPECT_EQ(stack.size(), 2u); // pause overlay pushed
```

- **Edges.** `pressed` and `released` stay set until `input.beginFrame()`. Call it between simulated frames, as `App` does, or one press will fire on every tick.
- **Inspect the scene.** `dynamic_cast<const MyScene*>(stack.top())` gives access to a read-only accessor such as `state()`.
- **Assert on drawing.** Render into a `RenderQueue` and assert on `queue.sorted()`: count, layer, depth, and `std::holds_alternative<sgl::TextCmd>(entry.command)`.
- **Assets.** A default `FontId{}` or `TextureId{}` is fine, since nothing resolves it.

## 4. Test sound and feedback

Pass a `sgl::AudioSpy` ([tests/mocks/AudioSpy.hpp](../../tests/mocks/AudioSpy.hpp)) wherever the app expects an `AudioI&`, then assert on `spy.plays` (the id and volume of each `play` call).

## 5. Test the frame loop with mocks

To cover `App` itself (polling, fixed steps, render passes, closing), use the GoogleMock ports in `tests/mocks/`: `PlatformMock`, `ClockMock` and `sgl::RendererMock`. Drive a bounded number of frames with `App::runFrames(n)`:

```cpp
NiceMock<PlatformMock> platform;
NiceMock<ClockMock> clock;
NiceMock<sgl::RendererMock> renderer;
ON_CALL(platform, isOpen()).WillByDefault(Return(true));
ON_CALL(platform, renderer()).WillByDefault(ReturnRef(renderer));
ON_CALL(clock, restart()).WillByDefault(Return(sgl::kTick)); // exactly one fixed step per frame
EXPECT_CALL(platform, poll())
    .WillOnce(Return(sgl::InputEvent{sgl::KeyDown{.key = sgl::Key::Escape}}))
    .WillRepeatedly(Return(std::nullopt));

sgl::App{platform, clock, sgl::dodger::defaultBindings(actions), stack, queue}.runFrames(2);

EXPECT_EQ(stack.size(), 2u);
```

Include the mocks with `#include <mocks/PlatformMock.hpp>`, because the tests root is on the include path. For a trivial scene, use `sgl::SceneSpy` from `tests/engine/scene/fakes/`: it counts updates, records `dt`, and runs an optional `onUpdate` callback. Add `${CMAKE_SOURCE_DIR}/tests/engine/scene/fakes/SceneSpy.cpp` to the suite's `SOURCES`.

The clock mock controls how many fixed steps each frame runs. `Seconds{0.f}` runs none, which is useful for checking that input edges survive a frame without a step. `kTick` runs one.

## 6. Test anything that writes files

High scores and settings write to disk. Pass a unique path under `std::filesystem::temp_directory_path()` and remove it, plus its `.tmp` sibling, in the fixture destructor. `tests/tetris/app/ScenesTest.cpp` shows this pattern. Never write into the user data directory from a test.

## 7. Checklist

- The suite is registered with `add_unit_test` and builds under `build_ut`.
- No window opens, and no SFML appears outside `event_translate_test`.
- Randomness comes from a fixed seed, and there is no wall clock.
- Floating-point comparisons use `EXPECT_FLOAT_EQ` or `EXPECT_NEAR`.
- Temp files are cleaned up.
- The suite is listed in [source-layout.md](../reference/source-layout.md).
