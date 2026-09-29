---
title: Syndicate sim
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/sim/include/syndicate/sim/Money.hpp
  - games/syndicate/sim/include/syndicate/sim/Ids.hpp
  - games/syndicate/sim/include/syndicate/sim/BoardGraph.hpp
  - games/syndicate/sim/include/syndicate/sim/BoardLayouts.hpp
  - games/syndicate/sim/include/syndicate/sim/Pathing.hpp
  - games/syndicate/sim/include/syndicate/sim/Match.hpp
  - games/syndicate/sim/include/syndicate/sim/GameState.hpp
  - games/syndicate/sim/include/syndicate/sim/Commands.hpp
  - games/syndicate/sim/include/syndicate/sim/Events.hpp
  - games/syndicate/sim/include/syndicate/sim/Bot.hpp
  - games/syndicate/sim/src/BoardGraph.cpp
  - games/syndicate/sim/src/BoardLayouts.cpp
  - games/syndicate/sim/src/Pathing.cpp
  - games/syndicate/sim/src/Match.cpp
  - games/syndicate/sim/src/Bot.cpp
  - games/syndicate/sim/parts/match.cmake
  - games/syndicate/sim/parts/bot.cmake
  - games/syndicate/sim/CMakeLists.txt
  - tests/syndicate/sim/MoneyTest.cpp
  - tests/syndicate/sim/BoardGraphTest.cpp
  - tests/syndicate/sim/BoardLayoutsTest.cpp
  - tests/syndicate/sim/PathingTest.cpp
  - tests/syndicate/sim/MatchTest.cpp
  - tests/syndicate/sim/MatchPropertyTest.cpp
  - tests/syndicate/sim/BotTest.cpp
  - tests/syndicate/sim/parts/match.cmake
  - tests/syndicate/sim/parts/bot.cmake
  - tests/syndicate/sim/CMakeLists.txt
related_docs:
  - ../explanation/syndicate-design.md
  - syndicate-economy.md
  - syndicate-cards.md
  - syndicate-auction.md
  - source-layout.md
  - ../explanation/architecture.md
keywords: [syndicate, sim, Money, BoardGraph, standardBoard, moveOptions, Match, turn machine, Payday, CSR, NodeId, Bot, BotPersona]
last_reviewed: 2026-09-29
---

# Syndicate sim

Headless board/pathing foundation in `namespace sgl::syndicate`. Static library `syndicate_sim` links `PUBLIC` `sgl_core` only. No SFML. Include as `<syndicate/sim/X.hpp>`. Rules overview: [syndicate-design.md](../explanation/syndicate-design.md).

Optional CMake part hooks (`parts/{economy,cards,auction,match,bot}.cmake`) are included after the library definition for later parallel tasks; this slice lists its own sources directly.

## Money (`Money.hpp`)

| Name | Role |
|------|------|
| `Money` | `std::int64_t v`; `<=>`, `+`, `-`, `* int` / `int *` |
| `Percent` | `std::int32_t v` default 100 |
| `scale(m, p)` | `Money{m.v * p.v / 100}` truncates toward 0 |

## Ids (`Ids.hpp`)

| Name | Role |
|------|------|
| `NodeId` | `enum class : std::uint16_t` with `<=>` |
| `PlayerId` | `enum class : std::uint8_t` |
| `kMaxPlayers` | 4 |
| `Sector` | AI, CleanEnergy, Biotech, CyberSec, Logistics, Fintech |

## BoardGraph (`BoardGraph.hpp`)

CSR adjacency: `offsets_` (`n+1` `uint32`), `targets_` (`NodeId`).

`BoardGraph::build(nodes, edges) -> expected<BoardGraph, BoardError>`:

| Error | When |
|-------|------|
| `DuplicateId` | Same `NodeId` twice |
| `NonDenseIds` | Ids not exactly `0..n-1` |
| `DanglingEdge` | Endpoint ≥ n |
| `DeadEnd` | Node with zero successors |
| `NotStronglyConnected` | Directed graph not strongly connected |
| `PaydayCount` | Number of `Payday` tiles ≠ 1 |

API: `successors(NodeId)`, `node(NodeId)`, `size()`.

`TileKind = variant<SectorTile, Payday, Exchange, Hub, AuditTile, Grant>`.

## standardBoard (`BoardLayouts.hpp`)

Returns a 40-node graph; throws `std::logic_error` only if `build` fails (should not happen).

### Topology

| Ring | Ids | Slots | Directed edges |
|------|-----|-------|----------------|
| Outer (0) | 0..23 | 0..23 | `i → (i+1)%24` CW |
| Inner (1) | 24..35 | 0..11 | slot `s → (s+11)%12` CCW; id `24+slot` |
| Core (2) | 36..39 | 0..3 | slot CW; id `36+slot` |

Bidirectional bridges: outer 0/6/12/18 ↔ inner 0/3/6/9 (ids 24/27/30/33); inner 0/6 (24/30) ↔ core 0/2 (36/38).

Eight nodes have out-degree > 1 at ring–ring bridges (outer `0,6,12,18` and inner `24,27,30,33`); core bridge ends `36,38` also have out-degree > 1 (ten junctions total).

### Tile assignment (constants in `BoardLayouts.cpp`)

Outer prices/fees: 100/10. Inner: 150/15. Core: 300/30. Audit tax 10%. Grant amount 100.

| Id | Ring/slot | Kind |
|----|-----------|------|
| 0 | 0/0 | **Payday** |
| 1 | 0/1 | Sector AI |
| 2 | 0/2 | Sector CleanEnergy |
| 3 | 0/3 | Sector Biotech |
| 4 | 0/4 | Exchange |
| 5 | 0/5 | Sector CyberSec |
| 6 | 0/6 | Sector Logistics |
| 7 | 0/7 | Sector Fintech |
| 8 | 0/8 | Hub |
| 9 | 0/9 | Sector AI |
| 10 | 0/10 | Sector CleanEnergy |
| 11 | 0/11 | AuditTile |
| 12 | 0/12 | Sector Biotech |
| 13 | 0/13 | Sector CyberSec |
| 14 | 0/14 | Grant |
| 15 | 0/15 | Sector Logistics |
| 16 | 0/16 | Sector Fintech |
| 17 | 0/17 | Exchange |
| 18 | 0/18 | Sector AI |
| 19 | 0/19 | Sector CleanEnergy |
| 20 | 0/20 | Hub |
| 21 | 0/21 | Sector Biotech |
| 22 | 0/22 | Sector CyberSec |
| 23 | 0/23 | Grant |
| 24 | 1/0 | Sector Logistics |
| 25 | 1/1 | Sector Fintech |
| 26 | 1/2 | Exchange |
| 27 | 1/3 | Sector AI |
| 28 | 1/4 | Hub |
| 29 | 1/5 | Sector CleanEnergy |
| 30 | 1/6 | Sector Biotech |
| 31 | 1/7 | AuditTile |
| 32 | 1/8 | Sector CyberSec |
| 33 | 1/9 | Sector Logistics |
| 34 | 1/10 | Grant |
| 35 | 1/11 | Sector Fintech |
| 36 | 2/0 | Sector AI (premium) |
| 37 | 2/1 | Sector CleanEnergy (premium) |
| 38 | 2/2 | Sector Biotech (premium) |
| 39 | 2/3 | Sector CyberSec (premium) |

`node(NodeId{0})` is always Payday.

## Pathing (`Pathing.hpp`)

`moveOptions(board, from, cameFrom, steps) -> vector<MoveOption>`.

- Walks exactly `steps` edges (cap 12; `steps < 0` → empty; `steps == 0` → stay, `passesPayday` false).
- No immediate reversal on any step (`cameFrom` blocks the first step; later steps never reverse the edge just taken).
- `path` length is `steps+1` and includes the start node.
- `passesPayday` iff some node **after** the start equals the unique Payday node.
- Dedupe by `(destination, passesPayday)` keeping the lexicographically smaller path; sort by destination, then `passesPayday` (false before true), then path.

## Turn machine (`Match.hpp`)

Closed command API over `GameState` in `namespace sgl::syndicate`. Headers: `Match.hpp`, `GameState.hpp`, `Commands.hpp`, `Events.hpp`. Wired via `parts/match.cmake`. Calls Economy / Cards / Auction / Trade; does not reimplement fee, deck, or auction math.

| API | Role |
|-----|------|
| `Match::create(MatchConfig)` | Seats **2..4** else `SetupError::BadSeats`. `standardBoard()`, cash/`startCash`, all at Payday node 0, market indexes 100, influence `Deck::fillAndShuffle`, 12-card market-event deck (±20 per sector), phase `AwaitRoll`, acting seat 0, round 1. One `Pcg32` from `config.seed`. |
| `apply(actor, Command)` | Only `actingPlayer()`; `GameOver` / wrong phase → `WrongPhase`; else command-specific `InvalidArgument` / `InsufficientFunds` / `NotYourTurn`. |
| `legalCommands(actor)` | Empty if not acting or `GameOver`. Every returned command must `apply` successfully (property tests rely on this). `ProposeTrade` is omitted from the enumeration. |
| `drainEvents()` | Moves out and clears the event buffer. |

### Phase flow

`AwaitRoll` → `AwaitPath` (2d4 via `nextU32()%4+1`, optional Momentum ±1 for 25) → `ChooseDestination` → optional payday (salary 200 + dividends, then 10% interest if still negative) → land resolve:

| Landing | Next phase |
|---------|------------|
| `SectorTile` free shares > 0 | `AwaitPurchase` (buy up to `min(3,free)` or decline) |
| `SectorTile` free == 0 | fee (embargo zeroes; else `landingFee` + optional `liquidationPlan`) → `Manage` |
| `Hub` | draw into hand if not full → `Manage` |
| `Exchange` / `Grant` / `AuditTile` / `Payday` | resolve tile effect → `Manage` |

Decline with ≥3 free opens `InAuction` (3-share block at 50% mark). After auction Sold/Unsold → `Manage`. Manage allows one card, up to three trades (hot-seat `AwaitTradeResponse`), upgrade/sell/buy-card, then `EndTurn`. Last seat ending a round: `round++`, draw/apply one market event (`applyShift` then `revertTowardCenter`; Insider peek consumes that card), then game-over check (`round > maxRounds` or any `netWorth >= target`) else next seat `AwaitRoll`.

### Events

`GameEvent` variant includes at least: `Rolled`, `Moved`, `PaydayPaid`, `FeePaid`, `Liquidated`, `SharesBought`, `AuctionDone`, `CardPlayed`, `TradeResolved`, `MarketShifted`, `GameEnded`.

### Tests

`syndicate_match_test` (seeded scenarios, each `CommandError`, max-rounds `GameOver`). `syndicate_match_property_test` (seeds 1..200, 4 seats, 8 rounds, share invariant bank+players==10). Full `Match` fork is not available; first-step legality is re-checked on fresh creates with the same seed.

## Bot (`Bot.hpp`)

Deterministic heuristic over `Match::legalCommands`. Wired via `parts/bot.cmake`.

| API | Role |
|-----|------|
| `BotPersona` | `riskAppetite` (`Percent`, default 100) and `cashReserve` (`Money`, default 200). |
| `chooseCommand(match, self, persona)` | Scores each legal command with fixed integers; picks the max score (lowest index on ties). No RNG. Empty legal → `EndTurn{}`. |

Heuristic highlights: `RollDice` 100; `ChooseDestination` path/fee/payday/free-share bonuses; `BuyShares` / `Bid` respect `cashReserve` (else −100); `Upgrade` 35 if affordable; `PlayCard` 25; `BuyMomentum` 10; `PassBid` 5; `DeclinePurchase` 1; `EndTurn` and other commands 0.

Tests: `syndicate_bot_test` (fresh `RollDice`, 50 seeds × 4 seats × 6 rounds to `GameOver` with share invariant, no `BuyShares` into negative cash, chosen command always legal).
