---
title: Getting started
diataxis: tutorial
audience: [ai, human]
related_code:
  - CMakePresets.json
  - CMakeLists.txt
  - cmake/FetchSFML.cmake
  - cmake/FetchGTest.cmake
  - games/arkanoid/main.cpp
  - games/arkanoid/app/include/arkanoid/app/Scenes.hpp
  - games/arkanoid/sim/include/arkanoid/sim/State.hpp
  - games/syndicate/main.cpp
  - .github/workflows/ci.yml
related_docs:
  - create_new_game.md
  - ../how-to/build-and-test.md
  - ../reference/source-layout.md
  - ../reference/input-and-events.md
  - ../reference/pause-overlay.md
  - ../../games/arkanoid/docs/reference/sim.md
  - ../../games/syndicate/docs/reference/app.md
  - ../explanation/architecture.md
  - ../../mvp/10-engine-progress.md
keywords: [setup, toolchain, build, run, msys2, ninja, cmake, first time, arkanoid, tetris, syndicate]
last_reviewed: 2026-09-29
---

# Getting started

Goal: from a fresh clone of sfml-game-lab to a running window. This tutorial launches the first game in the lab. By the end you will have configured, built, and launched `arkanoid`.

## 1. Install the toolchain

You need CMake, Ninja, and a C++23 compiler on your `PATH`. On Windows this repo is set up around MSYS2 MinGW.

MSYS2 MinGW 64-bit shell:

```bash
pacman -S --needed \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-cmake
```

Add `C:\msys64\mingw64\bin` to `PATH` and restart the terminal. Verify:

```sh
g++ --version
ninja --version
cmake --version
```

SFML 3.1 (Graphics/Window/System only) and GoogleTest 1.18 are fetched automatically by CMake (`cmake/FetchSFML.cmake`, `cmake/FetchGTest.cmake` in Debug). No separate SFML install.

## 2. Configure

```sh
cmake --preset debug
```

This creates `build/debug/` and downloads dependencies on first run (can take a minute).

## 3. Build the game

```sh
cmake --build --preset debug --target arkanoid
```

## 4. Run it

```sh
# Windows
./build/debug/bin/arkanoid.exe
# Linux / macOS
./build/debug/bin/arkanoid
```

A 1280×720 window titled "Arkanoid" opens on a Start / Quit menu. Start or Enter plays Stage1 (arrows or A/D move the paddle, Enter launches). Clearing a stage loads the next; Stage3 is the last. Escape pauses. Backspace on the menu quits. Donor bricks drop Wide, MultiBall, Slow, and ExtraLife capsules.

The lab also ships Tetris and Syndicate. Build Tetris with `cmake --build --preset debug --target tetris` and run `./build/debug/bin/tetris` (or `tetris.exe` on Windows). Build Syndicate with `cmake --build --preset debug --target syndicate` and run `./build/debug/bin/syndicate` (or `syndicate.exe` on Windows). Both open a 1280×720 window; Syndicate is titled "Syndicate" with a Play / Quit menu, seat setup (2–4 Human/Bot), then the board (Space rolls, Escape pauses).

## Next steps

Everyday commands and tests: [../how-to/build-and-test.md](../how-to/build-and-test.md). Layout: [../reference/source-layout.md](../reference/source-layout.md). Input: [../reference/input-and-events.md](../reference/input-and-events.md). Pause: [../reference/pause-overlay.md](../reference/pause-overlay.md). Sim rules: [games/arkanoid/docs/reference/sim.md](../../games/arkanoid/docs/reference/sim.md). Build your own game next: [create_new_game.md](create_new_game.md).
