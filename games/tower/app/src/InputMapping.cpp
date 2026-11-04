#include <tower/app/InputMapping.hpp>

namespace sgl::tower
{

SimInput toSimInput(const sgl::InputState& input, const TowerActions& actions)
{
    SimInput sim;
    sim.left = input.action(actions.left).held;
    sim.right = input.action(actions.right).held;
    sim.jump = input.action(actions.jump).pressed;
    return sim;
}

} // namespace sgl::tower
