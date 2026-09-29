---
title: Add a game
diataxis: how-to
audience: [ai, human]
related_code:
  - games/CMakeLists.txt
  - games/tetris/CMakeLists.txt
  - games/tetris/sim/CMakeLists.txt
  - games/tetris/app/CMakeLists.txt
  - games/tetris/main.cpp
  - cmake/CheckBoundaries.cmake
  - cmake/AddUnitTest.cmake
  - tests/CMakeLists.txt
related_docs:
  - build-and-test.md
  - ../reference/source-layout.md
  - ../../games/tetris/docs/reference/sim.md
  - ../../games/tetris/docs/reference/app.md
  - ../explanation/architecture.md
keywords: [game, tetris, arkanoid, cmake, check_boundaries, SceneI, sim, app]
last_reviewed: 2026-09-29
---

# Add a game

Add a title under `games/<name>/` the way Tetris sits beside Arkanoid. The simulation stays headless. Only `games/<name>/main.cpp` may include SFML.

## 1. Directories

```
games/<name>/
  CMakeLists.txt
  main.cpp
  sim/include/<name>/sim/
  sim/src/
  sim/CMakeLists.txt
  app/include/<name>/app/
  app/src/
  app/CMakeLists.txt
tests/<name>/sim/
tests/<name>/app/
```

Namespace is `sgl::<name>`. Includes are `<name>/sim/X.hpp>` and `<name>/app/X.hpp>`. Register the directory from [`games/CMakeLists.txt`](../../games/CMakeLists.txt) and the test tree from [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt).

## 2. Targets

| Target | Kind | Links |
|--------|------|--------|
| `<name>_sim` | STATIC | `sgl_core`, plus `sgl_collision` only if the sim sweeps shapes. No SFML. |
| `<name>_app` | STATIC | `<name>_sim`, `sgl_loop`, `sgl_resources`. Add `sgl_audio`, `sgl_fx`, and `sgl_save` when the game plays sound, particles, or high scores. No SFML includes. |
| `<name>` | executable | `<name>_app`, `sgl_sfml`. Defines `ASSET_DIR` to the repo `assets/` root. |

List every new `.cpp` in that module's own `CMakeLists.txt`. Mirror [`games/tetris/CMakeLists.txt`](../../games/tetris/CMakeLists.txt): `cxx_std_23`, `${BASE_COMPILE_FLAGS}`, and `add_dependencies(<name> check_boundaries)`.

## 3. Boundaries

`check_boundaries` ([`cmake/CheckBoundaries.cmake`](../../cmake/CheckBoundaries.cmake)) fails the build when:

- any engine file outside `engine/sfml` includes SFML
- any game file other than `games/<name>/main.cpp` includes `<SFML/`
- any engine file includes `<name/>` for a directory under `games/`

Keep randomness on `sgl::Pcg32`. Do not call `std::*_distribution`, `std::shuffle`, `rand`, or the wall clock from sim code.

## 4. Scenes and the loop

App scenes implement `sgl::SceneI` and talk to the stack with `SceneRequest` (`PushScene`, `PopScene`, `ReplaceScene`, `RequestPause`, `QuitApp`). A scene must not close the platform. Pause is `RequestPause` when the top scene is `pausable`; the overlay is not opaque. `main.cpp` builds `SfmlPlatform`, loads assets, pushes the menu, and runs `sgl::App`.

## 5. Tests

Register each suite with `add_unit_test` under `tests/<name>/sim/` and `tests/<name>/app/`. Suites are headless. Cover the sim rules before the window, and pass a temp directory into any high-score test.

```sh
cmake --preset debug
cmake --build --preset debug --target build_ut <name>
ctest --preset debug
```

## 6. Docs

In the same change:

- add `docs/reference/<name>-sim.md` and `docs/reference/<name>-app.md` with `related_code` listing every new header and `.cpp`
- add both pages to [`docs/index.md`](../index.md)
- add the targets and suites to [`docs/reference/source-layout.md`](../reference/source-layout.md)
- mention the executable in [`README.md`](../../README.md)

Set `last_reviewed` to the day you edit the page.
