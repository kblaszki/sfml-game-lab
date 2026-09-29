---
title: Syndicate documentation map
diataxis: reference
audience: [ai, human]
related_code:
  - games/syndicate/CMakeLists.txt
  - games/syndicate/main.cpp
related_docs:
  - explanation/design.md
  - explanation/ui.md
  - reference/sim.md
  - reference/economy.md
  - reference/cards.md
  - reference/auction.md
  - reference/app.md
  - reference/assets.md
  - ../../../docs/index.md
  - ../../../docs/explanation/architecture.md
keywords: [syndicate, board game, docs, index, diataxis]
last_reviewed: 2026-09-29
---

# Syndicate documentation

Hot-seat economic board game on the `sgl` engine: `namespace sgl::syndicate`, targets `syndicate_sim`, `syndicate_app`, and executable `syndicate`. Engine docs live in [docs/index.md](../../../docs/index.md).

| Quadrant | Document | Keywords |
|----------|----------|----------|
| Tutorials | none yet | |
| How-to | none yet | |
| Reference | [reference/sim.md](reference/sim.md) | Money, BoardGraph, moveOptions, Match, Bot |
| Reference | [reference/economy.md](reference/economy.md) | Market, Ownership, landingFee, dividends |
| Reference | [reference/cards.md](reference/cards.md) | Deck, Hand, CardKind, CardEffect |
| Reference | [reference/auction.md](reference/auction.md) | Auction, TradeOffer, validateTrade |
| Reference | [reference/app.md](reference/app.md) | scenes, LayoutView, HandView, Presentation, ActivityLog |
| Reference | [reference/assets.md](reference/assets.md) | atlas, SpriteBank, PNG, procedural fallback |
| Explanation | [explanation/design.md](explanation/design.md) | rules contract, rings, economy, cards, phases |
| Explanation | [explanation/ui.md](explanation/ui.md) | procedural chrome, hybrid atlas, animators, layout |
