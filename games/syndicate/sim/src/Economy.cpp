#include <algorithm>
#include <syndicate/sim/Economy.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] int countControlledInSector(const BoardGraph& board,
                                          const Ownership& ownership,
                                          PlayerId controller,
                                          Sector sector)
{
    int count = 0;
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const auto* tile = std::get_if<SectorTile>(&board.node(id).kind);
        if(tile == nullptr || tile->sector != sector)
        {
            continue;
        }
        const auto ctrl = ownership.controller(id);
        if(ctrl.has_value() && *ctrl == controller)
        {
            ++count;
        }
    }
    return count;
}

[[nodiscard]] Money cumulativeUpgradeSpend(const BoardGraph& board, NodeId tile, std::uint8_t level)
{
    Money sum{};
    for(std::uint8_t k = 0; k < level; ++k)
    {
        sum = sum + upgradeCost(board, tile, k);
    }
    return sum;
}

} // namespace

FeeSplit landingFee(const BoardGraph& board,
                    const Ownership& ownership,
                    const Market& market,
                    NodeId tile,
                    PlayerId payer)
{
    FeeSplit out{};
    const auto* sector = std::get_if<SectorTile>(&board.node(tile).kind);
    if(sector == nullptr)
    {
        return out;
    }

    const auto& h = ownership.holdings(tile);
    const Percent levelMul{100 + 50 * static_cast<std::int32_t>(h.level)};
    Percent synergy{100};
    if(const auto ctrl = ownership.controller(tile); ctrl.has_value())
    {
        const int controlled = countControlledInSector(board, ownership, *ctrl, sector->sector);
        synergy = Percent{100 + 20 * (controlled - 1)};
    }

    const Money f =
        scale(scale(scale(sector->baseFee, levelMul), market.index[std::to_underlying(sector->sector)]), synergy);

    int otherPlayerShares = 0;
    for(std::size_t p = 0; p < kMaxPlayers; ++p)
    {
        if(PlayerId{static_cast<std::uint8_t>(p)} == payer)
        {
            continue;
        }
        otherPlayerShares += h.shares[p];
    }
    if(otherPlayerShares == 0)
    {
        return out;
    }

    out.total = Money{f.v * otherPlayerShares / 10};

    Money distributed{};
    for(std::size_t p = 0; p < kMaxPlayers; ++p)
    {
        const auto pid = PlayerId{static_cast<std::uint8_t>(p)};
        if(pid == payer || h.shares[p] == 0)
        {
            continue;
        }
        const Money share{h.shares[p] * out.total.v / otherPlayerShares};
        out.toPlayer[p] = share;
        distributed = distributed + share;
    }

    const Money remainder = out.total - distributed;
    if(remainder.v != 0)
    {
        std::size_t best = kMaxPlayers;
        std::uint8_t bestShares = 0;
        for(std::size_t p = 0; p < kMaxPlayers; ++p)
        {
            const auto pid = PlayerId{static_cast<std::uint8_t>(p)};
            if(pid == payer)
            {
                continue;
            }
            if(best == kMaxPlayers || h.shares[p] > bestShares
               || (h.shares[p] == bestShares && p < best))
            {
                best = p;
                bestShares = h.shares[p];
            }
        }
        if(best < kMaxPlayers)
        {
            out.toPlayer[best] = out.toPlayer[best] + remainder;
        }
    }

    return out;
}

Money dividends(const BoardGraph& board, const Ownership& ownership, const Market& market, PlayerId player)
{
    Money sum{};
    const auto p = std::to_underlying(player);
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const auto* sector = std::get_if<SectorTile>(&board.node(id).kind);
        if(sector == nullptr)
        {
            continue;
        }
        const auto shares = ownership.holdings(id).shares[p];
        if(shares == 0)
        {
            continue;
        }
        const auto perShare = scale(sector->baseFee, market.index[std::to_underlying(sector->sector)]).v / 10;
        sum = sum + Money{perShare * shares};
    }
    return sum;
}

Money upgradeCost(const BoardGraph& board, NodeId tile, std::uint8_t currentLevel)
{
    const auto* sector = std::get_if<SectorTile>(&board.node(tile).kind);
    if(sector == nullptr || currentLevel >= 3)
    {
        return Money{0};
    }
    return scale(sector->basePrice, Percent{50}) * static_cast<int>(currentLevel + 1);
}

Money netWorth(const BoardGraph& board,
               const Ownership& ownership,
               const Market& market,
               PlayerId player,
               Money cash)
{
    Money worth = cash;
    const auto p = std::to_underlying(player);
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const auto shares = ownership.holdings(id).shares[p];
        if(shares != 0)
        {
            worth = worth + sharePrice(board, market, id) * static_cast<int>(shares);
        }
        const auto ctrl = ownership.controller(id);
        if(ctrl.has_value() && *ctrl == player)
        {
            const auto spend = cumulativeUpgradeSpend(board, id, ownership.holdings(id).level);
            worth = worth + scale(spend, Percent{50});
        }
    }
    return worth;
}

std::vector<Sale> liquidationPlan(const BoardGraph& board,
                                  const Ownership& ownership,
                                  const Market& market,
                                  PlayerId player,
                                  Money debt)
{
    std::vector<Sale> plan;
    if(debt.v <= 0)
    {
        return plan;
    }

    struct Lot
    {
        NodeId tile;
        std::uint8_t held;
        Money price;
    };
    std::vector<Lot> lots;
    const auto p = std::to_underlying(player);
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const auto held = ownership.holdings(id).shares[p];
        if(held == 0)
        {
            continue;
        }
        lots.push_back(Lot{id, held, sharePrice(board, market, id)});
    }
    std::sort(lots.begin(), lots.end(), [](const Lot& a, const Lot& b) {
        if(a.price != b.price)
        {
            return a.price < b.price;
        }
        return a.tile < b.tile;
    });

    Money raised{};
    for(const auto& lot : lots)
    {
        if(raised.v >= debt.v)
        {
            break;
        }
        const Money unitProceeds = scale(lot.price, Percent{80});
        if(unitProceeds.v <= 0)
        {
            // Zero-price shares never raise cash; skip.
            continue;
        }
        std::uint8_t sell = 0;
        while(sell < lot.held && raised.v < debt.v)
        {
            raised = raised + unitProceeds;
            ++sell;
        }
        if(sell > 0)
        {
            plan.push_back(Sale{lot.tile, sell});
        }
    }
    return plan;
}

} // namespace sgl::syndicate
