#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PawnAnimator.hpp>
#include <vector>

using sgl::syndicate::LayoutView;
using sgl::syndicate::PawnAnimator;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RingSlot;

namespace
{

[[nodiscard]] float dist(sgl::Vec2f a, sgl::Vec2f b)
{
    return std::sqrt(sgl::lengthSq(a - b));
}

[[nodiscard]] float distToSegment(sgl::Vec2f p, sgl::Vec2f a, sgl::Vec2f b)
{
    const sgl::Vec2f ab = b - a;
    const float lenSq = sgl::lengthSq(ab);
    if(lenSq <= 0.f)
    {
        return dist(p, a);
    }
    float t = ((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / lenSq;
    t = std::clamp(t, 0.f, 1.f);
    return dist(p, a + ab * t);
}

} // namespace

TEST(PawnAnimatorTest, easesAlongCatmullRomBetweenControlPoints)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    PawnAnimator anim{layout};

    const sgl::Vec2f a{0.f, 0.f};
    const sgl::Vec2f b{100.f, 0.f};
    const PlayerId id{0};
    const sgl::Vec2f offset{(0.f - 1.5f) * 12.f, 0.f};

    anim.start(id, std::vector<sgl::Vec2f>{a, b});
    EXPECT_TRUE(anim.busy());

    anim.update(sgl::Seconds{0.f});
    const auto atStart = anim.position(id);
    EXPECT_NEAR(atStart.x, a.x + offset.x, 0.01f);
    EXPECT_NEAR(atStart.y, a.y + offset.y, 0.01f);

    // hops = 1 → T = 0.25s
    constexpr float T = 0.25f;
    const float window = 0.02f;

    anim.start(id, std::vector<sgl::Vec2f>{a, b});
    anim.update(sgl::Seconds{0.1f * T - window * 0.5f});
    const auto pEarly0 = anim.position(id);
    anim.update(sgl::Seconds{window});
    const auto pEarly1 = anim.position(id);
    const float earlyDist = dist(pEarly0, pEarly1);

    anim.start(id, std::vector<sgl::Vec2f>{a, b});
    anim.update(sgl::Seconds{0.5f * T - window * 0.5f});
    const auto pMid0 = anim.position(id);
    anim.update(sgl::Seconds{window});
    const auto pMid1 = anim.position(id);
    const float midDist = dist(pMid0, pMid1);
    EXPECT_LT(earlyDist, midDist);

    anim.start(id, std::vector<sgl::Vec2f>{a, b});
    anim.update(sgl::Seconds{T});
    EXPECT_FALSE(anim.busy());
    const auto atEnd = anim.position(id);
    EXPECT_NEAR(atEnd.x, b.x + offset.x, 0.01f);
    EXPECT_NEAR(atEnd.y, b.y + offset.y, 0.01f);

    anim.start(id, std::vector<sgl::Vec2f>{a, b});
    const float maxDist = 2.f * layout.nodeRadius();
    for(int i = 0; i < 10; ++i)
    {
        anim.update(sgl::Seconds{T / 10.f});
        const sgl::Vec2f pos = anim.position(id) - offset;
        EXPECT_LE(distToSegment(pos, a, b), maxDist);
    }
}

TEST(PawnAnimatorTest, busyClearsOnArrival)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    PawnAnimator anim{layout};
    anim.start(PlayerId{1}, std::vector<sgl::Vec2f>{{100.f, 100.f}, {500.f, 100.f}});
    EXPECT_TRUE(anim.busy());
    anim.update(sgl::Seconds{0.25f});
    EXPECT_FALSE(anim.busy());
}

TEST(PawnAnimatorTest, sameSlotDifferentPositions)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    PawnAnimator anim{layout};
    const RingSlot slot{.ring = 0, .slot = 0};
    anim.reset(PlayerId{0}, slot);
    anim.reset(PlayerId{1}, slot);
    const auto p0 = anim.position(PlayerId{0});
    const auto p1 = anim.position(PlayerId{1});
    EXPECT_NE(p0, p1);
    EXPECT_NEAR(std::abs(p0.x - p1.x), 12.f, 0.01f);
}

TEST(PawnAnimatorTest, finishAllSnapsToEnd)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    PawnAnimator anim{layout};
    const sgl::Vec2f a{0.f, 0.f};
    const sgl::Vec2f b{200.f, 0.f};
    anim.start(PlayerId{0}, std::vector<sgl::Vec2f>{a, b});
    anim.finishAll();
    EXPECT_FALSE(anim.busy());
    const auto pos = anim.position(PlayerId{0});
    EXPECT_NEAR(pos.x, b.x - 18.f, 0.01f);
}
