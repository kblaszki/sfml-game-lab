#include <algorithm>
#include <sgl/anim/Easing.hpp>
#include <sgl/ui/HoverAnim.hpp>

namespace sgl
{
namespace
{

constexpr float kIdleScale{1.f};
constexpr float kHoverScale{1.06f};
constexpr float kDuration{0.06f};

} // namespace

void HoverAnim::update(std::optional<std::size_t> hovered, std::size_t count, Seconds dt)
{
    scales_.resize(count, kIdleScale);

    const float t = std::min(1.f, dt.count() / kDuration);
    const float e = easeOutCubic(t);

    for(std::size_t i = 0; i < scales_.size(); ++i)
    {
        const float target = (hovered && *hovered == i) ? kHoverScale : kIdleScale;
        scales_[i] += (target - scales_[i]) * e;
    }
}

float HoverAnim::scale(std::size_t index) const
{
    if(index >= scales_.size())
    {
        return kIdleScale;
    }
    return scales_[index];
}

} // namespace sgl
