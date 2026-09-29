#include <cmath>
#include <cstdint>
#include <numbers>
#include <syndicate/app/LayoutView.hpp>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] constexpr std::uint8_t ringSlotCount(std::uint8_t ring) noexcept
{
    switch(ring)
    {
        case 0:
            return 24;
        case 1:
            return 12;
        case 2:
            return 4;
        default:
            return 1;
    }
}

[[nodiscard]] float ringRadius(float outerRadius, std::uint8_t ring) noexcept
{
    switch(ring)
    {
        case 0:
            return outerRadius;
        case 1:
            return outerRadius * 0.62f;
        case 2:
            return outerRadius * 0.30f;
        default:
            return outerRadius;
    }
}

} // namespace

LayoutView::LayoutView(sgl::Vec2f origin, float outerRadius)
    : origin_{origin}
    , outerRadius_{outerRadius}
{
}

sgl::Vec2f LayoutView::position(RingSlot slot) const
{
    const auto count = static_cast<float>(ringSlotCount(slot.ring));
    const float angle = static_cast<float>(slot.slot) / count * 2.f * std::numbers::pi_v<float>;
    const float r = ringRadius(outerRadius_, slot.ring);
    // Slot 0 at top (-Y / smaller screen Y when Y grows down).
    return sgl::Vec2f{origin_.x + r * std::sin(angle), origin_.y - r * std::cos(angle)};
}

float LayoutView::nodeRadius() const
{
    // Outer circumference / 24 / 2 * 0.7 == outerRadius * π * 0.7 / 24
    return outerRadius_ * std::numbers::pi_v<float> * 0.7f / 24.f;
}

} // namespace sgl::syndicate
