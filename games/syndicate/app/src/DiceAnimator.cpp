#include <syndicate/app/DiceAnimator.hpp>

#include <algorithm>
#include <cmath>
#include <sgl/anim/Easing.hpp>
#include <sgl/core/Color.hpp>
#include <sgl/core/Rect.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/Shapes.hpp>
#include <vector>

namespace sgl::syndicate
{
namespace
{

constexpr float kBaseInterval = 0.04f;
constexpr float kIntervalGrowth = 1.35f;
constexpr float kTumbleEnd = 0.9f;
constexpr float kPopDur = 0.25f;
constexpr float kPopScaleFrom = 1.15f;
constexpr float kDieSize = 48.f;
constexpr float kDieGap = 16.f;
constexpr float kCorner = 8.f;
constexpr float kPipR = 4.f;

[[nodiscard]] int clampFace(int v) noexcept
{
    return std::clamp(v, 1, 4);
}

void pushPips(sgl::RenderQueue& queue, sgl::Vec2f dieCenter, float half, int face, float depthBase)
{
    const sgl::Color pip{30, 32, 40};
    const float inset = half * 0.45f;
    std::vector<sgl::Vec2f> spots;
    spots.reserve(4);
    switch(face)
    {
    case 1:
        spots.push_back(dieCenter);
        break;
    case 2:
        spots.push_back({dieCenter.x - inset, dieCenter.y - inset});
        spots.push_back({dieCenter.x + inset, dieCenter.y + inset});
        break;
    case 3:
        spots.push_back({dieCenter.x - inset, dieCenter.y - inset});
        spots.push_back(dieCenter);
        spots.push_back({dieCenter.x + inset, dieCenter.y + inset});
        break;
    default:
        spots.push_back({dieCenter.x - inset, dieCenter.y - inset});
        spots.push_back({dieCenter.x + inset, dieCenter.y - inset});
        spots.push_back({dieCenter.x - inset, dieCenter.y + inset});
        spots.push_back({dieCenter.x + inset, dieCenter.y + inset});
        break;
    }
    float d = depthBase;
    for(const sgl::Vec2f& p: spots)
    {
        queue.push(sgl::Layer::Hud,
                   d++,
                   sgl::CircleCmd{
                       .center = p,
                       .radius = kPipR,
                       .fill = pip,
                   });
    }
}

} // namespace

DiceAnimator::DiceAnimator(std::uint64_t seed)
    : rng_{seed ^ 0xD1CEULL}
{
}

void DiceAnimator::splitFinal(int finalRoll)
{
    const int lo = std::max(1, finalRoll - 4);
    const int hi = std::min(4, finalRoll - 1);
    if(lo <= hi)
    {
        const int span = hi - lo + 1;
        finalA_ = lo + static_cast<int>(rng_.uniformBelow(static_cast<std::uint32_t>(span)));
        finalB_ = finalRoll - finalA_;
    }
    else
    {
        finalA_ = clampFace(finalRoll / 2);
        finalB_ = clampFace(finalRoll - finalA_);
    }
    finalA_ = clampFace(finalA_);
    finalB_ = clampFace(finalB_);
}

void DiceAnimator::randomizeFaces()
{
    const int prevA = faceA_;
    const int prevB = faceB_;
    do
    {
        faceA_ = 1 + static_cast<int>(rng_.uniformBelow(4u));
        faceB_ = 1 + static_cast<int>(rng_.uniformBelow(4u));
    } while(faceA_ == prevA && faceB_ == prevB);
}

void DiceAnimator::start(int finalRoll)
{
    splitFinal(finalRoll);
    active_ = true;
    popping_ = false;
    elapsed_ = 0.f;
    intervalK_ = 0;
    nextChangeAt_ = kBaseInterval;
    scale_ = 1.f;
    randomizeFaces();
}

void DiceAnimator::update(sgl::Seconds dt)
{
    if(!active_)
    {
        return;
    }

    elapsed_ += dt.count();

    if(!popping_)
    {
        while(elapsed_ >= nextChangeAt_ && nextChangeAt_ < kTumbleEnd)
        {
            randomizeFaces();
            ++intervalK_;
            nextChangeAt_ += kBaseInterval * std::pow(kIntervalGrowth, static_cast<float>(intervalK_));
        }

        if(elapsed_ >= kTumbleEnd)
        {
            faceA_ = finalA_;
            faceB_ = finalB_;
            popping_ = true;
        }
    }

    if(popping_)
    {
        const float popT = (elapsed_ - kTumbleEnd) / kPopDur;
        if(popT >= 1.f)
        {
            scale_ = 1.f;
            active_ = false;
            popping_ = false;
        }
        else
        {
            const float e = sgl::easeOutBack(popT);
            scale_ = kPopScaleFrom + (1.f - kPopScaleFrom) * e;
        }
    }
}

bool DiceAnimator::busy() const noexcept
{
    return active_;
}

void DiceAnimator::fastForward()
{
    if(!active_)
    {
        return;
    }
    faceA_ = finalA_;
    faceB_ = finalB_;
    scale_ = 1.f;
    active_ = false;
    popping_ = false;
    elapsed_ = kTumbleEnd + kPopDur;
}

void DiceAnimator::render(sgl::RenderQueue& queue, sgl::Vec2f center, const Theme& theme) const
{
    if(!active_)
    {
        return;
    }

    const float size = kDieSize * scale_;
    const float half = size * 0.5f;
    const float offset = (kDieSize * 0.5f + kDieGap * 0.5f) * scale_;
    const sgl::Color fill = theme.highlight;
    const sgl::Vec2f left{center.x - offset, center.y};
    const sgl::Vec2f right{center.x + offset, center.y};

    auto drawDie = [&](sgl::Vec2f c, int face, float depth) {
        const sgl::Rect<float> rect{.pos = {c.x - half, c.y - half}, .size = {size, size}};
        queue.push(sgl::Layer::Hud, depth, sgl::roundedRect(rect, kCorner * scale_, fill));
        pushPips(queue, c, half, face, depth + 1.f);
    };

    drawDie(left, faceA_, 80.f);
    drawDie(right, faceB_, 90.f);
}

int DiceAnimator::faceA() const noexcept
{
    return faceA_;
}

int DiceAnimator::faceB() const noexcept
{
    return faceB_;
}

} // namespace sgl::syndicate
