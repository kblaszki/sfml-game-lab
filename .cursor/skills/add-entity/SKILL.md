---
name: add-entity
description: >-
  Adds a new type under engine/ or games/<name>/, wires it into that module's
  CMakeLists, covers it with a windowless test, and documents it. Use when
  creating a library type in sgl, sgl::arkanoid, sgl::tetris, sgl::syndicate,
  or a new game.
---

# Add a type to an engine or game module

Source of truth: [docs/reference/source-layout.md](../../../docs/reference/source-layout.md), [docs/explanation/architecture.md](../../../docs/explanation/architecture.md), and [docs/reference/engine-api-map.md](../../../docs/reference/engine-api-map.md). For game types, also read `games/<name>/docs/index.md`.

## Checklist

```
- [ ] Decide the owner: SFML-free engine module, <name>_sim (rules), or <name>_app (presentation)
- [ ] Create Header.hpp / Name.cpp under the owning module
- [ ] Add Name.cpp to that module's CMakeLists.txt
- [ ] Add or extend a test under tests/engine/<module>/ or tests/<name>/<sim|app>/
- [ ] Register the test with add_unit_test if it is a new suite
- [ ] Add or update the Diataxis reference entry (engine: docs/reference/engine-*.md; game: games/<name>/docs/reference/*.md)
- [ ] Mermaid diagram if it changes architecture or a state flow
```

## Steps

1. **Owner.** Follow Headless First (`headless-first.mdc`). Rules and mechanics belong in `<name>_sim`. Engine features belong in an SFML-free module. Only adapters belong in `engine/sfml`.

2. **Header.** Use `#pragma once`, `namespace sgl` or `sgl::<name>`, one primary type, and CamelCase:

```cpp
#pragma once

namespace sgl
{

class Name
{
public:
    int doWork() const;
};

} // namespace sgl
```

3. **Implementation.** Keep sim and app code SFML-free. Only `engine/sfml` and `games/<name>/main.cpp` may include SFML. Owners are values or `std::unique_ptr`, never raw `new`.

4. **CMake.** Append `Name.cpp` to the owning module's `CMakeLists.txt`. Without this, the file never builds.

5. **Test.** Write a windowless GoogleTest, registered via `add_unit_test` in the matching tests leaf. See [write-headless-tests.md](../../../docs/how-to/write-headless-tests.md).

6. **Docs.** Every new engine feature or game mechanic gets a Diátaxis entry in the same change. Refresh docs whose `related_code` lists the files you touched, in both `docs/` and `games/*/docs/`, and add new files to `docs/reference/source-layout.md`. Use the `update-docs` skill for a full sweep.
