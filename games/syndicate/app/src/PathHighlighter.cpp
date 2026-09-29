#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <numbers>
#include <set>
#include <sgl/core/Color.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <syndicate/app/PathHighlighter.hpp>
#include <utility>
#include <vector>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] sgl::Vec2f unitOrDefault(sgl::Vec2f delta) noexcept
{
    if(const auto n = sgl::normalized(delta))
    {
        return *n;
    }
    return sgl::Vec2f{1.f, 0.f};
}

} // namespace

void PathHighlighter::build(std::span<const MoveOption> options, const BoardGraph& board, const LayoutView& layout)
{
    options_.assign(options.begin(), options.end());
    nodeRadius_ = layout.nodeRadius();
    model_.destinations.clear();
    model_.arrows.clear();
    model_.hovered = std::nullopt;
    destinationCenters_.clear();
    optionPathPoints_.clear();
    optionPathPoints_.reserve(options_.size());

    std::set<NodeId> uniqueDest;
    for(const MoveOption& opt: options_)
    {
        if(uniqueDest.insert(opt.destination).second)
        {
            model_.destinations.push_back(opt.destination);
            destinationCenters_.push_back(layout.position(board.node(opt.destination).at));
        }

        std::vector<sgl::Vec2f> points;
        points.reserve(opt.path.size());
        for(const NodeId id: opt.path)
        {
            points.push_back(layout.position(board.node(id).at));
        }
        optionPathPoints_.push_back(std::move(points));
    }

    std::set<std::pair<NodeId, NodeId>> arrowEdges;
    std::size_t maxLen = 0;
    for(const MoveOption& opt: options_)
    {
        maxLen = std::max(maxLen, opt.path.size());
    }

    for(std::size_t i = 0; i + 1 < maxLen; ++i)
    {
        std::map<std::vector<NodeId>, std::vector<std::size_t>> byPrefix;
        for(std::size_t oi = 0; oi < options_.size(); ++oi)
        {
            const auto& path = options_[oi].path;
            if(path.size() <= i + 1)
            {
                continue;
            }
            std::vector<NodeId> prefix(path.begin(), path.begin() + static_cast<std::ptrdiff_t>(i) + 1);
            byPrefix[std::move(prefix)].push_back(oi);
        }

        for(const auto& [prefix, indices]: byPrefix)
        {
            if(indices.size() < 2)
            {
                continue;
            }
            std::set<NodeId> nexts;
            for(const std::size_t oi: indices)
            {
                nexts.insert(options_[oi].path[i + 1]);
            }
            if(nexts.size() < 2)
            {
                continue;
            }

            const NodeId from = prefix.back();
            const sgl::Vec2f fromPos = layout.position(board.node(from).at);
            for(const NodeId to: nexts)
            {
                if(!arrowEdges.insert({from, to}).second)
                {
                    continue;
                }
                const sgl::Vec2f toPos = layout.position(board.node(to).at);
                const sgl::Vec2f delta = toPos - fromPos;
                model_.arrows.push_back(Arrow{.tip = fromPos + delta * 0.6f,
                                              .direction = unitOrDefault(delta),
                                              .from = from,
                                              .to = to});
            }
        }
    }
}

void PathHighlighter::update(sgl::Seconds dt, sgl::Vec2f pointer)
{
    t_ += dt.count();
    constexpr float kTwoPi = 2.f * std::numbers::pi_v<float>;
    model_.pulse = 1.f + 0.12f * std::sin(kTwoPi * 1.2f * t_);

    model_.hovered = std::nullopt;
    const float maxDist = nodeRadius_ * 1.5f;
    const float maxDistSq = maxDist * maxDist;
    for(std::size_t i = 0; i < destinationCenters_.size(); ++i)
    {
        if(sgl::lengthSq(pointer - destinationCenters_[i]) <= maxDistSq)
        {
            model_.hovered = i;
            break;
        }
    }
}

const HighlightModel& PathHighlighter::model() const noexcept
{
    return model_;
}

void PathHighlighter::render(sgl::RenderQueue& queue, const Theme& theme) const
{
    float depth = 5000.f;
    const float glowOuter = nodeRadius_ * 2.2f * model_.pulse;
    const float glowInner = std::max(nodeRadius_ * 0.35f * model_.pulse, 1.f);

    for(std::size_t i = 0; i < destinationCenters_.size(); ++i)
    {
        queue.push(sgl::Layer::World, depth++,
                   sgl::radialGlow(destinationCenters_[i], glowInner, glowOuter, theme.highlight));
    }

    const float arrowSize = std::max(nodeRadius_ * 0.9f, 8.f);
    for(const Arrow& arrow: model_.arrows)
    {
        queue.push(sgl::Layer::World, depth++,
                   sgl::arrowHead(arrow.tip, arrow.direction, arrowSize, theme.highlight));
    }

    if(!model_.hovered.has_value())
    {
        return;
    }
    const NodeId dest = model_.destinations[*model_.hovered];
    for(std::size_t oi = 0; oi < options_.size(); ++oi)
    {
        if(options_[oi].destination != dest)
        {
            continue;
        }
        if(optionPathPoints_[oi].size() < 2)
        {
            break;
        }
        queue.push(sgl::Layer::World, depth++,
                   sgl::polyline(optionPathPoints_[oi], 5.f, theme.highlight));
        break;
    }
}

} // namespace sgl::syndicate
