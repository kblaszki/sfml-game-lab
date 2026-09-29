#pragma once

#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::arkanoid
{

struct Actions
{
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
    sgl::ActionId up{};
    sgl::ActionId down{};
    sgl::AxisId paddle{};
};

[[nodiscard]] Actions makeActions();
[[nodiscard]] sgl::ActionMap defaultBindings(const Actions& actions);
[[nodiscard]] sgl::MenuActions menuActions(const Actions& actions);

} // namespace sgl::arkanoid
