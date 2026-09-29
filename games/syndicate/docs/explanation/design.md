---
title: Syndicate design contract
diataxis: explanation
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Money.hpp
  - games/syndicate/sim/include/syndicate/sim/Ids.hpp
  - games/syndicate/sim/include/syndicate/sim/BoardGraph.hpp
  - games/syndicate/sim/include/syndicate/sim/BoardLayouts.hpp
  - games/syndicate/sim/include/syndicate/sim/Pathing.hpp
related_docs:
  - ../reference/sim.md
  - ../../../../docs/reference/source-layout.md
  - ../../../../docs/explanation/architecture.md
keywords: [syndicate, design, board, rings, bridges, economy, shares, auction, cards, phases, debt]
last_reviewed: 2026-09-29
---

# Syndicate: Dynamic Grid — design contract

Hot-seat board game for 2–4 players (any seat may be a deterministic bot). Simulation is integer-only and headless in `sgl::syndicate`. This page is the rules contract; implemented types live in [games/syndicate/docs/reference/sim.md](../reference/sim.md).

## Board: concentric directed rings

- **Outer** ring: 24 nodes, clockwise. **Inner**: 12 nodes, counter-clockwise. **Core**: 4 premium nodes, clockwise. Total **40** nodes.
- **Bridges** (both directions): outer slots 0/6/12/18 ↔ inner 0/3/6/9; inner 0/6 ↔ core 0/2. Nodes with more than one successor are path choice junctions.
- **Tile kinds**: `SectorTile`, exactly one `Payday` (outer slot 0 / node 0), `Exchange`, `Hub`, `AuditTile{tax%}`, `Grant{amount}`.
- **Sectors**: AI, CleanEnergy, Biotech, CyberSec, Logistics, Fintech.

## Movement

Match movement uses **2d4** (sum 2–8). Once per turn a player may buy **Momentum** for 25 to adjust the roll by ±1 (floor 1). The pathing engine returns every walk of exactly that many steps with no immediate edge reversal, deduped by `(destination, passesPayday)` and sorted deterministically. The player then picks a destination.

## Market and economy

- Each sector tile has **10 shares**. **Control** is holding at least **6 of 10**. Only the controller may upgrade (levels 0–3) for `basePrice * 50% * (level+1)`.
- Each sector has a **index** (`Percent`) in **50–200**, starting at 100.
- Share price: `basePrice/10 * index%`.
- Landing fee: `F = baseFee * (100+50*level)% * index% * synergy%`, where `synergy = 100 + 20*(controlled tiles of that sector − 1)`. Payer pays `F * othersShares/10`; payouts split by share count (remainder to the largest holder, lower `PlayerId` on ties). Bank shares collect nothing. Inflows must equal outflows.
- **Payday**: salary 200 plus dividend `baseFee*index%/10` per share. Debt accrues **10%** interest.
- End of round: draw a market event (Pcg32-shuffled deck), shift one or all sectors by ±10..30, then mean-revert toward 100 by `(100-index)/10`, clamp 50–200.
- **No bankruptcy elimination.** If a player cannot pay, shares sell to the bank at 80% price (cheapest first, deterministic); a shortfall becomes **debt** (negative cash). Debtors cannot buy, bid, or upgrade, but still move and collect dividends.
- Game ends after `maxRounds` (default 15) or when someone reaches `targetNetWorth` (default 6000). Net worth = cash + share mark-to-market + 50% of upgrade cost. Ties: cash, then lower `PlayerId`.

## Purchase, auction, trade

- On a sector with free shares, buy up to `min(3, free)`. Declining with ≥3 free starts an auction of a 3-share block.
- Auction: ascending queue; start at 50% of block price; minimum raise +10; begin with the next player; passer is out; ends when one bidder remains or all pass (unsold).
- Trade: one open `TradeOffer` at a time, at most 3 per turn; hot-seat accept/reject by the addressee.

## Influence cards (8 kinds)

Draw at `Hub` and on Payday; buy a card for 100 at `Exchange`. Hand cap 5. Shared draw/discard, reshuffled with Pcg32. At most one card per turn in Manage.

| Kind | Effect |
|------|--------|
| HostileTakeover | Take 1 share from a chosen player at 150% price |
| Audit | Target pays bank 10% of positive cash |
| MarketCrash | Sector index −30 |
| ShortSqueeze | Sector index +30; player gains 50 per own share in that sector |
| Lobbying | Free upgrade on a controlled tile |
| Embargo | Fees on a tile are 0 until end of next round |
| Subsidy | +200 |
| Insider | Peek next market event (only the player sees it) |

## Turn phases

`AwaitRoll` → `AwaitPath` → `Resolve` → (`AwaitPurchase` / `Auction`) → `Manage` (optional trade) → next player / `GameOver`.

Commands are a closed `std::variant` applied through `Match::apply` so UI, bots, tests, and replays share one API.
