---
title: Build, test, and format
diataxis: how-to
audience: [ai, human]
related_code:
  - CMakePresets.json
  - CMakeLists.txt
  - cmake/FetchSFML.cmake
  - tests/CMakeLists.txt
  - tests/engine/core/CMakeLists.txt
  - tests/arkanoid/sim/CMakeLists.txt
  - tests/arkanoid/app/CMakeLists.txt
  - tests/tetris/sim/CMakeLists.txt
  - tests/tetris/app/CMakeLists.txt
  - .github/workflows/ci.yml
  - .clang-tidy
  - scripts/install-deps-ubuntu.sh
related_docs:
  - ../tutorials/getting-started.md
  - ../reference/source-layout.md
  - ../reference/engine-audio.md
keywords: [build, test, ctest, format, presets, build_ut, debug, release, asan, coverage, tidy, arkanoid, tetris, syndicate, SFML, Audio]
last_reviewed: 2026-11-04
---

# Build, test, and format

Prerequisite: toolchain installed (see [getting-started](../tutorials/getting-started.md); on Ubuntu/Debian: `sudo ./scripts/install-deps-ubuntu.sh`). The project is C++23; GoogleTest 1.18 is fetched only for Debug-family presets. Ubuntu CI uses g++-14. CMake FetchContent pulls SFML 3.1 with **Audio** enabled (`SFML_BUILD_AUDIO ON`; Network stays OFF) — no manual SFML install.

## Configure

```sh
cmake --preset debug      # or: release, asan, coverage
```

| Preset | Notes |
|--------|--------|
| `debug` | Debug; writes `compile_commands.json` (`CMAKE_EXPORT_COMPILE_COMMANDS ON`) |
| `release` | Release; no unit tests |
| `asan` | Debug + AddressSanitizer and UBSan on GCC/Clang (`SGL_ENABLE_ASAN`; skipped on MSVC) |
| `coverage` | Debug + GCC `--coverage` (`SGL_ENABLE_COVERAGE`) |

Outputs go to `build/<preset>/`.

## Build a game

```sh
cmake --build --preset debug --target arkanoid
cmake --build --preset debug --target tetris
cmake --build --preset debug --target syndicate
```

Binaries: `build/<preset>/bin/arkanoid`, `build/<preset>/bin/tetris`, and `build/<preset>/bin/syndicate` (`.exe` on Windows).

## Run tests (Debug-family only)

Tests and GoogleTest exist only when `CMAKE_BUILD_TYPE=Debug`. Suites are headless except `event_translate_test` (links `SFML::Window`, opens no window).

```sh
cmake --preset debug
cmake --build --preset debug --target build_ut
ctest --preset debug
```

ASan (`-fsanitize=address,undefined`) is for Linux GCC/Clang. MinGW in MSYS2 does not ship `libubsan`, so the `asan` preset does not link there. CI runs it on Ubuntu g++-14.

Coverage instrumentation is GCC `--coverage` (preset `coverage`). `gcov` comes with that GCC. `gcovr` is only the report and the 90% line gate; `ctest` does not need it.

```sh
cmake --preset asan
cmake --build --preset asan --target build_ut
ctest --preset asan

cmake --preset coverage
cmake --build --preset coverage --target build_ut
ctest --preset coverage
gcovr --root . --filter 'engine/.*' --exclude 'engine/sfml/.*' --filter 'games/.*/sim/.*' \
  --exclude-unreachable-branches --exclude-throw-branches \
  --html-details build/coverage/html/index.html --txt --fail-under-line 90 \
  --object-directory build/coverage
```

On Windows, install `gcovr` with the system Python (`py -m pip install gcovr`), not inside MSYS2. Run it as `py -m gcovr` with the same flags when the `gcovr` command is not on `PATH`. `gcov` on `PATH` must be the MinGW one that built the preset (`C:\msys64\mingw64\bin`). HTML lands in `build/coverage/html/index.html`.

Suites (`tests/engine/`, `tests/arkanoid/`, `tests/tetris/`, `tests/syndicate/`, `tests/snake/`, `tests/asteroids/`, `tests/dodger/`):

- Engine: `vec_test`, `rect_test`, `handle_test`, `image_test`, `random_test`, `input_state_test`, `scene_stack_test`, `fixed_step_loop_test`, `app_test`, `render_queue_test`, `projection_test`, `collision_test`, `resource_cache_test`, `tone_test`, `particle_system_test`, save suites, `event_translate_test`, …
- Arkanoid: `arkanoid_sim_test`, `arkanoid_physics_test`, `arkanoid_sim_property_test`, `arkanoid_powerup_test`, `arkanoid_assets_test`, `arkanoid_hud_test`, `arkanoid_scene_render_test`, `arkanoid_scenes_test`, `arkanoid_result_scene_test`, `arkanoid_sounds_test`, `arkanoid_feedback_test`
- Tetris: `tetris_grid_test`, `tetris_srs_test`, `tetris_bag_test`, `tetris_scoring_test`, `tetris_game_test`, `tetris_property_test`, `tetris_input_mapping_test`, `tetris_board_render_test`, `tetris_hud_test`, `tetris_scenes_test`, `tetris_sounds_test`, `tetris_feedback_test`
- Syndicate: `syndicate_money_test`, `syndicate_match_test`, `syndicate_layout_view_test`, `syndicate_play_scene_test`, `syndicate_scenes_test`, `syndicate_feedback_test`, …
- Snake: `snake_game_test`, `snake_scenes_test`, `snake_play_scene_test`, `snake_sounds_test`, `snake_feedback_test`
- Asteroids: `asteroids_game_test`, `asteroids_scenes_test`, `asteroids_play_scene_test`, `asteroids_sounds_test`, `asteroids_feedback_test`, `asteroids_sprite_bank_test`
- Dodger: `dodger_game_test`, `dodger_scenes_test`, `dodger_play_scene_test`, `dodger_sounds_test`, `dodger_feedback_test`

`build_ut` builds every suite registered with `add_unit_test(...)` (`cmake/AddUnitTest.cmake`). In Debug it also depends on `check_boundaries`, so building tests enforces the engine/games SFML and include-boundary rules.

## clang-tidy

Requires a Debug configure (for `compile_commands.json`) and `run-clang-tidy` on `PATH`:

```sh
cmake --preset debug
cmake --build --preset debug --target tidy
```

## CI

GitHub Actions (`.github/workflows/ci.yml`):

- `build` — Debug `build_ut` + `ctest`, then Release `arkanoid` and `tetris`, on `ubuntu-24.04` (g++-14) and `windows-2022` (MSVC)
- `asan` — Linux g++-14, preset `asan`, `build_ut` + `ctest`
- `tidy` — Linux: `build_ut` with g++-14, then the blocking `tidy` target (clang-tidy reads that compile database; `-std=c++23` and `-D__cpp_concepts=202002L` so clang-tidy-18 sees `std::expected` in libstdc++ 14) over `engine/` and `games/`
- Linux jobs install `libvorbis-dev` and `libflac-dev` because SFML uses system audio codecs there (`SFML_USE_SYSTEM_DEPS`)
- `coverage` — Linux g++-14, preset `coverage`, gcovr with `--fail-under-line 90` and `--gcov-executable gcov-14` (notes must match that compiler), filtered to `engine/` (excluding `engine/sfml`) and `games/*/sim`; HTML artifact uploaded

## Format the code

The `format` target is created only after a Debug configure and runs clang-format in place:

```sh
cmake --build --preset debug --target format
```

## Clean rebuild

If a configure failed halfway (e.g. missing tool cached), delete the preset build dir and reconfigure:

```sh
rm -rf build/debug
cmake --preset debug
```
