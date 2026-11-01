#pragma once

#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::asteroids
{

struct AsteroidsActions
{
    sgl::ActionId thrust{};
    sgl::ActionId rotateCw{};
    sgl::ActionId rotateCcw{};
    sgl::ActionId fire{};
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
};

[[nodiscard]] AsteroidsActions bindAsteroids(sgl::ActionMap& map);
[[nodiscard]] sgl::MenuActions menuActions(const AsteroidsActions& actions);

} // namespace sgl::asteroids
