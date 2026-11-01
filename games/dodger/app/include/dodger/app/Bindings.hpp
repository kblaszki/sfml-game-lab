#pragma once

#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::dodger
{

struct DodgerActions
{
    sgl::ActionId left{};
    sgl::ActionId right{};
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
};

[[nodiscard]] DodgerActions bindDodger(sgl::ActionMap& map);
[[nodiscard]] sgl::MenuActions menuActions(const DodgerActions& actions);

} // namespace sgl::dodger
