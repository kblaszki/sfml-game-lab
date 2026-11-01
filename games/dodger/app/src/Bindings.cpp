#include <dodger/app/Bindings.hpp>
#include <sgl/input/Key.hpp>

namespace sgl::dodger
{

DodgerActions bindDodger(sgl::ActionMap& map)
{
    DodgerActions actions{
        .left = sgl::ActionId{.id = 1},
        .right = sgl::ActionId{.id = 2},
        .confirm = sgl::ActionId{.id = 3},
        .cancel = sgl::ActionId{.id = 4},
        .pause = sgl::ActionId{.id = 5},
    };

    map.bind(sgl::Key::Left, actions.left);
    map.bind(sgl::Key::A, actions.left);
    map.bind(sgl::Key::Right, actions.right);
    map.bind(sgl::Key::D, actions.right);
    map.bind(sgl::Key::Enter, actions.confirm);
    map.bind(sgl::Key::Backspace, actions.cancel);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::P, actions.pause);
    return actions;
}

sgl::MenuActions menuActions(const DodgerActions& actions)
{
    return sgl::MenuActions{
        .up = actions.left,
        .down = actions.right,
        .confirm = actions.confirm,
        .cancel = actions.cancel,
    };
}

} // namespace sgl::dodger
