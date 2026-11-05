# sfml-game-lab

A lab of small C++23 games on a shared [SFML](https://www.sfml-dev.org/) 3.1 engine (2D now, 2.5D-ready). Eight titles ship today: a three-stage breakout in `games/arkanoid/`, Tetris in `games/tetris/`, Syndicate in `games/syndicate/`, Snake in `games/snake/`, Asteroids in `games/asteroids/`, Dodger in `games/dodger/`, Tower in `games/tower/`, and Tappy in `games/tappy/`. All use CMake presets and windowless unit tests.

The engine lives in `engine/` (`namespace sgl`). Prospective notes stay in [`mvp/`](mvp/README.md) (`01`–`09`); what landed is [`mvp/10`](mvp/10-engine-progress.md). The contract is [`docs/explanation/architecture.md`](docs/explanation/architecture.md).

## Table of Contents

- [Installation](#installation)
- [Project structure](#project-structure)
- [Building the Project](#building-the-project)
- [Running the Game](#running-the-game)
- [Running the Tests](#running-the-tests)
- [Contributing](#contributing)
- [License](#license)

## Installation

### Prerequisites

- [CMake](https://cmake.org/) 3.20 or higher
- [Ninja](https://ninja-build.org/) (used by CMake presets)
- A C++23 compiler (GCC, Clang, or MSVC)

SFML 3.1 is downloaded automatically via CMake FetchContent (`cmake/FetchSFML.cmake`; Graphics, Window, System, and Audio are on; Network stays off). A separate SFML install is not required. GoogleTest 1.18 is fetched only for Debug.

```sh
git clone git@github.com:DevKrystian/sfml-game-lab.git
cd sfml-game-lab
```

## Project structure

| Path | Role |
|------|------|
| `engine/` | Engine modules (`sgl_core`, input, scene, loop, render, collision, resources, audio, fx, save, `sgl_sfml`) |
| `games/arkanoid/` | Headless sim, scenes, and the `arkanoid` executable |
| `games/tetris/` | Headless sim, scenes, and the `tetris` executable |
| `games/syndicate/` | Headless sim, scenes, and the `syndicate` executable |
| `games/snake/` | Headless sim, scenes, and the `snake` executable |
| `games/asteroids/` | Headless sim, scenes, and the `asteroids` executable |
| `games/dodger/` | Headless sim, scenes, and the `dodger` executable |
| `games/tower/` | Headless sim, scenes, and the `tower` executable |
| `games/tappy/` | Headless sim, scenes, and the `tappy` executable |
| `assets/fonts/` | UI TTF (`ASSET_DIR`) |
| `tests/engine/`, `tests/arkanoid/`, `tests/tetris/`, `tests/syndicate/`, `tests/snake/`, `tests/asteroids/`, `tests/dodger/`, `tests/tower/`, `tests/tappy/` | Debug GoogleTest (no window, except event translation) |
| `docs/` | Diátaxis documentation for the engine (map: `docs/index.md`) |
| `games/<game>/docs/` | Diátaxis documentation for each game |
| `mvp/` | Prospective design (`01`–`09`) and living progress (`10`) |

Executable targets: `arkanoid`, `tetris`, `syndicate`, `snake`, `asteroids`, `dodger`, `tower`, and `tappy`. Only `engine/sfml` and `games/<game>/main.cpp` include SFML.

## Building the Project

The project uses `CMakePresets.json` for configurations.

### Available presets

- `debug` — Debug build (tests and `format` target enabled)
- `release` — Release build
- `asan`, `coverage` — Linux GCC/Clang gates; see [docs/how-to/build-and-test.md](docs/how-to/build-and-test.md)

### Build steps

1. Configure:

```sh
cmake --preset debug
```

or:

```sh
cmake --preset release
```

2. Build the game:

```sh
cmake --build --preset debug --target arkanoid
```

or:

```sh
cmake --build --preset release --target arkanoid
```

## Running the Game

After building, run the executable from the preset output directory:

```sh
# Windows
./build/debug/bin/arkanoid.exe
./build/release/bin/arkanoid.exe

# Linux / macOS
./build/debug/bin/arkanoid
./build/release/bin/arkanoid
```

A 1280×720 window titled "Arkanoid" opens on a Start / Quit menu. Enter starts Stage1. Clearing a stage loads the next one. Escape pauses.

Build and run Tetris the same way with target `tetris` (`./build/debug/bin/tetris` or `tetris.exe` on Windows). A 1280×720 window titled "Tetris" opens on a Start / Quit menu; Enter starts a game (arrows move, Down soft-drops, Space hard-drops, Up/X and Z rotate, C/Shift hold). Escape or P pauses.

Build and run Syndicate with target `syndicate` (`./build/debug/bin/syndicate` or `syndicate.exe` on Windows). A 1280×720 window titled "Syndicate" opens on a Play / Quit menu; Play opens seat setup (2–4 Human/Bot), Confirm starts the match (Space rolls, Escape pauses).

Build and run Snake with target `snake` (`./build/debug/bin/snake` or `snake.exe` on Windows). A 1280×720 window titled "Snake" opens on a Play / Quit menu; Play starts a game (arrows or WASD turn). Escape or P pauses.

Build and run Asteroids with target `asteroids` (`./build/debug/bin/asteroids` or `asteroids.exe` on Windows). A 1280×720 window titled "Asteroids" opens on a Play / Quit menu; Play starts a game (Left/Right or A/D rotate, Up or W thrusts, Space fires). Escape or P pauses.

Build and run Dodger with target `dodger` (`./build/debug/bin/dodger` or `dodger.exe` on Windows). A 1280×720 window titled "Dodger" opens on a Play / Quit menu; Play starts a game (Left/Right or A/D move the paddle). Escape or P pauses.

Build and run Tower with target `tower` (`./build/debug/bin/tower` or `tower.exe` on Windows). A 1280×720 window titled "Tower" opens on a Play / Quit menu; Play starts a game (Left/Right or A/D run, Space or Up or W jumps). Escape or P pauses.

Build and run Tappy with target `tappy` (`./build/debug/bin/tappy` or `tappy.exe` on Windows). A 1280×720 window titled "Tappy" opens on a Play / Quit menu; Play starts a flap game (Space or Up or W). Escape or P pauses.

## Running the Tests

Tests are available only for the **debug** preset. See [docs/how-to/build-and-test.md](docs/how-to/build-and-test.md).

```sh
cmake --preset debug
cmake --build --preset debug --target build_ut
ctest --preset debug
```

## Contributing

Contributions are welcome. Please:

1. Fork the repository and create a feature branch.
2. Match existing C++ style (see `.clang-format`). With a debug configure: `cmake --build --preset debug --target format`.
3. Treat warnings as errors — keep the build clean under the project flags.
4. Add or update unit tests under `tests/engine/`, `tests/arkanoid/`, `tests/tetris/`, `tests/syndicate/`, `tests/snake/`, `tests/asteroids/`, `tests/dodger/`, `tests/tower/`, or `tests/tappy/`.
5. List every new `.cpp` in that module's `CMakeLists.txt`.
6. Keep `docs/` and `games/<game>/docs/` current for files listed in a doc's `related_code` (see `docs/index.md` and [`AGENTS.md`](AGENTS.md)).
7. Open a pull request with a short description of the change.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
