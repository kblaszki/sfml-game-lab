#include <syndicate/sim/Ownership.hpp>
#include <utility>

namespace sgl::syndicate
{

Ownership::Ownership(std::size_t tileCount)
    : tiles_(tileCount)
{
}

TileHoldings& Ownership::holdings(NodeId id)
{
    return tiles_[static_cast<std::size_t>(id)];
}

const TileHoldings& Ownership::holdings(NodeId id) const
{
    return tiles_[static_cast<std::size_t>(id)];
}

std::uint8_t Ownership::freeShares(NodeId id) const
{
    return holdings(id).bank;
}

std::optional<PlayerId> Ownership::controller(NodeId id) const
{
    const auto& h = holdings(id);
    for(std::size_t p = 0; p < kMaxPlayers; ++p)
    {
        if(h.shares[p] >= 6)
        {
            return PlayerId{static_cast<std::uint8_t>(p)};
        }
    }
    return std::nullopt;
}

} // namespace sgl::syndicate
