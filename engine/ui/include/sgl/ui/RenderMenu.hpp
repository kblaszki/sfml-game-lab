#pragma once

#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/HoverAnim.hpp>
#include <sgl/ui/MenuModel.hpp>
#include <sgl/ui/MenuStyle.hpp>

namespace sgl
{

void renderMenu(RenderQueue& queue, const MenuModel& model, const MenuStyle& style, Layer layer);

void renderMenu(RenderQueue& queue, const MenuModel& model, const MenuStyle& style, Layer layer, const HoverAnim& hover);

} // namespace sgl
