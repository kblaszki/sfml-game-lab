#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PawnAnimator.hpp>
#include <syndicate/app/Presentation.hpp>
#include <vector>

using sgl::syndicate::LayoutView;
using sgl::syndicate::PawnAnimator;
using sgl::syndicate::PlayerId;
using sgl::syndicate::Presentation;

TEST(PresentationTest, pathWaitsForDiceThenStarts)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    PawnAnimator animator{layout};
    Presentation presentation{animator, 7};

    presentation.onRolled(7);
    EXPECT_TRUE(presentation.busy());

    const std::vector<sgl::Vec2f> points{{0.f, 0.f}, {100.f, 0.f}};
    presentation.onPath(PlayerId{0}, points);
    EXPECT_FALSE(animator.busy());
    EXPECT_TRUE(presentation.busy());

    // Advance past the dice tumble+pop (~1.15s)
    presentation.update(sgl::Seconds{1.2f});
    EXPECT_TRUE(animator.busy());
    EXPECT_TRUE(presentation.busy());
}

TEST(PresentationTest, fastForwardEndsDiceAndPath)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    PawnAnimator animator{layout};
    Presentation presentation{animator, 3};

    presentation.onRolled(7);
    presentation.onPath(PlayerId{0}, std::vector<sgl::Vec2f>{{10.f, 10.f}, {110.f, 10.f}});
    EXPECT_TRUE(presentation.busy());
    EXPECT_FALSE(animator.busy());

    presentation.fastForward();
    EXPECT_FALSE(presentation.busy());
    EXPECT_FALSE(animator.busy());
}
