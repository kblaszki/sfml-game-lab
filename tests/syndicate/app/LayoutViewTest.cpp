#include <cmath>
#include <gtest/gtest.h>
#include <sgl/core/Rect.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <vector>

using sgl::syndicate::LayoutView;
using sgl::syndicate::RingSlot;
using sgl::syndicate::kActionBar;
using sgl::syndicate::kBoardBounds;
using sgl::syndicate::kLeftPanel;
using sgl::syndicate::kRightChart;
using sgl::syndicate::kRightLog;

TEST(LayoutViewTest, outerNeighborsDoNotOverlap)
{
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    const float minDist = layout.nodeRadius() * 2.f;
    for(std::uint8_t a = 0; a < 24; ++a)
    {
        const auto pa = layout.position(RingSlot{.ring = 0, .slot = a});
        for(std::uint8_t b = static_cast<std::uint8_t>(a + 1); b < 24; ++b)
        {
            const auto pb = layout.position(RingSlot{.ring = 0, .slot = b});
            const float d = std::sqrt(sgl::lengthSq(pa - pb));
            EXPECT_GE(d, minDist) << "slots " << static_cast<int>(a) << " and " << static_cast<int>(b);
        }
    }
}

TEST(LayoutViewTest, ring0Slot0IsAboveOrigin)
{
    const sgl::Vec2f origin{640.f, 360.f};
    const LayoutView layout{origin, 320.f};
    const auto p = layout.position(RingSlot{.ring = 0, .slot = 0});
    EXPECT_LT(p.y, origin.y);
    EXPECT_NEAR(p.x, origin.x, 0.01f);
}

TEST(LayoutViewTest, ring2CloserThanRing0)
{
    const sgl::Vec2f origin{640.f, 360.f};
    const LayoutView layout{origin, 320.f};
    const auto outer = layout.position(RingSlot{.ring = 0, .slot = 0});
    const auto inner = layout.position(RingSlot{.ring = 2, .slot = 0});
    const float d0 = std::sqrt(sgl::lengthSq(outer - origin));
    const float d2 = std::sqrt(sgl::lengthSq(inner - origin));
    EXPECT_LT(d2, d0);
}

TEST(LayoutViewTest, playLayoutRectsDoNotOverlap)
{
    const std::vector<sgl::Rect<float>> rects{kLeftPanel, kRightChart, kRightLog, kActionBar, kBoardBounds};
    for(std::size_t i = 0; i < rects.size(); ++i)
    {
        for(std::size_t j = i + 1; j < rects.size(); ++j)
        {
            EXPECT_FALSE(rects[i].intersects(rects[j])) << i << " vs " << j;
        }
    }
    EXPECT_FALSE(kRightChart.intersects(kRightLog));
    EXPECT_FALSE(kBoardBounds.intersects(kLeftPanel));
    EXPECT_FALSE(kBoardBounds.intersects(kRightChart));
    EXPECT_FALSE(kBoardBounds.intersects(kRightLog));
    EXPECT_FALSE(kBoardBounds.intersects(kActionBar));
}
