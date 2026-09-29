#pragma once

#include <cstdint>
#include <syndicate/sim/Cards.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>
#include <vector>

namespace sgl::syndicate
{

struct Rolled
{
    PlayerId player;
    int roll;
};

struct Moved
{
    PlayerId player;
    NodeId from;
    NodeId to;
    bool passesPayday;
};

struct PaydayPaid
{
    PlayerId player;
    Money salary;
    Money dividends;
    Money interest;
};

struct FeePaid
{
    PlayerId payer;
    NodeId tile;
    Money total;
};

struct Liquidated
{
    PlayerId player;
    NodeId tile;
    std::uint8_t count;
    Money proceeds;
};

struct SharesBought
{
    PlayerId player;
    NodeId tile;
    std::uint8_t count;
    Money cost;
};

struct AuctionDone
{
    NodeId tile;
    std::uint8_t shares;
    bool sold;
    PlayerId winner; // meaningful when sold
    Money price;
};

struct CardPlayed
{
    PlayerId player;
    CardKind kind;
};

struct TradeResolved
{
    PlayerId from;
    PlayerId to;
    bool accepted;
};

struct MarketShifted
{
    Sector sector;
    int delta;
};

struct GameEnded
{
};

using GameEvent = std::variant<Rolled,
                               Moved,
                               PaydayPaid,
                               FeePaid,
                               Liquidated,
                               SharesBought,
                               AuctionDone,
                               CardPlayed,
                               TradeResolved,
                               MarketShifted,
                               GameEnded>;

} // namespace sgl::syndicate
