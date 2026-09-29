#include <cstddef>
#include <syndicate/sim/Auction.hpp>
#include <utility>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::size_t playerIndex(PlayerId id) noexcept
{
    return static_cast<std::size_t>(std::to_underlying(id));
}

[[nodiscard]] int activeCount(const AuctionState& state) noexcept
{
    int n = 0;
    for(int i = 0; i < state.seats; ++i)
    {
        if(state.active[static_cast<std::size_t>(i)])
        {
            ++n;
        }
    }
    return n;
}

[[nodiscard]] PlayerId nextActiveAfter(const AuctionState& state, PlayerId from) noexcept
{
    const int seats = state.seats;
    const auto start = static_cast<int>(std::to_underlying(from));
    for(int step = 1; step <= seats; ++step)
    {
        const int idx = (start + step) % seats;
        if(state.active[static_cast<std::size_t>(idx)])
        {
            return PlayerId{static_cast<std::uint8_t>(idx)};
        }
    }
    return from;
}

[[nodiscard]] std::expected<void, AuctionError> requireActor(const AuctionState& state, PlayerId who)
{
    if(activeCount(state) == 0)
    {
        return std::unexpected(AuctionError::Closed);
    }
    if(who != state.toAct)
    {
        return std::unexpected(AuctionError::NotYourTurn);
    }
    if(!state.active[playerIndex(who)])
    {
        return std::unexpected(AuctionError::NotActive);
    }
    return {};
}

} // namespace

AuctionState startAuction(NodeId tile,
                          std::uint8_t shares,
                          Money openingBid,
                          int seats,
                          PlayerId firstToAct,
                          const std::array<bool, kMaxPlayers>& eligible)
{
    AuctionState state;
    state.tile = tile;
    state.shares = shares;
    state.highBid = openingBid;
    state.leader = std::nullopt;
    state.seats = seats;
    state.active = {};
    for(int i = 0; i < seats; ++i)
    {
        state.active[static_cast<std::size_t>(i)] = eligible[static_cast<std::size_t>(i)];
    }

    state.toAct = firstToAct;
    if(seats > 0 && !state.active[playerIndex(firstToAct)])
    {
        state.toAct = nextActiveAfter(state, firstToAct);
    }
    return state;
}

std::expected<AuctionOutcome, AuctionError> bid(AuctionState& state, PlayerId who, Money amount, Money cashOf)
{
    if(auto gate = requireActor(state, who); !gate)
    {
        return std::unexpected(gate.error());
    }

    const Money required = state.leader.has_value() ? state.highBid + Money{10} : state.highBid;
    if(amount < required)
    {
        return std::unexpected(AuctionError::BidTooLow);
    }
    if(cashOf < amount)
    {
        return std::unexpected(AuctionError::CannotAfford);
    }

    state.leader = who;
    state.highBid = amount;

    if(activeCount(state) == 1)
    {
        return Sold{*state.leader, state.highBid};
    }

    state.toAct = nextActiveAfter(state, who);
    return AuctionContinue{};
}

std::expected<AuctionOutcome, AuctionError> passBid(AuctionState& state, PlayerId who)
{
    if(auto gate = requireActor(state, who); !gate)
    {
        return std::unexpected(gate.error());
    }

    state.active[playerIndex(who)] = false;
    const int remaining = activeCount(state);

    if(remaining == 0)
    {
        if(state.leader.has_value())
        {
            return Sold{*state.leader, state.highBid};
        }
        return Unsold{};
    }

    if(remaining == 1 && state.leader.has_value() && state.active[playerIndex(*state.leader)])
    {
        return Sold{*state.leader, state.highBid};
    }

    state.toAct = nextActiveAfter(state, who);
    return AuctionContinue{};
}

} // namespace sgl::syndicate
