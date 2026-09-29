---
title: Add an influence card
diataxis: how-to
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Cards.hpp
  - games/syndicate/sim/src/Cards.cpp
  - games/syndicate/sim/include/syndicate/sim/Commands.hpp
  - games/syndicate/sim/src/Match.cpp
  - games/syndicate/app/include/syndicate/app/Describe.hpp
  - games/syndicate/app/src/Describe.cpp
  - games/syndicate/app/src/HandView.cpp
  - tests/syndicate/sim/CardsTest.cpp
  - tests/syndicate/sim/MatchTest.cpp
  - tests/syndicate/app/DescribeTest.cpp
related_docs:
  - ../reference/cards.md
  - ../reference/sim.md
  - ../explanation/design.md
  - ../explanation/economy.md
keywords: [syndicate, how-to, CardKind, describeEffect, Deck, PlayCard, describeCard]
last_reviewed: 2026-09-29
---

# Add an influence card

Add a new `CardKind`, wire its target/effect, deck weight, Match application, UI text, and tests. Reference facts: [cards.md](../reference/cards.md).

## Steps

1. **`CardKind`** — In `games/syndicate/sim/include/syndicate/sim/Cards.hpp`, add an enumerator to `enum class CardKind`. Extend `CardTarget` / `CardEffect` variants only if the card needs a new target or effect shape.

2. **Target validation** — In `games/syndicate/sim/src/Cards.cpp`, update `targetMatches` (and thus `validateTarget`) so the kind accepts the correct `CardTarget` alternative (`NoTarget`, `SectorTarget`, `PlayerTarget`, `TileTarget`, or `TakeoverTarget`).

3. **`describeEffect`** — In the same file, add a `case` that returns one or more `CardEffect` values after validation. Prefer placeholders (`Money{0}`, `PlayerId{}`) when Match must fill economy-dependent amounts, matching existing HostileTakeover / Audit / Subsidy patterns.

4. **Deck composition** — In `Deck::fillAndShuffle`, `kAllKinds[]` lists every kind twice (16 cards). Append the new kind so `fillAndShuffle` still builds two of each. Update composition assertions in `tests/syndicate/sim/CardsTest.cpp`.

5. **Match application** — In `games/syndicate/sim/src/Match.cpp`:
   - Extend `dummyTargetFor` so `legalCommands` can emit `PlayCard{kind, target}` during `Manage` (at most one card per turn via `cardsPlayed`).
   - Extend `applyCardEffects` to interpret each `CardEffect` (and any Match-only side effects, e.g. ShortSqueeze cash).
   - Emit `CardPlayed` as today when `PlayCard` succeeds.

6. **UI copy** — In `games/syndicate/app/src/Describe.cpp`, add a `describeCard` case (display name). `describeCommand` / `describeEvent` already route through `describeCard` for `PlayCard` / `CardPlayed`. `HandView` tooltips use `describeCard` for title and body; no HandView change unless you need richer tooltip text.

7. **Optional atlas** — If shipping card art, add a region name under `assets/syndicate/` per [assets.md](../reference/assets.md). Procedural fallback remains valid with zero PNGs.

8. **Tests** — Update `tests/syndicate/sim/CardsTest.cpp` (composition, bad targets, `describeEffect`). Add or extend a `MatchTest` play scenario. Cover `describeCard` / `CardPlayed` in `tests/syndicate/app/DescribeTest.cpp` if new event text matters. Rebuild with `syndicate_cards_test` / `syndicate_match_test` / app describe suites as needed.

9. **Docs** — Update [reference/cards.md](../reference/cards.md) (kind table, effects, placeholders) and the design contract table in [explanation/design.md](../explanation/design.md). Set `last_reviewed` on touched docs.

## Checklist of real symbols

| Layer | Files / symbols |
|-------|-----------------|
| Sim kinds | `CardKind`, `CardTarget`, `CardEffect`, `validateTarget`, `describeEffect` |
| Deck | `Deck::fillAndShuffle`, `kAllKinds` |
| Commands | `PlayCard` in `Commands.hpp` |
| Match | `dummyTargetFor`, `applyCardEffects`, `PlayCard` branch in `Match::apply` |
| App | `describeCard`, `HandView` hotspots |
| Tests | `CardsTest.cpp`, `MatchTest.cpp`, `DescribeTest.cpp` |
