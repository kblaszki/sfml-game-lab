#pragma once

#include <sgl/input/InputState.hpp>
#include <tower/app/Bindings.hpp>
#include <tower/sim/Types.hpp>

namespace sgl::tower
{

// Held left, right, and jump (held jump autojumps on each landing).
[[nodiscard]] SimInput toSimInput(const sgl::InputState& input, const TowerActions& actions);

} // namespace sgl::tower
