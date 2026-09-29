#include <syndicate/app/Bindings.hpp>

#include <cstddef>
#include <cstdint>
#include <sgl/input/Key.hpp>

namespace sgl::syndicate
{

SyndicateActions bindSyndicate(sgl::ActionMap& map)
{
    SyndicateActions actions{
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
        .slot = {},
    };

    for(std::size_t i = 0; i < actions.slot.size(); ++i)
    {
        actions.slot[i] = sgl::ActionId{.id = static_cast<std::uint32_t>(110 + i)};
    }

    map.bind(sgl::Key::Space, actions.roll);
    map.bind(sgl::Key::Enter, actions.endTurn);
    map.bind(sgl::Key::C, actions.bid); // Key::B unavailable
    map.bind(sgl::Key::P, actions.passBid);
    map.bind(sgl::Key::Escape, actions.pause);
    map.bind(sgl::Key::Up, actions.menu.up);
    map.bind(sgl::Key::Down, actions.menu.down);
    map.bind(sgl::Key::Enter, actions.menu.confirm);
    map.bind(sgl::Key::Backspace, actions.menu.cancel);

    constexpr sgl::Key kNums[9] = {sgl::Key::Num1,
                                   sgl::Key::Num2,
                                   sgl::Key::Num3,
                                   sgl::Key::Num4,
                                   sgl::Key::Num5,
                                   sgl::Key::Num6,
                                   sgl::Key::Num7,
                                   sgl::Key::Num8,
                                   sgl::Key::Num9};
    for(std::size_t i = 0; i < actions.slot.size(); ++i)
    {
        map.bind(kNums[i], actions.slot[i]);
    }
    return actions;
}

} // namespace sgl::syndicate
