#pragma once

#include <cstdint>
#include <syndicate/sim/Cards.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <syndicate/sim/Trade.hpp>
#include <variant>

namespace sgl::syndicate
{

struct RollDice
{
};

struct BuyMomentum
{
    int delta; // +1 or -1
};

struct ChooseDestination
{
    NodeId destination;
    bool passesPayday;
};

struct BuyShares
{
    std::uint8_t count;
};

struct DeclinePurchase
{
};

struct Bid
{
    Money amount;
};

struct PassBid
{
};

struct PlayCard
{
    CardKind kind;
    CardTarget target;
};

struct ProposeTrade
{
    TradeOffer offer;
};

struct RespondTrade
{
    bool accept;
};

struct Upgrade
{
    NodeId tile;
};

struct SellShares
{
    NodeId tile;
    std::uint8_t count;
};

struct BuyCard
{
};

struct EndTurn
{
};

using Command = std::variant<RollDice,
                             BuyMomentum,
                             ChooseDestination,
                             BuyShares,
                             DeclinePurchase,
                             Bid,
                             PassBid,
                             PlayCard,
                             ProposeTrade,
                             RespondTrade,
                             Upgrade,
                             SellShares,
                             BuyCard,
                             EndTurn>;

} // namespace sgl::syndicate
