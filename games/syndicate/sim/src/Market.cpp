#include <algorithm>
#include <syndicate/sim/Market.hpp>
#include <utility>

namespace sgl::syndicate
{
namespace
{

constexpr int kIndexMin = 50;
constexpr int kIndexMax = 200;
constexpr int kIndexCenter = 100;

} // namespace

void clampIndex(Market& market) noexcept
{
    for(auto& p : market.index)
    {
        p.v = std::clamp(p.v, kIndexMin, kIndexMax);
    }
}

void revertTowardCenter(Market& market) noexcept
{
    for(auto& p : market.index)
    {
        const int delta = (kIndexCenter - p.v) / 10;
        p.v += delta;
    }
    clampIndex(market);
}

void applyShift(Market& market, Sector sector, int delta)
{
    auto& p = market.index[std::to_underlying(sector)];
    p.v += delta;
    clampIndex(market);
}

Money sharePrice(const BoardGraph& board, const Market& market, NodeId tile)
{
    const auto* sector = std::get_if<SectorTile>(&board.node(tile).kind);
    if(sector == nullptr)
    {
        return Money{0};
    }
    const auto unit = Money{sector->basePrice.v / 10};
    return scale(unit, market.index[std::to_underlying(sector->sector)]);
}

} // namespace sgl::syndicate
