#pragma once

#include <sgl/core/Rect.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/ui/TooltipSystem.hpp>
#include <syndicate/app/MarketHistory.hpp>
#include <syndicate/app/Theme.hpp>
#include <vector>

namespace sgl::syndicate
{

void renderTrendChart(sgl::RenderQueue& queue,
                      const MarketHistory& history,
                      sgl::Rect<float> area,
                      const Theme& theme,
                      sgl::FontId font);

[[nodiscard]] std::vector<sgl::Hotspot> chartHotspots(const MarketHistory& history, sgl::Rect<float> area);

} // namespace sgl::syndicate
