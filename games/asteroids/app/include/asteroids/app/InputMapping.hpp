#pragma once

#include <asteroids/app/Bindings.hpp>
#include <asteroids/sim/Types.hpp>
#include <sgl/input/InputState.hpp>

namespace sgl::asteroids
{

// Held keys. Thrust, rotation, and fire stay active for as long as the key is down.
[[nodiscard]] SimInput toSimInput(const sgl::InputState& input, const AsteroidsActions& actions);

} // namespace sgl::asteroids
