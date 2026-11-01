#pragma once

#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::snake
{

struct SnakeActions
{
    sgl::ActionId moveUp{};
    sgl::ActionId moveDown{};
    sgl::ActionId moveLeft{};
    sgl::ActionId moveRight{};
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
};

[[nodiscard]] SnakeActions bindSnake(sgl::ActionMap& map);
[[nodiscard]] sgl::MenuActions menuActions(const SnakeActions& actions);

} // namespace sgl::snake
