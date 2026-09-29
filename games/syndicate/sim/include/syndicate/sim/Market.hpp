#pragma once

#include <array>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>

namespace sgl::syndicate
{

struct Market
{
    std::array<Percent, 6> index{};
};

void clampIndex(Market& market) noexcept;
void revertTowardCenter(Market& market) noexcept;
void applyShift(Market& market, Sector sector, int delta);

[[nodiscard]] Money sharePrice(const BoardGraph& board, const Market& market, NodeId tile);

} // namespace sgl::syndicate
