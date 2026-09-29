# Agent guide: sfml-game-lab

This is a C++23 game lab on SFML 3.1. The engine is `engine/` (`namespace sgl`). The games are `games/arkanoid`, `games/tetris` and `games/syndicate` (`namespace sgl::<game>`, one executable each). This file is a tool-neutral summary. The full rules live in [`.cursor/rules/`](.cursor/rules) and the task skills in [`.cursor/skills/`](.cursor/skills).

## Build and test

```sh
cmake --preset debug
cmake --build --preset debug --target arkanoid tetris syndicate
cmake --build --preset debug --target build_ut
ctest --preset debug
```

Tests exist only in Debug. Binaries go to `build/<preset>/bin/`. On Windows PowerShell there is no `&&`; chain with `; if ($LASTEXITCODE -eq 0) { ... }`.

## Where to read first

| Task | Read |
|------|------|
| Engine work | [`docs/explanation/architecture.md`](docs/explanation/architecture.md), [`docs/reference/engine-api-map.md`](docs/reference/engine-api-map.md), `docs/reference/engine-*.md` |
| An existing game | `games/<game>/docs/index.md` |
| A new game | [`docs/tutorials/create_new_game.md`](docs/tutorials/create_new_game.md), [`docs/how-to/add-a-game.md`](docs/how-to/add-a-game.md) |
| Tests | [`docs/how-to/write-headless-tests.md`](docs/how-to/write-headless-tests.md) |

The map of engine docs is [`docs/index.md`](docs/index.md). Each game has its own Diátaxis tree under `games/<game>/docs/`. Use the real type names (`SceneStack`, `SceneI`, `InputState`, `ActionMap`, `ResourceCache`). There is no SceneManager, InputManager, ResourceManager or EventManager.

## Non-negotiables

1. **Headless First.** Mechanics live in `<game>_sim` (SFML-free, `sgl::Pcg32` only, fixed `kTick`) and are tested before any presentation. Only `engine/sfml/` and `games/<game>/main.cpp` include SFML, and `check_boundaries` enforces it. See [headless-first.mdc](.cursor/rules/headless-first.mdc).
2. **Docs in the same change.** Every new engine feature or game mechanic gets a Diátaxis entry, in `docs/` for the engine or `games/<game>/docs/` for a game. Architecture and state-flow changes include a Mermaid diagram (`classDiagram`, `sequenceDiagram` or `stateDiagram-v2`). Update the docs whose `related_code` lists the files you changed, and set `last_reviewed`. See [documentation.mdc](.cursor/rules/documentation.mdc).
3. **Wire every source.** Each new `.cpp` goes in its own module's `CMakeLists.txt`, and each new suite uses `add_unit_test`.
4. **Scenes change the stack only by request.** `PushScene`, `PopScene`, `ReplaceScene`, `RequestPause` or `QuitApp`. A scene never closes the platform.
5. **Surgical, verified, committed.** Keep changes minimal (see [golden-rule.mdc](.cursor/rules/golden-rule.mdc)), and commit each verified unit with explicit paths. Never commit `build/`.

## Conventions

The code uses `#pragma once` and follows `.clang-format` (`if(` with no space, braces on new lines, 120 columns). Interfaces take the `FooI` suffix. Ownership is by value or `std::unique_ptr`, with no raw `new`/`delete`. Errors are returned as `std::expected` or `std::optional`, not exceptions. Details are in [cpp-conventions.mdc](.cursor/rules/cpp-conventions.mdc) and [cmake-and-tests.mdc](.cursor/rules/cmake-and-tests.mdc).
