#pragma once

#include <span>
#include <string>
#include <string_view>
#include <syndicate/sim/GameState.hpp>

namespace sgl::syndicate
{

struct ControlRow
{
    std::string_view phase;
    std::string_view input;
    std::string_view effect;
};

[[nodiscard]] std::span<const ControlRow> controlMap();

[[nodiscard]] std::string hintLine(const Phase& phase);

} // namespace sgl::syndicate
