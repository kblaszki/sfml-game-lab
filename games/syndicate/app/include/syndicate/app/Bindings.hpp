#pragma once

#include <array>
#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::syndicate
{

/// Syndicate play / menu actions.
/// Keys: Space roll, Enter end turn, C bid, P pass, Escape pause, Num1–Num9 action-bar slots.
/// Note: `sgl::Key` has no `B`, so bid is bound to `Key::C` until `Key::B` exists.
struct SyndicateActions
{
    sgl::ActionId roll;
    sgl::ActionId endTurn;
    sgl::ActionId bid;
    sgl::ActionId passBid;
    sgl::ActionId pause;
    sgl::MenuActions menu;
    std::array<sgl::ActionId, 9> slot{};
};

[[nodiscard]] SyndicateActions bindSyndicate(sgl::ActionMap& map);

} // namespace sgl::syndicate
