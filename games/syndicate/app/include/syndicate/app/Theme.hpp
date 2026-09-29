#pragma once

#include <array>
#include <sgl/core/Color.hpp>

namespace sgl::syndicate
{

struct Theme
{
    std::array<sgl::Color, 6> sector; // AI, CleanEnergy, Biotech, CyberSec, Logistics, Fintech
    sgl::Color edge;
    sgl::Color payday, exchange, hub, audit, grant;
    sgl::Color highlight;
    sgl::Color pawn[4];
};

[[nodiscard]] Theme defaultTheme();

} // namespace sgl::syndicate
