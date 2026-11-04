#pragma once

#include <sgl/input/InputState.hpp>
#include <tower/app/Bindings.hpp>
#include <tower/sim/Types.hpp>

namespace sgl::tower
{

// Held left and right. Jump is the pressed edge, so a key that stays down does not jump again.
[[nodiscard]] SimInput toSimInput(const sgl::InputState& input, const TowerActions& actions);

} // namespace sgl::tower
