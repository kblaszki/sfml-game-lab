#pragma once

#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>

namespace sgl
{

void renderMenu(RenderQueue& queue, const MenuModel& model, const MenuStyle& style, Layer layer);

} // namespace sgl
