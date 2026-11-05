#pragma once

#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::tappy
{

struct TappyActions
{
    sgl::ActionId flap{};
    sgl::ActionId menuUp{};
    sgl::ActionId menuDown{};
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
};

[[nodiscard]] TappyActions bindTappy(sgl::ActionMap& map);
[[nodiscard]] sgl::MenuActions menuActions(const TappyActions& actions);

} // namespace sgl::tappy
