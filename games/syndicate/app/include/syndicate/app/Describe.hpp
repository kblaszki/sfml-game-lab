#pragma once

#include <string>
#include <string_view>
#include <syndicate/sim/Cards.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>

namespace sgl::syndicate
{

[[nodiscard]] std::string formatMoney(Money money);

[[nodiscard]] std::string_view sectorName(Sector sector);

struct Label
{
    std::string text;
    std::string tooltip;
};

[[nodiscard]] Label describeCommand(const GameState& state, PlayerId actor, const Command& command);

[[nodiscard]] std::string describeTile(const GameState& state, NodeId tile);

[[nodiscard]] std::string describeCard(CardKind kind);

} // namespace sgl::syndicate
