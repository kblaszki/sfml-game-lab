#pragma once

#include <sgl/input/ActionId.hpp>

namespace sgl
{

struct MenuActions
{
    ActionId up;
    ActionId down;
    ActionId confirm;
    ActionId cancel;
    ActionId left{};
    ActionId right{};
};

} // namespace sgl
