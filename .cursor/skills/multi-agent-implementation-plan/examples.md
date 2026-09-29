# Examples

## Code: grow a module in sequential waves

This tree is `engine/` (`eng`) + `games/arkanoid/` (`arkanoid`, executable `arkanoid`). Contract: [`docs/explanation/architecture.md`](../../../docs/explanation/architecture.md). Use `SceneI` / `SceneStack`, not legacy `ScreenStack`.

| Wave | Package | Owned writes (typical) |
|------|---------|------------------------|
| 1 | Engine module type | `engine/<module>/…`, `tests/engine/<module>/…`, matching `docs/reference/engine-*.md` + `source-layout.md` |
| 2 | Game sim or app | `games/arkanoid/<sim|app>/…`, `tests/arkanoid/…`, matching `games/arkanoid/docs/**/*.md` |
| 3 | Docs leftover sweep | leftover `docs/**` only (`related_code`, `docs/index.md`) |

Within a wave, split only if paths stay disjoint and each agent owns its module `CMakeLists.txt`.

After each of waves 1–2:

1. Same-change Diátaxis docs for every touched file. New types update `source-layout.md` (and a new reference/how-to if behavior needs a home).
2. Hard gate: the wave diff must include a `docs/` path. If not, write the docs before commit.
3. Debug `build_ut` + `ctest --preset debug`.
4. One orchestrator commit (owned paths + matching docs). Do not commit if the docs gate or verify failed.

Wave 3 is a leftover-docs audit, not the first time docs are written. Commit that sweep if it changes files.

Locked names stay those in this tree: `sgl::*`, `sgl::arkanoid::*`, executable `arkanoid`. Prospective spelling in [`mvp/`](../../../mvp/README.md) is notes, not current types.

## Docs-only: `mvp/` snapshots

`mvp/01`–`09` are **prospective**. Do not rewrite them as current source facts. Use this docs-only fan-out only when the user asked for a written plan, not the implementation. Orchestrator commits after stitch if verify (links / contract lock) passes.
