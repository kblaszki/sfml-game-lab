#pragma once

#include <array>
#include <cstdint>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <vector>

namespace sgl::syndicate
{

struct FeeSplit
{
    Money total;
    std::array<Money, kMaxPlayers> toPlayer{};
};

[[nodiscard]] FeeSplit landingFee(const BoardGraph& board,
                                  const Ownership& ownership,
                                  const Market& market,
                                  NodeId tile,
                                  PlayerId payer);

[[nodiscard]] Money dividends(const BoardGraph& board,
                              const Ownership& ownership,
                              const Market& market,
                              PlayerId player);

[[nodiscard]] Money upgradeCost(const BoardGraph& board, NodeId tile, std::uint8_t currentLevel);

[[nodiscard]] Money netWorth(const BoardGraph& board,
                             const Ownership& ownership,
                             const Market& market,
                             PlayerId player,
                             Money cash);

struct Sale
{
    NodeId tile;
    std::uint8_t count;
};

[[nodiscard]] std::vector<Sale> liquidationPlan(const BoardGraph& board,
                                                const Ownership& ownership,
                                                const Market& market,
                                                PlayerId player,
                                                Money debt);

} // namespace sgl::syndicate
