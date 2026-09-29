#include <sgl/input/Key.hpp>
#include <tetris/app/Bindings.hpp>

namespace sgl::tetris
{

Actions makeActions()
{
    return Actions{
        .confirm = sgl::ActionId{.id = 1},
        .cancel = sgl::ActionId{.id = 2},
        .pause = sgl::ActionId{.id = 3},
        .left = sgl::ActionId{.id = 4},
        .right = sgl::ActionId{.id = 5},
        .softDrop = sgl::ActionId{.id = 6},
        .hardDrop = sgl::ActionId{.id = 7},
        .rotateCw = sgl::ActionId{.id = 8},
        .rotateCcw = sgl::ActionId{.id = 9},
        .hold = sgl::ActionId{.id = 10},
    };
}

sgl::ActionMap defaultBindings(const Actions& actions)
{
    sgl::ActionMap map;
    map.bind(sgl::Key::Enter, actions.confirm);
    map.bind(sgl::Key::Backspace, actions.cancel);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::P, actions.pause);
    map.bind(sgl::Key::Left, actions.left);
    map.bind(sgl::Key::Right, actions.right);
    map.bind(sgl::Key::Down, actions.softDrop);
    map.bind(sgl::Key::Space, actions.hardDrop);
    map.bind(sgl::Key::Up, actions.rotateCw);
    map.bind(sgl::Key::X, actions.rotateCw);
    map.bind(sgl::Key::Z, actions.rotateCcw);
    map.bind(sgl::Key::C, actions.hold);
    map.bind(sgl::Key::LShift, actions.hold);
    return map;
}

sgl::MenuActions menuActions(const Actions& actions)
{
    return sgl::MenuActions{
        .up = actions.rotateCw,
        .down = actions.softDrop,
        .confirm = actions.confirm,
        .cancel = actions.cancel,
    };
}

} // namespace sgl::tetris
