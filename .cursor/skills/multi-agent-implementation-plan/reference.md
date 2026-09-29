# Package cards and agent prompts

Read this after [SKILL.md](SKILL.md).

## Package card

The orchestrator fills one card per agent:

```
Goal: (one sentence)
Owned write paths: (engine/..., games/<game>/..., tests/..., and the matching doc pages — docs/ for engine, games/<game>/docs/ for games; always include docs when code is owned)
Forbidden paths: (everything else, especially siblings in this wave)
Locked signatures: (paste from contract)
Read first: (existing headers, contract, listed mvp/ or docs)
Verify: cmake --build --preset debug --target build_ut && ctest --preset debug
          (or “no code — skip build”)
```

Typical owned paths: `engine/input/src/…`, `games/syndicate/sim/…`, `tests/engine/input/…`. Matching docs: `docs/reference/source-layout.md`, the matching `docs/reference/engine-*.md` or `games/<game>/docs/reference/*.md`. Do not invent legacy `ScreenStack` / `gameLib` names.

Owned writes **must** include the matching doc paths whenever the card owns `engine/` or `games/<game>/` code. Parallelize two cards only when owned write paths do not overlap. Each module owns its `CMakeLists.txt` — parallel agents must not edit a shared sources list.

## Implement prompt

`model: grok-4.7-high-fast`, `subagent_type: generalPurpose`.

```
You own ONE work package. Implement it.

READ FIRST:
- CONTRACT / locked signatures (do not invent synonyms)
- SOURCE_PATHS (do not contradict the current tree)
- docs/explanation/architecture.md

OWNED WRITE PATHS (only these):
- ...

FORBIDDEN: do not edit other files. Do not reintroduce src/, gameLib, ScreenStack, WindowI, or DrawerI. Do not invent SceneManager / InputManager / ResourceManager / EventManager.

DO:
- Implement the goal in C++23, #pragma once, namespaces sgl / sgl::<game>
- Headless First: rules in <game>_sim with tests before presentation; no SFML outside engine/sfml and games/<game>/main.cpp
- List every new .cpp in that module’s own CMakeLists.txt
- Add windowless GoogleTest via add_unit_test in the matching tests leaf
- You must produce a doc edit (docs/ or games/<game>/docs/) if you change engine/ or games/ code (hard gate)
- Update docs whose related_code lists a file you changed; last_reviewed today
- New/removed source file or target → docs/reference/source-layout.md
- New engine feature or game mechanic → update or add a reference (or how-to) page in the owning tree; architecture/state-flow changes add a Mermaid diagram; source-layout inventory alone is not enough
- New/removed/retitled doc → docs/index.md or games/<game>/docs/index.md
- Do not git commit (the orchestrator commits after stitch)

VERIFY before you finish: VERIFY_COMMAND

GOAL:
- (one paragraph)

LOCKED SIGNATURES:
- (paste)
```

## Docs-only prompt (optional)

Use only when the user asked for documents, not code.

```
You own ONE document path. WRITE ONLY that file.

READ FIRST: contract + listed sources.
Do not implement C++. Do not edit siblings. Do not git commit.

Include: purpose, non-goals, sibling links, pitfalls.
If you use mermaid: PascalCase/camelCase node ids, no spaces;
quoted labels with punctuation; subgraph id [Label];
no style, colors, or click.
```

## Stitch checklist

- Build and tests (when the wave included code)
- Signature / name drift vs contract
- Two agents wrote the same path
- New `.cpp` missing from its module `CMakeLists.txt`
- Docs `related_code` current; `last_reviewed` today; `source-layout.md` / `index.md` if structure changed
- Hard gate: `engine/` or `games/` code in the wave diff without any `docs/` or `games/*/docs/` path → incomplete, no commit
- New behavior has a reference/how-to page, not only a source-layout row
- Contract / todos updated with locked decisions
- Docs hard gate or verify failed → no commit
- Verify passed → orchestrator committed owned paths + matching docs (record the hash)
- Dirty tree is only the next wave (or clean). Never staged `build/`
