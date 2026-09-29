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
  - ../tutorials/create_new_game.md
  - add-a-scene.md
  - write-headless-tests.md
  - build-and-test.md
  - ../reference/source-layout.md
  - ../../games/tetris/docs/index.md
  - ../explanation/architecture.md
keywords: [game, new game, checklist, cmake, check_boundaries, SceneI, sim, app, games docs]
last_reviewed: 2026-09-29
---

# Add a game

This is the checklist. For a full worked example with compiled code, follow [create_new_game.md](../tutorials/create_new_game.md). The simulation stays headless. Only `games/<name>/main.cpp` may include SFML.

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
  docs/index.md
  docs/{tutorials,how-to,reference,explanation}/
tests/<name>/sim/
tests/<name>/app/
```

The namespace is `sgl::<name>`, and includes are `<<name>/sim/X.hpp>` and `<<name>/app/X.hpp>`. Register the game directory in [`games/CMakeLists.txt`](../../games/CMakeLists.txt) and the test leaves in [`tests/CMakeLists.txt`](../../tests/CMakeLists.txt).

## 2. Targets

| Target | Kind | Links |
|--------|------|--------|
| `<name>_sim` | STATIC | `sgl_core`, plus `sgl_collision` only if the sim sweeps shapes. No SFML. |
| `<name>_app` | STATIC | `<name>_sim` and the engine modules the scenes use: `sgl_scene`, `sgl_render`, `sgl_ui`, and as needed `sgl_audio`, `sgl_fx`, `sgl_anim`, `sgl_save`, `sgl_resources`. No SFML includes. |
| `<name>` | executable | `<name>_app`, `sgl_sfml`. Defines `ASSET_DIR` as the repo's `assets/` root. |

List every new `.cpp` in its own module's `CMakeLists.txt`. Mirror [`games/tetris/CMakeLists.txt`](../../games/tetris/CMakeLists.txt): `cxx_std_23`, `${BASE_COMPILE_FLAGS}`, and `add_dependencies(<name> check_boundaries)`.

## 3. Boundaries

`check_boundaries` ([`cmake/CheckBoundaries.cmake`](../../cmake/CheckBoundaries.cmake)) fails the build when:

- any engine file outside `engine/sfml` includes SFML
- any game file other than `games/<name>/main.cpp` includes `<SFML/`
- any engine file includes `<name/>` for a directory under `games/`

Keep randomness on `sgl::Pcg32`. Do not call `std::*_distribution`, `std::shuffle`, `rand`, or the wall clock from sim code.

## 4. Scenes and the loop

App scenes implement `sgl::SceneI` and change the stack only through `SceneRequest`. `main.cpp` builds `SfmlPlatform`, loads assets, constructs `SceneStack{pauseFactory}`, pushes the first scene, and runs `sgl::App`. See [add-a-scene.md](add-a-scene.md).

## 5. Tests

Register each suite with `add_unit_test` under `tests/<name>/sim/` and `tests/<name>/app/`. Cover the sim rules, including a same-seed determinism test, before the window. See [write-headless-tests.md](write-headless-tests.md).

```sh
cmake --preset debug
cmake --build --preset debug --target build_ut <name>
ctest --preset debug
```

## 6. Docs

In the same change:

- Create `games/<name>/docs/index.md` listing all four Diátaxis quadrants.
- Add `games/<name>/docs/reference/sim.md` and `reference/app.md`, with `related_code` listing every new header and `.cpp`.
- Add an explanation page with a Mermaid `stateDiagram-v2` of the game flow.
- Add the game to the "Game documentation" table in [`docs/index.md`](../index.md).
- Add the targets and suites to [`docs/reference/source-layout.md`](../reference/source-layout.md).
- Mention the executable in [`README.md`](../../README.md).

Set `last_reviewed` to the day you edit the page.
