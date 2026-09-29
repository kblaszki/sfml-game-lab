#include <cmath>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PawnAnimator.hpp>
#include <vector>

using sgl::syndicate::LayoutView;
using sgl::syndicate::PawnAnimator;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RingSlot;

TEST(PawnAnimatorTest, travelsPolylineInOneSecond)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    PawnAnimator anim{layout};
    anim.setSpeed(400.f);

    const sgl::Vec2f a{0.f, 0.f};
    const sgl::Vec2f b{400.f, 0.f};
    anim.start(PlayerId{0}, std::vector<sgl::Vec2f>{a, b});
    EXPECT_TRUE(anim.busy());

    for(int i = 0; i < 20; ++i)
    {
        anim.update(sgl::Seconds{0.05f});
    }

    EXPECT_FALSE(anim.busy());
    const auto pos = anim.position(PlayerId{0});
    // Seat offset for id 0: (0 - 1.5) * 12 = -18
    EXPECT_NEAR(pos.x, b.x - 18.f, 1.f);
    EXPECT_NEAR(pos.y, b.y, 1.f);
}

TEST(PawnAnimatorTest, busyClearsOnArrival)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    PawnAnimator anim{layout};
    anim.setSpeed(400.f);
    anim.start(PlayerId{1}, std::vector<sgl::Vec2f>{{100.f, 100.f}, {500.f, 100.f}});
    EXPECT_TRUE(anim.busy());
    anim.update(sgl::Seconds{1.f});
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
