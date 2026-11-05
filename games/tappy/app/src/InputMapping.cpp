#include <tappy/app/InputMapping.hpp>

namespace sgl::tappy
{

SimInput toSimInput(const sgl::InputState& input, const TappyActions& actions)
{
    SimInput sim{};
    sim.flap = input.action(actions.flap).pressed;
    return sim;
}

} // namespace sgl::tappy
