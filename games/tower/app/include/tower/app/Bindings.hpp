#pragma once

#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::tower
{

struct TowerActions
{
    sgl::ActionId left{};
    sgl::ActionId right{};
    sgl::ActionId jump{};
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
};

[[nodiscard]] TowerActions bindTower(sgl::ActionMap& map);
[[nodiscard]] sgl::MenuActions menuActions(const TowerActions& actions);

} // namespace sgl::tower
