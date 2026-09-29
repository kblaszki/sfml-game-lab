#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <optional>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>

namespace sgl::syndicate
{

struct AuctionState
{
    NodeId tile{};
    std::uint8_t shares{};
    Money highBid{};
    std::optional<PlayerId> leader{};
    std::array<bool, kMaxPlayers> active{};
    PlayerId toAct{};
    int seats{}; // 2..4, only players < seats exist
};

enum class AuctionError
{
    NotYourTurn,
    BidTooLow,
    CannotAfford,
    NotActive,
    Closed,
};

struct Sold
{
    PlayerId winner;
    Money price;
};

struct Unsold
{
};

struct AuctionContinue
{
};

using AuctionOutcome = std::variant<AuctionContinue, Sold, Unsold>;

[[nodiscard]] AuctionState startAuction(NodeId tile,
                                        std::uint8_t shares,
                                        Money openingBid,
                                        int seats,
                                        PlayerId firstToAct,
                                        const std::array<bool, kMaxPlayers>& eligible);

[[nodiscard]] std::expected<AuctionOutcome, AuctionError> bid(AuctionState& state,
                                                              PlayerId who,
                                                              Money amount,
                                                              Money cashOf);

[[nodiscard]] std::expected<AuctionOutcome, AuctionError> passBid(AuctionState& state, PlayerId who);

} // namespace sgl::syndicate
