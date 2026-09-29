#pragma once

#include <cstddef>
#include <optional>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <span>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Pathing.hpp>
#include <vector>

namespace sgl::syndicate
{

struct Arrow
{
    sgl::Vec2f tip;
    sgl::Vec2f direction;
    NodeId from;
    NodeId to;
};

struct HighlightModel
{
    std::vector<NodeId> destinations;
    std::vector<Arrow> arrows;
    std::optional<std::size_t> hovered;
    float pulse{1.f};
};

class PathHighlighter
{
public:
    void build(std::span<const MoveOption> options, const BoardGraph& board, const LayoutView& layout);
    void update(sgl::Seconds dt, sgl::Vec2f pointer);
    [[nodiscard]] const HighlightModel& model() const noexcept;
    void render(sgl::RenderQueue& queue, const Theme& theme) const;

private:
    HighlightModel model_{};
    std::vector<MoveOption> options_{};
    std::vector<sgl::Vec2f> destinationCenters_{};
    std::vector<std::vector<sgl::Vec2f>> optionPathPoints_{};
    float nodeRadius_{};
    float t_{};
};

} // namespace sgl::syndicate
