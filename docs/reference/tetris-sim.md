---
title: Tetris sim
diataxis: reference
audience: [ai, human]
related_code:
  - games/tetris/sim/include/tetris/sim/Types.hpp
  - games/tetris/sim/include/tetris/sim/Shapes.hpp
  - games/tetris/sim/include/tetris/sim/Srs.hpp
  - games/tetris/sim/include/tetris/sim/Grid.hpp
  - games/tetris/sim/include/tetris/sim/Piece.hpp
  - games/tetris/sim/include/tetris/sim/Bag.hpp
  - games/tetris/sim/include/tetris/sim/Scoring.hpp
  - games/tetris/sim/include/tetris/sim/Tuning.hpp
  - games/tetris/sim/include/tetris/sim/Events.hpp
  - games/tetris/sim/include/tetris/sim/TetrisGame.hpp
  - games/tetris/sim/src/Grid.cpp
  - games/tetris/sim/src/Piece.cpp
  - games/tetris/sim/src/Bag.cpp
  - games/tetris/sim/src/Scoring.cpp
  - games/tetris/sim/src/TetrisGame.cpp
  - games/tetris/sim/CMakeLists.txt
  - tests/tetris/sim/GridTest.cpp
  - tests/tetris/sim/SrsTest.cpp
  - tests/tetris/sim/BagTest.cpp
  - tests/tetris/sim/ScoringTest.cpp
  - tests/tetris/sim/TetrisGameTest.cpp
  - tests/tetris/sim/TetrisPropertyTest.cpp
  - tests/tetris/sim/CMakeLists.txt
related_docs:
  - engine-core.md
  - source-layout.md
  - ../explanation/architecture.md
keywords: [tetris, sim, Grid, Piece, SRS, Bag, Scoring, TetrisGame, DAS, ARR, lock delay, hold, 7-bag]
last_reviewed: 2026-09-29
---

# Tetris sim

Headless Tetris rules in `namespace sgl::tetris`. Static library target `tetris_sim` links `PUBLIC` `sgl_core` only. No SFML. Include as `<tetris/sim/X.hpp>`.

Coordinates: **+y up**, row 0 is the bottom of the 10×40 grid (`kVisibleHeight` 20).

## Types (`Types.hpp`)

| Name | Role |
|------|------|
| `PieceType` | `I`, `O`, `T`, `S`, `Z`, `J`, `L` |
| `Rotation` | `Spawn`, `Right`, `Reverse`, `Left` (0, R, 2, L) |
| `Turn` | `Cw`, `Ccw` |
| `Coord` | `x`/`y` with `<=>` and `operator+` |
| `Cells` | `std::array<Coord, 4>` |
| `Tile` | `std::optional<PieceType>` |

## Shapes (`Shapes.hpp`)

`boxSize`: I→4, O→2, others→3. `shape(type, rotation)` returns cells in the local `n×n` box. Spawn offsets match the guideline boxes; CW rotation is `(x, y) → (y, n - 1 - x)`. O cells are fixed for every rotation. Compile-time checks require four distinct cells per state and that four CW turns return the spawn set.

## Grid (`Grid.hpp`)

| API | Behavior |
|-----|----------|
| `inside` | `[0, 10) × [0, 40)` |
| `occupied` | Outside counts as occupied; else tile present |
| `fits` | All four cells free |
| `lock` | Writes `PieceType` into cells (precondition: `fits`) |
| `clearFullRows` | Records full-row indices, `std::ranges::remove_if` full rows (`all_of` tiles), fills freed top rows empty |
| `rows` | `span` of 40 rows |

`ClearedRows` holds up to four original indices and `count`.

## Piece and SRS (`Piece.hpp`, `Srs.hpp`)

`Piece` is `{type, rotation, origin}`. `cells` is origin plus `shape`. `spawnPiece` uses column 3 and places the box top at row 21. `tryShift` / `tryRotate` / `dropToFloor` are pure queries on a `Grid`.

`tryRotate` uses the five SRS kick tests from `kicks` (y-up tables for JLSTZ and I). O has no kicks; rotation only updates the enum (cells unchanged).

## Bag (`Bag.hpp`)

7-bag randomizer: each refill Fisher–Yates-shuffles all seven `PieceType` values with `sgl::Pcg32::uniformBelow`. `next()` consumes one type; `preview()` returns the next five without consuming.

## Scoring (`Scoring.hpp`)

`Score` holds `points`, `lines`, and `level` (starts at 1). Line clears score `100` / `300` / `500` / `800` times level for 1–4 lines. Soft drop `+1` per row; hard drop `+2` per row. Level is `1 + lines / 10`. Gravity interval (seconds per row) is `pow(0.8 - (level - 1) * 0.007, level - 1)`.

## Tuning and events

`Tuning` defaults: DAS `0.167` s, ARR `0.033` s, lock delay `0.5` s, max lock resets `15`, soft-drop factor `20`. `TetrisEvent` is a `std::variant` of `PieceSpawned`, `PieceShifted`, `PieceRotated`, `PieceLocked`, `LinesCleared`, `HardDropped`, `HoldUsed`, `LevelUp`, `GameOver`.

`LinesCleared` carries the real cleared row indices: `std::array<std::uint8_t, 4> rows` (original grid y values, bottom-up) plus `count` (1–4). Unused `rows` slots stay zero. `TetrisGame` copies them from `Grid::clearFullRows()` when emitting the event.

## TetrisGame (`TetrisGame.hpp`)

`TetrisGame(seed, Tuning = {})` owns bag, grid, active/held pieces, and score. `step(TetrisInput, Seconds)` processes in order: hold → rotations → horizontal DAS/ARR → soft/hard drop → gravity → lock → line clear → spawn.

| Rule | Behavior |
|------|----------|
| DAS/ARR | Pressed edge (or held false→true) shifts once and resets DAS; after `das`, repeats every `arr` (`arr == 0` slides to the wall). Last pressed direction wins if both are held. |
| Lock delay | `0.5` s while grounded; successful move/rotate while grounded resets the timer up to `maxLockResets` times; reaching a new lowest row restores the reset counter. |
| Hold | Once per piece; held type respawns in spawn orientation. |
| Game over | Block out (spawn overlaps) or lock out (all four locked cells above row 19). |

Accessors: `grid()`, `active()`, `ghost()`, `held()`, `nextQueue()`, `score()`, `over()`.

## Tests

`GridTest` / `SrsTest` cover fit/occupy/lock/clear and kicks. `BagTest` covers 100 bags, seed stability, and preview. `ScoringTest` covers the line table, drop points, level-ups, and gravity at levels 1 and 15. `TetrisGameTest` covers DAS/ARR, hard drop, lock delay/resets, hold, block/lock out, and a level-1 tetris (including `LinesCleared` row indices 0–3). `TetrisPropertyTest.invariantsOverRandomInput` runs 20 seeds × 20 000 ticks of random input.
