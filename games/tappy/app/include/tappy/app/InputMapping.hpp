#pragma once

#include <sgl/input/InputState.hpp>
#include <tappy/app/Bindings.hpp>
#include <tappy/sim/Types.hpp>

namespace sgl::tappy
{

// Flap is the pressed edge so holding the key does not spam impulses.
[[nodiscard]] SimInput toSimInput(const sgl::InputState& input, const TappyActions& actions);

} // namespace sgl::tappy
