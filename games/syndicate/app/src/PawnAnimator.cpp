#include <cmath>
#include <cstddef>
#include <syndicate/app/PawnAnimator.hpp>
#include <utility>

namespace sgl::syndicate
{
namespace
{

[[nodiscard]] std::size_t playerIndex(PlayerId id) noexcept
{
    return static_cast<std::size_t>(std::to_underlying(id));
}

[[nodiscard]] sgl::Vec2f seatOffset(PlayerId id) noexcept
{
    const float x = (static_cast<float>(std::to_underlying(id)) - 1.5f) * 12.f;
    return sgl::Vec2f{x, 0.f};
}

[[nodiscard]] float vecLength(sgl::Vec2f v) noexcept
{
    return std::sqrt(sgl::lengthSq(v));
}

} // namespace

PawnAnimator::PawnAnimator(const LayoutView& layout)
    : layout_{&layout}
{
}

void PawnAnimator::reset(PlayerId id, RingSlot at)
{
    auto& s = states_[playerIndex(id)];
    s.pos = layout_->position(at);
    s.points.clear();
    s.segment = 0;
    s.along = 0.f;
    s.animating = false;
}

void PawnAnimator::start(PlayerId id, const std::vector<sgl::Vec2f>& points)
{
    auto& s = states_[playerIndex(id)];
    s.points = points;
    s.segment = 0;
    s.along = 0.f;
    if(points.empty())
    {
        s.animating = false;
        return;
    }
    s.pos = points.front();
    s.animating = points.size() > 1;
}

void PawnAnimator::update(sgl::Seconds dt)
{
    float budget = speed_ * dt.count();
    if(budget <= 0.f)
    {
        return;
    }

    for(auto& s: states_)
    {
        if(!s.animating)
        {
            continue;
        }

        float remaining = budget;
        while(s.animating && remaining > 0.f)
        {
            if(s.segment + 1 >= s.points.size())
            {
                s.pos = s.points.back();
                s.animating = false;
                break;
            }

            const sgl::Vec2f a = s.points[s.segment];
            const sgl::Vec2f b = s.points[s.segment + 1];
            const sgl::Vec2f delta = b - a;
            const float segLen = vecLength(delta);
            if(segLen <= 0.f)
            {
                ++s.segment;
                s.along = 0.f;
                s.pos = b;
                continue;
            }

            const float left = segLen - s.along;
            if(remaining >= left)
            {
                remaining -= left;
                ++s.segment;
                s.along = 0.f;
                s.pos = b;
                if(s.segment + 1 >= s.points.size())
                {
                    s.animating = false;
                }
            }
            else
            {
                s.along += remaining;
                const float t = s.along / segLen;
                s.pos = a + delta * t;
                remaining = 0.f;
            }
        }
    }
}

sgl::Vec2f PawnAnimator::position(PlayerId id) const
{
    return states_[playerIndex(id)].pos + seatOffset(id);
}

bool PawnAnimator::busy() const noexcept
{
    for(const auto& s: states_)
    {
        if(s.animating)
        {
            return true;
        }
    }
    return false;
}

void PawnAnimator::setSpeed(float pixelsPerSecond)
{
    speed_ = pixelsPerSecond;
}

} // namespace sgl::syndicate
