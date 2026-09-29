---
title: Syndicate auction and trade
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Auction.hpp
  - games/syndicate/sim/include/syndicate/sim/Trade.hpp
  - games/syndicate/sim/src/Auction.cpp
  - games/syndicate/sim/src/Trade.cpp
  - games/syndicate/sim/parts/auction.cmake
  - tests/syndicate/sim/AuctionTest.cpp
  - tests/syndicate/sim/TradeTest.cpp
  - tests/syndicate/sim/parts/auction.cmake
related_docs:
  - sim.md
  - ../explanation/design.md
keywords: [syndicate, auction, trade, bid, passBid, TradeOffer, validateTrade, Sold, Unsold]
last_reviewed: 2026-09-29
---

# Syndicate auction and trade

Headless ascending auction and hot-seat trade validation in `namespace sgl::syndicate`. Depends only on `Money.hpp` and `Ids.hpp`. Wired via optional CMake parts `parts/auction.cmake` (sim sources and unit tests). Design context: [games/syndicate/docs/explanation/design.md](../explanation/design.md).

## Auction (`Auction.hpp`)

`AuctionState` tracks the lot (`tile`, `shares`), `highBid`, optional `leader`, `active` flags, `toAct`, and `seats` (2..4; only players `< seats` exist).

| API | Role |
|-----|------|
| `startAuction(tile, shares, openingBid, seats, firstToAct, eligible)` | Opens the round. `highBid = openingBid`, `leader` empty. Players with `eligible[i] == false` are inactive and skipped. `firstToAct` advances to the next active seat when ineligible. |
| `bid(state, who, amount, cashOf)` | Raise while `who == toAct` and active. |
| `passBid(state, who)` | Drop out (`active[who] = false`). |

`AuctionOutcome = variant<AuctionContinue, Sold, Unsold>`. Errors: `NotYourTurn`, `BidTooLow`, `CannotAfford`, `NotActive`, `Closed` (no active seats left when acting).

### Bid rules

- First bid (no `leader`): `amount >= highBid` (the opening bid; no mandatory +10 over zero).
- Later bids: `amount >= highBid + Money{10}`.
- `cashOf < amount` → `CannotAfford`.
- On success: set `leader` / `highBid`, then advance `toAct` to the next active seat. If only one active player remains after the bid → `Sold{leader, highBid}`.

### Pass rules

After `active[who] = false`:

| Remaining active | Leader | Outcome |
|------------------|--------|---------|
| 0 | none | `Unsold` |
| 0 | set | `Sold{leader, highBid}` |
| 1 and that seat is the leader | set | `Sold{leader, highBid}` |
| otherwise | — | `AuctionContinue` (advance `toAct`) |

## Trade (`Trade.hpp`)

`TradeOffer` exchanges optional cash both ways (`cashFrom` / `cashTo`) and share lots (`sharesFrom` / `sharesTo`). Validation is header-only:

```cpp
template<class SharesOf>
std::expected<void, TradeError>
validateTrade(const TradeOffer&, std::span<const Money> cash, SharesOf sharesOf);
```

`sharesOf(PlayerId, NodeId) -> uint8_t`. Duplicate lots for the same tile are summed before comparing to holdings.

| Error | When |
|-------|------|
| `SamePlayer` | `from == to` |
| `Empty` | No cash either way and both share vectors empty |
| `InsufficientCash` | `cash.size() < kMaxPlayers`, or a side cannot cover its cash leg |
| `InsufficientShares` | A side offers more shares of a tile than `sharesOf` reports |

Validation does not mutate cash or holdings; Match applies accepted trades.
