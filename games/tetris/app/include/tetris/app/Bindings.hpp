#pragma once

#include <sgl/input/ActionId.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/ui/MenuActions.hpp>

namespace sgl::tetris
{

struct Actions
{
    sgl::ActionId confirm{};
    sgl::ActionId cancel{};
    sgl::ActionId pause{};
    sgl::ActionId left{};
    sgl::ActionId right{};
    sgl::ActionId softDrop{};
    sgl::ActionId hardDrop{};
    sgl::ActionId rotateCw{};
    sgl::ActionId rotateCcw{};
    sgl::ActionId hold{};
};

[[nodiscard]] Actions makeActions();
[[nodiscard]] sgl::ActionMap defaultBindings(const Actions& actions);
[[nodiscard]] sgl::MenuActions menuActions(const Actions& actions);

} // namespace sgl::tetris
