#pragma once

#include <optional>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <span>
#include <string>
#include <vector>

namespace sgl
{

struct Hotspot
{
    Rect<float> area;
    std::string title;
    std::string body;
};

struct TooltipStyle
{
    FontId font{};
    unsigned titleSize{18};
    unsigned bodySize{14};
    float maxWidth{300.f};
    Color panel{24, 28, 40, 235};
    Color border{180, 190, 210, 255};
    Color title{255, 255, 255, 255};
    Color body{210, 216, 224, 255};
    float delay{0.35f};
};

struct TooltipView
{
    Rect<float> panel;
    std::string title;
    std::vector<std::string> lines;
    float alpha{0.f};
};

class TooltipSystem
{
public:
    void update(Vec2f pointer, std::span<const Hotspot> hotspots, Seconds dt, const TooltipStyle& style);
    [[nodiscard]] std::optional<TooltipView> active() const;
    void render(RenderQueue& queue, const TooltipStyle& style) const;

private:
    std::optional<std::size_t> hotspotIndex_;
    float hoverTime_{};
    std::optional<TooltipView> view_;
};

} // namespace sgl
