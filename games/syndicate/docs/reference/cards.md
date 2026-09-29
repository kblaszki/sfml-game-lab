---
title: Syndicate cards
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Cards.hpp
  - games/syndicate/sim/src/Cards.cpp
  - games/syndicate/sim/parts/cards.cmake
  - tests/syndicate/sim/CardsTest.cpp
  - tests/syndicate/sim/parts/cards.cmake
related_docs:
  - sim.md
  - ../explanation/design.md
keywords: [syndicate, cards, Deck, Hand, CardKind, describeEffect, CardEffect, CardTarget]
last_reviewed: 2026-09-29
---

# Syndicate cards

Headless influence-card helpers in `namespace sgl::syndicate` (`Cards.hpp` / `Cards.cpp`), linked into `syndicate_sim` via `parts/cards.cmake`. No SFML. No dependency on Economy, Market, or Ownership.

## CardKind and targets

Eight kinds. Each play requires a matching `CardTarget` alternative (`validateTarget`):

| Kind | Target |
|------|--------|
| `HostileTakeover` | `TakeoverTarget` (`victim`, `tile`) |
| `Audit` | `PlayerTarget` |
| `MarketCrash` | `SectorTarget` |
| `ShortSqueeze` | `SectorTarget` |
| `Lobbying` | `TileTarget` |
| `Embargo` | `TileTarget` |
| `Subsidy` | `NoTarget` |
| `Insider` | `NoTarget` |

Wrong alternative → `CardError::BadTarget`.

## Effects (`describeEffect`)

Returns `std::vector<CardEffect>` after target validation. A card may produce multiple effects later; this slice emits one effect per kind except where Match adds a second side effect.

| Kind | Effect produced here |
|------|----------------------|
| `HostileTakeover` | `ShareGrab{victim, tile, Money{0}}` |
| `Audit` | `CashDelta{player, Money{0}}` |
| `MarketCrash` | `MarketDelta{sector, -30}` |
| `ShortSqueeze` | `MarketDelta{sector, +30}` only |
| `Lobbying` | `FreeUpgrade{tile}` |
| `Embargo` | `EmbargoSet{tile}` |
| `Subsidy` | `CashDelta{PlayerId{}, Money{200}}` |
| `Insider` | `RevealNext{}` |

### Placeholders Match must resolve

- **HostileTakeover / `ShareGrab::price`**: `Money{0}` here. Match fills the real price (design: 150% of share price) using Economy.
- **Audit / `CashDelta::amount`**: `Money{0}` means “10% of the target’s positive cash”; Match computes and may treat the amount as a payment to the bank (negative cash movement).
- **ShortSqueeze**: only the sector index `+30` is described here. Match also pays the actor **50 per own share** in that sector (second effect, not returned by `describeEffect`).
- **Subsidy / `CashDelta::player`**: `PlayerId{}` is a stand-in; Match substitutes the acting player. Amount `200` is final.
- **Embargo / `EmbargoSet`**: marks the tile; Match applies duration (fees 0 until end of next round).

## Deck

Composition: **2 of each** of the 8 kinds (**16** cards). `fillAndShuffle(Pcg32&)` rebuilds draw, clears discard, Fisher–Yates-shuffles with `Pcg32::uniformBelow`.

`draw(Pcg32&)` pops from draw. If draw is empty, moves discard into draw, reshuffles with the passed rng, then draws. If discard is also empty → `EmptyDeck`. Caller must pass the same rng used for `fillAndShuffle` (or any continuing stream); the deck does not store a pointer to the rng.

`discard(CardKind)` appends to the discard pile. `drawCount` / `discardCount` report pile sizes.

## Hand

`Hand::kMax == 5`. `add` fails with `HandFull` when full. `remove` erases one matching copy and returns whether found. `cards()` is a span over the held kinds.
