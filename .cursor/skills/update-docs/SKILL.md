---
name: update-docs
description: >-
  Audits and updates the Diataxis docs (docs/ for the engine, games/<game>/docs/ per game)
  after code changes so documentation stays accurate. Use when code changed and docs may be
  stale, when adding files/targets, or when asked to refresh, verify, or sync the documentation.
---

# Update docs

Keep both doc trees in sync with the code:

- `docs/`: the engine and cross-game topics. Map: `docs/index.md`.
- `games/<game>/docs/`: per-game docs. Map: `games/<game>/docs/index.md`.

Docs are the source of truth. Each file has front-matter with `related_code`, `related_docs`, `keywords`, and `last_reviewed`.

## Workflow

```
- [ ] Get the list of changed code files (git diff --name-only)
- [ ] Find docs in docs/ AND games/*/docs/ whose related_code lists any changed file
- [ ] Update the content of those docs to match the code
- [ ] Set last_reviewed to today's date in each doc you edited
- [ ] Handle structural changes (new/removed file, target, directory, mechanic)
- [ ] Architecture or state-flow change -> Mermaid diagram present and current
- [ ] Verify links, related_docs, and related_code paths resolve in both trees
- [ ] Refresh docs/index.md and/or games/<game>/docs/index.md if docs were added/removed/retitled
```

## Steps

1. **Changed files.** Run `git diff --name-only`, or use the set you just edited.

2. **Map to docs.** Search `docs/` and `games/*/docs/` for each changed path inside `related_code`, for example `rg -l "games/syndicate/sim/src/Match.cpp" docs games`. Update every matching doc so its statements match the new code, then bump `last_reviewed` to today.
   - `engine/**` usually maps to `docs/reference/engine-*.md` and `docs/explanation/*.md`.
   - `games/<game>/**` usually maps to `games/<game>/docs/**`.
   - `tests/**` maps to the suite table in `docs/reference/source-layout.md` and the reference page for the module under test.

3. **Structural changes.**
   - New, removed or renamed `engine/**` or `games/**` source file or CMake target: update `docs/reference/source-layout.md`.
   - A new engine feature: update or add a `docs/reference/engine-*.md` page, plus a how-to if it adds a workflow.
   - A new game mechanic: update or add a page under `games/<game>/docs/reference/`, plus an explanation if the rationale is not obvious.
   - A new interface, loop, scene-flow or game-state change: update the Mermaid `classDiagram` / `sequenceDiagram` / `stateDiagram-v2` in the matching explanation page (`docs/explanation/architecture.md` for the engine).
   - A new game: create `games/<game>/docs/index.md` and add a row to the "Game documentation" table in `docs/index.md`.
   - Any added, removed or retitled doc: update the owning index table and its `related_docs`.

4. **Verify.**
   - Relative links resolve.
   - Every `related_code` path exists (repo-root relative).
   - Every `related_docs` entry resolves relative to its doc.
   - Facts (C++23, GTest 1.18, presets, targets, binary paths) match the root `CMakeLists.txt`, the module `CMakeLists.txt` files and `CMakePresets.json`.
   - No references to removed trees (`src/`, `gameLib`, executable `game`), or to old paths such as `docs/reference/<game>-*.md`.
   - No generic manager names (SceneManager and so on) presented as real types.

## Front-matter shape

```yaml
---
title: ...
diataxis: tutorial | how-to | reference | explanation
audience: [ai, human]
related_code: [ ... repo-root-relative source paths ... ]
related_docs: [ ... paths relative to this doc ... ]
keywords: [ ... ]
last_reviewed: YYYY-MM-DD
---
```

Keep this schema identical across both trees so search stays reliable.
