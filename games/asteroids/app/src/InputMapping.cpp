#include <asteroids/app/InputMapping.hpp>

namespace sgl::asteroids
{

SimInput toSimInput(const sgl::InputState& input, const AsteroidsActions& actions)
{
    SimInput sim;
    sim.thrust = input.action(actions.thrust).held;
    sim.rotateCw = input.action(actions.rotateCw).held;
    sim.rotateCcw = input.action(actions.rotateCcw).held;
    sim.fire = input.action(actions.fire).held;
    return sim;
}

} // namespace sgl::asteroids
