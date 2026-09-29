#include <syndicate/app/Bindings.hpp>
#include <sgl/input/Key.hpp>

namespace sgl::syndicate
{

SyndicateActions bindSyndicate(sgl::ActionMap& map)
{
    const SyndicateActions actions{
        .roll = sgl::ActionId{.id = 100},
        .endTurn = sgl::ActionId{.id = 101},
        .bid = sgl::ActionId{.id = 102},
        .passBid = sgl::ActionId{.id = 103},
        .pause = sgl::ActionId{.id = 104},
        .menu =
            sgl::MenuActions{
                .up = sgl::ActionId{.id = 105},
                .down = sgl::ActionId{.id = 106},
                .confirm = sgl::ActionId{.id = 107},
                .cancel = sgl::ActionId{.id = 108},
            },
    };

    map.bind(sgl::Key::Space, actions.roll);
    map.bind(sgl::Key::Enter, actions.endTurn);
    map.bind(sgl::Key::C, actions.bid); // Key::B unavailable
    map.bind(sgl::Key::P, actions.passBid);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::Up, actions.menu.up);
    map.bind(sgl::Key::Down, actions.menu.down);
    map.bind(sgl::Key::Enter, actions.menu.confirm);
    map.bind(sgl::Key::Backspace, actions.menu.cancel);
    return actions;
}

} // namespace sgl::syndicate
