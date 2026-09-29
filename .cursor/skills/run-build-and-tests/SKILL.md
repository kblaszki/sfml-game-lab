---
name: run-build-and-tests
description: >-
  Configures, builds, formats, and runs tests for sfml-game-lab using CMake presets.
  Use when building the game, running unit tests, formatting code, or verifying CI-like steps.
---

# Run build and tests

Source of truth: [docs/how-to/build-and-test.md](../../../docs/how-to/build-and-test.md) and [docs/tutorials/getting-started.md](../../../docs/tutorials/getting-started.md). Keep this skill consistent with those docs.

## Prerequisites

- CMake ≥ 3.20, Ninja, C++23 compiler
- SFML 3.1 Graphics/Window/System/Audio, fetched by CMake (no manual install)
- GoogleTest 1.18 is fetched only for Debug-family presets (`debug`, `asan`, `coverage`)

## Configure

```sh
cmake --preset debug
# or
cmake --preset release
# or (GCC/Clang)
cmake --preset asan
cmake --preset coverage
```

Outputs go to `build/debug/`, `build/release/`, `build/asan/`, or `build/coverage/`.

## Build game

```sh
cmake --build --preset debug --target arkanoid
cmake --build --preset debug --target tetris
cmake --build --preset debug --target syndicate
# or the same targets with --preset release
```

Run from the binary directory:

- Windows: `build/debug/bin/arkanoid.exe`, `build/debug/bin/tetris.exe`, `build/debug/bin/syndicate.exe` (same names under `build/release/bin/`)
- Linux/macOS: `build/debug/bin/arkanoid`, `build/debug/bin/tetris`, `build/debug/bin/syndicate` (same names under `build/release/bin/`)

## Tests (Debug-family only)

Unit tests and GTest are configured only when `CMAKE_BUILD_TYPE` is Debug:

```sh
cmake --preset debug
cmake --build --preset debug --target build_ut
ctest --preset debug
```

ASan (GCC/Clang) and coverage (GCC):

```sh
cmake --preset asan && cmake --build --preset asan --target build_ut && ctest --preset asan
cmake --preset coverage && cmake --build --preset coverage --target build_ut && ctest --preset coverage
```

`gcovr` is the report only. On Windows it is the system Python package, run as `py -m gcovr` with MinGW `gcov` on `PATH`. Flags and the 90% filter: [build-and-test](../../../docs/how-to/build-and-test.md). The `asan` preset does not link on MSYS2 MinGW (no `libubsan`).

## Format / tidy

Require a Debug configure (`format` / `tidy` targets; tidy needs `run-clang-tidy`):

```sh
cmake --build --preset debug --target format
cmake --build --preset debug --target tidy
```

## Typical verify loop after code changes

1. Configure debug (if needed)
2. Build the touched game targets (`arkanoid`, `tetris`, `syndicate`) and/or `build_ut`
3. Run `ctest --preset debug`
4. Optionally run `format` before commit
5. On Linux CI-like checks: `asan`, `tidy`, and `coverage` (see [build-and-test](../../../docs/how-to/build-and-test.md))
