---
title: Syndicate economy rationale
diataxis: explanation
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Market.hpp
  - games/syndicate/sim/include/syndicate/sim/Ownership.hpp
  - games/syndicate/sim/include/syndicate/sim/Economy.hpp
  - games/syndicate/sim/include/syndicate/sim/Match.hpp
  - games/syndicate/sim/include/syndicate/sim/GameState.hpp
  - games/syndicate/sim/src/Market.cpp
  - games/syndicate/sim/src/Economy.cpp
  - games/syndicate/sim/src/Match.cpp
related_docs:
  - design.md
  - board-graph.md
  - ../reference/economy.md
  - ../reference/sim.md
  - ../reference/cards.md
  - ../reference/auction.md
keywords: [syndicate, economy, Market, payday, dividends, landingFee, debt, liquidation, turn phases, Pcg32]
last_reviewed: 2026-09-29
---

# Economy rationale

All money and market indexes are **integer-only** (`Money` is `std::int64_t`; `Percent` scales with truncating division). Formulas and signatures: [reference/economy.md](../reference/economy.md). Turn machine and events: [reference/sim.md](../reference/sim.md).

## Market cycle

Each of the six `Sector` values has an index in \([50, 200]\), starting at 100. End of round (`Match::` `endOfRound` after the last seat ends a turn):

1. Increment `round`.
2. Draw one `MarketEventCard` from a 12-card deck (per sector: \(+20\) and \(-20\)), Fisher–Yates-shuffled with `sgl::Pcg32` (`uniformBelow`). Insider peek may consume a previously revealed top card.
3. `applyShift(market, sector, delta)` then `clampIndex`.
4. `revertTowardCenter`: for each index \(v\),

\[
\Delta = \left\lfloor\frac{100 - v}{10}\right\rfloor,\qquad v \leftarrow v + \Delta
\]

then clamp again to \([50, 200]\).

Card effects (`MarketCrash` −30, `ShortSqueeze` +30) call the same `applyShift` / clamp path mid-turn.

## Shares, price, fees

Each sector tile has **10** shares (players + bank). Control is ≥6 shares. Share price:

\[
\text{unit} = \left\lfloor\frac{\text{basePrice}}{10}\right\rfloor,\qquad
\text{price} = \mathrm{scale}(\text{unit},\,\text{index})
\]

Landing fee \(F\) layers level, index, and synergy, then scales by other players’ share fraction of 10 (bank ignored). Payouts conserve `total` (remainder to largest other holder; lower `PlayerId` on ties). Embargo zeroes fees until end of next round. Exact expressions: [reference/economy.md](../reference/economy.md).

## Payday, dividends, interest

When a chosen path has `passesPayday`, `applyPayday` runs before landing resolve:

1. Salary `Money{200}`.
2. Dividends: per held sector share, \(\lfloor\mathrm{scale}(\text{baseFee},\,\text{index})/10\rfloor\).
3. If cash is still negative, interest \(\lfloor(-\text{cash})/10\rfloor\) (10%) is subtracted.

Landing on the Payday tile itself does not pay again (salary already came from `passesPayday`).

```mermaid
sequenceDiagram
  participant Match
  participant Economy
  participant Player
  Match->>Match: ChooseDestination with passesPayday
  Match->>Economy: dividends board ownership market player
  Economy-->>Match: divs
  Match->>Player: cash += 200 + divs
  alt cash still negative
    Match->>Player: cash -= floor of minus cash over 10
  end
  Match->>Match: emit PaydayPaid then resolveLanding
```

## Purchase, debt, liquidation

Free shares → `AwaitPurchase` (buy up to `min(3, free)` or decline; decline with ≥3 free opens auction). Occupied sector → fee. If cash cannot cover the fee, `liquidationPlan` sells cheapest shares first at 80% mark-to-market to the bank; remaining shortfall becomes **debt** (negative cash). Debtors still move and collect dividends but cannot buy, bid, or upgrade (enforced in `legalCommands` / apply checks).

Net worth = cash + share MTM + 50% of cumulative upgrade spend on controlled tiles. Game over when `round > maxRounds` or any seat reaches `targetNetWorth`.

## Turn phases

```mermaid
stateDiagram-v2
  [*] --> AwaitRoll
  AwaitRoll --> AwaitPath: RollDice
  AwaitPath --> AwaitPath: BuyMomentum
  AwaitPath --> AwaitPurchase: "ChooseDestination sector free"
  AwaitPath --> InAuction: "decline with free >= 3"
  AwaitPath --> Manage: "land Hub Exchange Grant Audit fee Payday"
  AwaitPurchase --> InAuction: "DeclinePurchase free >= 3"
  AwaitPurchase --> Manage: "BuyShares or decline free < 3"
  InAuction --> Manage: "Sold or Unsold"
  Manage --> AwaitTradeResponse: ProposeTrade
  AwaitTradeResponse --> Manage: RespondTrade
  Manage --> AwaitRoll: "EndTurn next seat"
  Manage --> AwaitRoll: "EndTurn last seat then market"
  Manage --> GameOver: "maxRounds or targetNetWorth"
```

Optional Manage actions: one `PlayCard`, up to three trades, upgrade/sell/`BuyCard` at Exchange (100). Hub draws into hand if not full. Commands go through `Match::apply` only.
