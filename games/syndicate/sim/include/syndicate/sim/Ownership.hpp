#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <syndicate/sim/Ids.hpp>
#include <vector>

namespace sgl::syndicate
{

struct TileHoldings
{
    std::array<std::uint8_t, kMaxPlayers> shares{};
    std::uint8_t bank{10};
    std::uint8_t level{};
};

class Ownership
{
public:
    explicit Ownership(std::size_t tileCount);

    [[nodiscard]] TileHoldings& holdings(NodeId id);
    [[nodiscard]] const TileHoldings& holdings(NodeId id) const;
    [[nodiscard]] std::uint8_t freeShares(NodeId id) const;
    [[nodiscard]] std::optional<PlayerId> controller(NodeId id) const;

private:
    std::vector<TileHoldings> tiles_;
};

} // namespace sgl::syndicate
