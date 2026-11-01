#include <sgl/input/Key.hpp>
#include <snake/app/Bindings.hpp>

namespace sgl::snake
{

SnakeActions bindSnake(sgl::ActionMap& map)
{
    SnakeActions actions{
        .moveUp = sgl::ActionId{.id = 1},
        .moveDown = sgl::ActionId{.id = 2},
        .moveLeft = sgl::ActionId{.id = 3},
        .moveRight = sgl::ActionId{.id = 4},
        .confirm = sgl::ActionId{.id = 5},
        .cancel = sgl::ActionId{.id = 6},
        .pause = sgl::ActionId{.id = 7},
    };

    map.bind(sgl::Key::Up, actions.moveUp);
    map.bind(sgl::Key::W, actions.moveUp);
    map.bind(sgl::Key::Down, actions.moveDown);
    map.bind(sgl::Key::S, actions.moveDown);
    map.bind(sgl::Key::Left, actions.moveLeft);
    map.bind(sgl::Key::A, actions.moveLeft);
    map.bind(sgl::Key::Right, actions.moveRight);
    map.bind(sgl::Key::D, actions.moveRight);
    map.bind(sgl::Key::Enter, actions.confirm);
    map.bind(sgl::Key::Backspace, actions.cancel);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::P, actions.pause);
    return actions;
}

sgl::MenuActions menuActions(const SnakeActions& actions)
{
    return sgl::MenuActions{
        .up = actions.moveUp,
        .down = actions.moveDown,
        .confirm = actions.confirm,
        .cancel = actions.cancel,
    };
}

} // namespace sgl::snake
