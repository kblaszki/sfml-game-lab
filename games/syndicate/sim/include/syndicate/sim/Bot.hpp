#pragma once

#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>

namespace sgl::syndicate
{

struct BotPersona
{
    Percent riskAppetite{100};
    Money cashReserve{200};
};

[[nodiscard]] Command chooseCommand(const Match& match, PlayerId self, const BotPersona& persona);

} // namespace sgl::syndicate
