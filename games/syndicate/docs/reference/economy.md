---
title: Syndicate economy
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Market.hpp
  - games/syndicate/sim/include/syndicate/sim/Ownership.hpp
  - games/syndicate/sim/include/syndicate/sim/Economy.hpp
  - games/syndicate/sim/src/Market.cpp
  - games/syndicate/sim/src/Ownership.cpp
  - games/syndicate/sim/src/Economy.cpp
  - games/syndicate/sim/parts/economy.cmake
  - tests/syndicate/sim/MarketTest.cpp
  - tests/syndicate/sim/EconomyTest.cpp
  - tests/syndicate/sim/parts/economy.cmake
related_docs:
  - sim.md
  - ../explanation/design.md
  - ../explanation/economy.md
  - ../explanation/board-graph.md
  - ../../../../docs/reference/source-layout.md
keywords: [syndicate, economy, Market, Ownership, landingFee, sharePrice, dividends, netWorth, liquidation]
last_reviewed: 2026-09-29
---

# Syndicate economy

Integer-only market, share ownership, and fee math in `namespace sgl::syndicate`. Headers: `<syndicate/sim/Market.hpp>`, `<syndicate/sim/Ownership.hpp>`, `<syndicate/sim/Economy.hpp>`. Wired into `syndicate_sim` via optional `parts/economy.cmake`. Rules overview: [games/syndicate/docs/explanation/design.md](../explanation/design.md).

## Ownership

`TileHoldings` stores per-player share counts (`kMaxPlayers`), `bank` free shares, and upgrade `level` (0..3). Invariant: sum of player shares + bank == 10.

`Ownership(tileCount)` initializes every tile to bank 10, level 0. `freeShares` is the bank count. `controller(tile)` returns the `PlayerId` holding at least 6 shares, else empty.

## Market

`Market::index` is six `Percent` values, one per `Sector` ordinal (`std::to_underlying(Sector)`), defaulting to 100.

| Function | Behavior |
|----------|----------|
| `clampIndex` | Each index clamped to **[50, 200]** |
| `revertTowardCenter` | `delta = (100 - v) / 10` (trunc toward 0); `v += delta`; then clamp |
| `applyShift(market, sector, delta)` | Add `delta` to that sector, then clamp |

## Share price

`sharePrice(board, market, tile)` — `SectorTile` only:

```
unit = basePrice / 10   (integer)
price = scale(unit, index[sector])
```

Non-sector tiles return `Money{0}`.

## Landing fee

`landingFee` returns `FeeSplit{total, toPlayer[]}`. Non-sector → all zeros.

For a `SectorTile`:

```
levelMul = Percent{100 + 50 * level}
synergy  = Percent{100} if no controller
         = Percent{100 + 20 * (controlled - 1)} otherwise
           where controlled = count of tiles with the same Sector
           whose controller() equals this tile's controller
F = scale(scale(scale(baseFee, levelMul), index[sector]), synergy)
otherPlayerShares = sum of shares of players ≠ payer  (bank ignored)
total = F * otherPlayerShares / 10
```

If `otherPlayerShares == 0`, total is 0 (owning all player-held shares, or only bank left, pays nothing).

Payout: each other player with shares gets `floor(playerShares * total / otherPlayerShares)`. Remainder `total − sum(floors)` goes to the other player with the most shares on the tile; ties → lowest `PlayerId`. Payer’s `toPlayer` slot is always 0. Conservation: `sum(toPlayer) == total`.

## Dividends

`dividends(board, ownership, market, player)` sums, over every `SectorTile` share the player holds:

```
(scale(baseFee, index[sector]).v / 10) * shares
```

## Upgrade cost

`upgradeCost(board, tile, currentLevel)`:

```
scale(basePrice, Percent{50}) * (currentLevel + 1)
```

Returns `Money{0}` when the tile is non-sector or `currentLevel >= 3` (not upgradable / invalid).

Cumulative spend to reach level `L` is `sum_{k=0}^{L-1} upgradeCost(board, tile, k)`.

## Net worth

```
cash
+ sum over tiles of (sharePrice * shares held)
+ if player controls the tile: scale(cumulativeUpgradeSpend, Percent{50})
```

Cash may be negative (debt); with no assets, net worth is that negative cash.

## Liquidation

`liquidationPlan(board, ownership, market, player, debt)` returns deterministic `Sale{tile, count}` lots. Empty when `debt <= 0`.

Sells the player’s shares cheapest-first by `sharePrice`, then lowest `NodeId`. Proceeds per share are `scale(price, Percent{80})`. Sells only what the player holds; the last lot may be partial. Does not mutate ownership.
