#include <dodger/app/InputMapping.hpp>

namespace sgl::dodger
{

SimInput toSimInput(const sgl::InputState& input, const DodgerActions& actions)
{
    SimInput sim;
    sim.left = input.action(actions.left).held;
    sim.right = input.action(actions.right).held;
    return sim;
}

} // namespace sgl::dodger
