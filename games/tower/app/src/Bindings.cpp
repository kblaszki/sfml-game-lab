#include <sgl/input/Key.hpp>
#include <tower/app/Bindings.hpp>

namespace sgl::tower
{

TowerActions bindTower(sgl::ActionMap& map)
{
    TowerActions actions{
        .left = sgl::ActionId{.id = 1},
        .right = sgl::ActionId{.id = 2},
        .jump = sgl::ActionId{.id = 3},
        .confirm = sgl::ActionId{.id = 4},
        .cancel = sgl::ActionId{.id = 5},
        .pause = sgl::ActionId{.id = 6},
    };

    map.bind(sgl::Key::Left, actions.left);
    map.bind(sgl::Key::A, actions.left);
    map.bind(sgl::Key::Right, actions.right);
    map.bind(sgl::Key::D, actions.right);
    map.bind(sgl::Key::Space, actions.jump);
    map.bind(sgl::Key::Up, actions.jump);
    map.bind(sgl::Key::W, actions.jump);
    map.bind(sgl::Key::Enter, actions.confirm);
    map.bind(sgl::Key::Backspace, actions.cancel);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::P, actions.pause);
    return actions;
}

sgl::MenuActions menuActions(const TowerActions& actions)
{
    return sgl::MenuActions{
        .up = actions.left,
        .down = actions.right,
        .confirm = actions.confirm,
        .cancel = actions.cancel,
    };
}

} // namespace sgl::tower
