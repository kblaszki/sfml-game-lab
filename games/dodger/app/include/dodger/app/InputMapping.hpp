#pragma once

#include <dodger/app/Bindings.hpp>
#include <dodger/sim/Types.hpp>
#include <sgl/input/InputState.hpp>

namespace sgl::dodger
{

// Held keys. Left and right stay active for as long as the key is down.
[[nodiscard]] SimInput toSimInput(const sgl::InputState& input, const DodgerActions& actions);

} // namespace sgl::dodger
