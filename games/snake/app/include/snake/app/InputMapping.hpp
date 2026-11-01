#pragma once

#include <sgl/input/InputState.hpp>
#include <snake/app/Bindings.hpp>
#include <snake/sim/SnakeGame.hpp>

namespace sgl::snake
{

// Pressed edges only. When several directions land in one frame, the first of Up, Down, Left, Right wins.
[[nodiscard]] SimInput toSimInput(const sgl::InputState& input, const SnakeActions& actions);

} // namespace sgl::snake
