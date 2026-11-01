#include <asteroids/app/Bindings.hpp>
#include <sgl/input/Key.hpp>

namespace sgl::asteroids
{

AsteroidsActions bindAsteroids(sgl::ActionMap& map)
{
    AsteroidsActions actions{
        .thrust = sgl::ActionId{.id = 1},
        .rotateCw = sgl::ActionId{.id = 2},
        .rotateCcw = sgl::ActionId{.id = 3},
        .fire = sgl::ActionId{.id = 4},
        .confirm = sgl::ActionId{.id = 5},
        .cancel = sgl::ActionId{.id = 6},
        .pause = sgl::ActionId{.id = 7},
    };

    map.bind(sgl::Key::Up, actions.thrust);
    map.bind(sgl::Key::W, actions.thrust);
    map.bind(sgl::Key::Right, actions.rotateCw);
    map.bind(sgl::Key::D, actions.rotateCw);
    map.bind(sgl::Key::Left, actions.rotateCcw);
    map.bind(sgl::Key::A, actions.rotateCcw);
    map.bind(sgl::Key::Space, actions.fire);
    map.bind(sgl::Key::Enter, actions.confirm);
    map.bind(sgl::Key::Backspace, actions.cancel);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::P, actions.pause);
    return actions;
}

sgl::MenuActions menuActions(const AsteroidsActions& actions)
{
    return sgl::MenuActions{
        .up = actions.rotateCcw,
        .down = actions.rotateCw,
        .confirm = actions.confirm,
        .cancel = actions.cancel,
    };
}

} // namespace sgl::asteroids
