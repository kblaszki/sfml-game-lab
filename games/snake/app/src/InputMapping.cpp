#include <snake/app/InputMapping.hpp>

namespace sgl::snake
{

SimInput toSimInput(const sgl::InputState& input, const SnakeActions& actions)
{
    SimInput sim;
    if(input.action(actions.moveUp).pressed)
    {
        sim.turn = Dir::Up;
    }
    else if(input.action(actions.moveDown).pressed)
    {
        sim.turn = Dir::Down;
    }
    else if(input.action(actions.moveLeft).pressed)
    {
        sim.turn = Dir::Left;
    }
    else if(input.action(actions.moveRight).pressed)
    {
        sim.turn = Dir::Right;
    }
    return sim;
}

} // namespace sgl::snake
