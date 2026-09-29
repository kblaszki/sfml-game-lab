#include <cmath>
#include <gtest/gtest.h>
#include <syndicate/app/LayoutView.hpp>

using sgl::syndicate::LayoutView;
using sgl::syndicate::RingSlot;

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
