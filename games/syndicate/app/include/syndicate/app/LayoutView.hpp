#pragma once

#include <sgl/core/Vec2.hpp>
#include <syndicate/sim/BoardGraph.hpp>

namespace sgl::syndicate
{

/// Maps ring slots to screen positions for a 1280×720 design.
/// `nodeRadius()` is `outerRadius * π * 0.7f / 24` (outer chord half-spacing × 0.7).
class LayoutView
{
public:
    explicit LayoutView(sgl::Vec2f origin, float outerRadius);

    [[nodiscard]] sgl::Vec2f position(RingSlot slot) const;
    [[nodiscard]] float nodeRadius() const;

    [[nodiscard]] sgl::Vec2f origin() const noexcept
    {
        return origin_;
    }
    [[nodiscard]] float outerRadius() const noexcept
    {
        return outerRadius_;
    }

private:
    sgl::Vec2f origin_{};
    float outerRadius_{};
};

} // namespace sgl::syndicate
