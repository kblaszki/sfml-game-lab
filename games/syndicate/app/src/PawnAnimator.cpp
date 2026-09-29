#include <algorithm>
#include <cstddef>
#include <sgl/anim/Easing.hpp>
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

} // namespace

PawnAnimator::PawnAnimator(const LayoutView& layout)
    : layout_{&layout}
{
}

void PawnAnimator::snapToEnd(State& s)
{
    if(!s.control.empty())
    {
        s.pos = s.control.back();
    }
    s.elapsed = s.duration;
    s.animating = false;
}

void PawnAnimator::reset(PlayerId id, RingSlot at)
{
    auto& s = states_[playerIndex(id)];
    s.pos = layout_->position(at);
    s.control.clear();
    s.table = {};
    s.elapsed = 0.f;
    s.duration = 0.f;
    s.animating = false;
}

void PawnAnimator::start(PlayerId id, const std::vector<sgl::Vec2f>& points)
{
    auto& s = states_[playerIndex(id)];
    s.control = points;
    s.elapsed = 0.f;
    s.table = {};

    if(points.size() < 2)
    {
        if(!points.empty())
        {
            s.pos = points.back();
        }
        s.duration = 0.f;
        s.animating = false;
        return;
    }

    const auto samples = sgl::catmullRom(points, 8);
    s.table.build(samples);
    const std::size_t hops = points.size() - 1u;
    const float hopCount = static_cast<float>(hops < 1u ? 1u : hops);
    s.duration = std::min(0.25f * hopCount, 1.6f);
    s.pos = points.front();
    s.animating = true;
}

void PawnAnimator::update(sgl::Seconds dt)
{
    for(auto& s: states_)
    {
        if(!s.animating)
        {
            continue;
        }

        s.elapsed += dt.count();
        if(s.elapsed >= s.duration || s.duration <= 0.f)
        {
            snapToEnd(s);
            continue;
        }

        const float u = sgl::easeInOutCubic(s.elapsed / s.duration);
        s.pos = s.table.at(s.table.length() * u);
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

void PawnAnimator::finish(PlayerId id)
{
    snapToEnd(states_[playerIndex(id)]);
}

void PawnAnimator::finishAll()
{
    for(auto& s: states_)
    {
        if(s.animating || !s.control.empty())
        {
            snapToEnd(s);
        }
    }
}

} // namespace sgl::syndicate
