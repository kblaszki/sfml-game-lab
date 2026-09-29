#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sgl/render/Shapes.hpp>
#include <sgl/ui/TextLayout.hpp>
#include <sgl/ui/TooltipSystem.hpp>

namespace sgl
{
namespace
{

constexpr float kScreenW{1280.f};
constexpr float kScreenH{720.f};
constexpr float kPointerOffsetX{16.f};
constexpr float kPointerOffsetY{20.f};
constexpr float kPad{8.f};
constexpr float kTitleGap{4.f};
constexpr float kMinWidth{40.f};
constexpr float kFadeDuration{0.1f};
constexpr float kCornerRadius{8.f};

[[nodiscard]] Color applyAlpha(Color color, float alpha)
{
    const float a = std::clamp(alpha, 0.f, 1.f);
    return Color{color.r, color.g, color.b, static_cast<std::uint8_t>(std::lround(static_cast<float>(color.a) * a))};
}

[[nodiscard]] float longestLineWidth(const TooltipView& view, const TooltipStyle& style)
{
    float widest = estimateWidth(view.title, style.titleSize);
    for(const std::string& line: view.lines)
    {
        widest = std::max(widest, estimateWidth(line, style.bodySize));
    }
    return widest;
}

[[nodiscard]] float panelHeight(const TooltipView& view, const TooltipStyle& style)
{
    float content = static_cast<float>(style.titleSize);
    if(!view.lines.empty())
    {
        content += kTitleGap + static_cast<float>(view.lines.size()) * static_cast<float>(style.bodySize);
    }
    return content + kPad * 2.f;
}

void placePanel(TooltipView& view, Vec2f pointer, const TooltipStyle& style)
{
    const float width = std::max(kMinWidth, std::min(style.maxWidth, longestLineWidth(view, style)) + kPad * 2.f);
    const float height = panelHeight(view, style);

    Vec2f pos{pointer.x + kPointerOffsetX, pointer.y + kPointerOffsetY};
    if(pos.x + width > kScreenW)
    {
        pos.x = pointer.x - kPointerOffsetX - width;
    }
    if(pos.y + height > kScreenH)
    {
        pos.y = pointer.y - kPointerOffsetY - height;
    }
    pos.x = std::clamp(pos.x, 0.f, kScreenW - width);
    pos.y = std::clamp(pos.y, 0.f, kScreenH - height);

    view.panel = Rect<float>{.pos = pos, .size = {width, height}};
}

} // namespace

void TooltipSystem::update(Vec2f pointer, std::span<const Hotspot> hotspots, Seconds dt, const TooltipStyle& style)
{
    std::optional<std::size_t> hit;
    for(std::size_t i = 0; i < hotspots.size(); ++i)
    {
        if(hotspots[i].area.contains(pointer))
        {
            hit = i;
            break;
        }
    }

    if(!hit)
    {
        hotspotIndex_ = std::nullopt;
        hoverTime_ = 0.f;
        view_ = std::nullopt;
        return;
    }

    if(!hotspotIndex_ || *hotspotIndex_ != *hit)
    {
        hotspotIndex_ = hit;
        hoverTime_ = 0.f;
        view_ = std::nullopt;
    }

    hoverTime_ += dt.count();
    if(hoverTime_ < style.delay)
    {
        view_ = std::nullopt;
        return;
    }

    const Hotspot& hotspot = hotspots[*hit];
    if(!view_)
    {
        TooltipView next;
        next.title = hotspot.title;
        next.lines = wrap(hotspot.body, style.bodySize, style.maxWidth);
        next.alpha = 0.f;
        view_ = next;
    }

    view_->alpha = std::min(1.f, view_->alpha + dt.count() / kFadeDuration);
    placePanel(*view_, pointer, style);
}

std::optional<TooltipView> TooltipSystem::active() const
{
    return view_;
}

void TooltipSystem::render(RenderQueue& queue, const TooltipStyle& style) const
{
    if(!view_)
    {
        return;
    }

    const TooltipView& view = *view_;
    float depth = 0.f;

    MeshCmd panel = roundedRect(view.panel, kCornerRadius, applyAlpha(style.panel, view.alpha));
    queue.push(Layer::Overlay, depth++, panel);

    queue.push(Layer::Overlay,
               depth++,
               TextCmd{
                   .font = style.font,
                   .text = view.title,
                   .size = style.titleSize,
                   .position = {view.panel.pos.x + kPad, view.panel.pos.y + kPad},
                   .color = applyAlpha(style.title, view.alpha),
                   .anchor = Anchor::TopLeft,
               });

    float y = view.panel.pos.y + kPad + static_cast<float>(style.titleSize) + kTitleGap;
    for(const std::string& line: view.lines)
    {
        queue.push(Layer::Overlay,
                   depth++,
                   TextCmd{
                       .font = style.font,
                       .text = line,
                       .size = style.bodySize,
                       .position = {view.panel.pos.x + kPad, y},
                       .color = applyAlpha(style.body, view.alpha),
                       .anchor = Anchor::TopLeft,
                   });
        y += static_cast<float>(style.bodySize);
    }
}

} // namespace sgl
