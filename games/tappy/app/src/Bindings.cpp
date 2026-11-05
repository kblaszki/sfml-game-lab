#include <sgl/input/Key.hpp>
#include <tappy/app/Bindings.hpp>

namespace sgl::tappy
{

TappyActions bindTappy(sgl::ActionMap& map)
{
    TappyActions actions{
        .flap = sgl::ActionId{.id = 1},
        .menuUp = sgl::ActionId{.id = 2},
        .menuDown = sgl::ActionId{.id = 3},
        .confirm = sgl::ActionId{.id = 4},
        .cancel = sgl::ActionId{.id = 5},
        .pause = sgl::ActionId{.id = 6},
    };

    map.bind(sgl::Key::Space, actions.flap);
    map.bind(sgl::Key::Up, actions.flap);
    map.bind(sgl::Key::W, actions.flap);
    map.bind(sgl::Key::Left, actions.menuUp);
    map.bind(sgl::Key::A, actions.menuUp);
    map.bind(sgl::Key::Right, actions.menuDown);
    map.bind(sgl::Key::D, actions.menuDown);
    map.bind(sgl::Key::Enter, actions.confirm);
    map.bind(sgl::Key::Backspace, actions.cancel);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::P, actions.pause);
    return actions;
}

sgl::MenuActions menuActions(const TappyActions& actions)
{
    return sgl::MenuActions{
        .up = actions.menuUp,
        .down = actions.menuDown,
        .confirm = actions.confirm,
        .cancel = actions.cancel,
    };
}

} // namespace sgl::tappy
