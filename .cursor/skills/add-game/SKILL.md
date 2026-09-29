---
name: add-game
description: >-
  Creates a new game under games/<name>/ (headless sim, app scenes, executable,
  tests, and per-game Diataxis docs) on the sgl engine. Use when the user asks
  to add, scaffold, or start a new game or title.
---

# Add a game

Source of truth: [docs/tutorials/create_new_game.md](../../../docs/tutorials/create_new_game.md) (a worked, compiled example) and [docs/how-to/add-a-game.md](../../../docs/how-to/add-a-game.md) (the checklist). Model the layout on a sibling: `games/tetris/` for a small game, `games/syndicate/` for a large one, along with its `games/<game>/docs/`.

## Checklist

```
- [ ] games/<name>/{CMakeLists.txt, main.cpp, sim/, app/} + add_subdirectory in games/CMakeLists.txt
- [ ] <name>_sim links sgl_core (+ sgl_collision only if needed); no SFML, no clock, Pcg32 only
- [ ] Sim tests first under tests/<name>/sim/ incl. same-seed determinism; add_subdirectory in tests/CMakeLists.txt
- [ ] <name>_app: Actions + defaultBindings (ActionMap), scenes implementing SceneI, factories
- [ ] Pause via SceneStack{pauseFactory} + RequestPause on a pausable scene
- [ ] App tests under tests/<name>/app/ (SceneStack + InputState, or mocks + App::runFrames)
- [ ] main.cpp: SfmlPlatform, assets (std::expected checked), SceneStack, App{...}.run()
- [ ] add_dependencies(<name> check_boundaries); build <name> + build_ut; ctest --preset debug
- [ ] games/<name>/docs/index.md + reference/sim.md + reference/app.md + explanation with Mermaid stateDiagram-v2
- [ ] Row in docs/index.md "Game documentation"; targets/suites in docs/reference/source-layout.md; README executable
```

## Rules

- Headless First (`headless-first.mdc`): rules are in the sim and tested before any drawing.
- Namespace `sgl::<name>`. Includes are `<<name>/sim/X.hpp>` and `<<name>/app/X.hpp>`.
- Scenes never close the platform. Quitting is `QuitApp`. Menu cancel quits; overlay cancel resumes.
- Commit each verified step (golden rule), with explicit `git add` paths and never `build/`.
