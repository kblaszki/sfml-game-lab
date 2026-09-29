#pragma once

#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::syndicate
{

/// Syndicate play / menu actions.
/// Keys: Space roll, Enter end turn, B bid, P pass, Escape pause.
/// Digits are NOT actions — InputState has no digit query; destination pick is pointer-only.
/// Note: `sgl::Key` has no `B`, so bid is bound to `Key::C` until `Key::B` exists.
struct SyndicateActions
{
    sgl::ActionId roll;
    sgl::ActionId endTurn;
    sgl::ActionId bid;
    sgl::ActionId passBid;
    sgl::ActionId pause;
    sgl::MenuActions menu;
};

[[nodiscard]] SyndicateActions bindSyndicate(sgl::ActionMap& map);

} // namespace sgl::syndicate
