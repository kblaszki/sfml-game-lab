#pragma once

#include <sgl/core/Rect.hpp>
#include <sgl/core/Vec2.hpp>
#include <syndicate/sim/BoardGraph.hpp>

namespace sgl::syndicate
{

inline constexpr sgl::Vec2f kBoardOrigin{640.f, 340.f};
inline constexpr float kBoardRadius = 270.f;
inline constexpr sgl::Rect<float> kLeftPanel{{16.f, 16.f}, {324.f, 620.f}};
inline constexpr sgl::Rect<float> kRightChart{{940.f, 16.f}, {324.f, 284.f}};
inline constexpr sgl::Rect<float> kRightLog{{940.f, 316.f}, {324.f, 320.f}};
inline constexpr sgl::Rect<float> kActionBar{{16.f, 652.f}, {1248.f, 56.f}};
inline constexpr sgl::Rect<float> kBoardBounds{
    {kBoardOrigin.x - kBoardRadius, kBoardOrigin.y - kBoardRadius},
    {kBoardRadius * 2.f, kBoardRadius * 2.f}};

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
