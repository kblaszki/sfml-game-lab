#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <sgl/core/Random.hpp>
#include <syndicate/sim/Auction.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Cards.hpp>
#include <syndicate/sim/Events.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <syndicate/sim/Pathing.hpp>
#include <syndicate/sim/Trade.hpp>
#include <utility>
#include <variant>
#include <vector>

namespace sgl::syndicate
{

struct MatchConfig
{
    std::uint64_t seed{1};
    int seats{2}; // 2..4
    Money startCash{1500};
    int maxRounds{15};
    Money targetNetWorth{6000};
};

struct AwaitRoll
{
};

struct AwaitPath
{
    std::vector<MoveOption> options;
    int roll;
    bool momentumUsed;
};

struct AwaitPurchase
{
    NodeId tile;
    std::uint8_t maxCount;
};

struct InAuction
{
    AuctionState auction;
};

struct AwaitTradeResponse
{
    TradeOffer offer;
    int cardsPlayed;
    int tradesProposed;
};

struct Manage
{
    int cardsPlayed;
    int tradesProposed;
};

struct RankingEntry
{
    PlayerId player;
    Money net;
    Money cash;
};

struct GameOver
{
    std::vector<RankingEntry> ranking;
};

using Phase = std::variant<AwaitRoll, AwaitPath, AwaitPurchase, InAuction, AwaitTradeResponse, Manage, GameOver>;

struct MarketEventCard
{
    Sector sector;
    int delta;

    bool operator==(const MarketEventCard&) const = default;
};

struct GameState
{
    BoardGraph board;
    Ownership ownership;
    Market market;
    Deck deck;
    std::array<Hand, kMaxPlayers> hands{};
    std::array<Money, kMaxPlayers> cash{};
    std::array<NodeId, kMaxPlayers> position{};
    std::array<std::optional<NodeId>, kMaxPlayers> cameFrom{};
    Phase phase{AwaitRoll{}};
    PlayerId acting{PlayerId{0}};
    PlayerId turnOwner{PlayerId{0}}; // whose turn; acting follows auction toAct
    int round{1};
    int seats{};
    sgl::Pcg32 rng;
    std::vector<GameEvent> events;
    MatchConfig config;
    std::vector<std::pair<NodeId, int>> embargoUntilRound;
    std::vector<MarketEventCard> marketDeck;
    std::optional<MarketEventCard> revealed;
};

} // namespace sgl::syndicate
