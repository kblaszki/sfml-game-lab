#include <gtest/gtest.h>
#include <optional>
#include <sgl/core/Time.hpp>
#include <sgl/ui/HoverAnim.hpp>

TEST(HoverAnimShould, reachHoverScaleThenReturnToIdle)
{
    sgl::HoverAnim anim;

    anim.update(0, 1, sgl::Seconds{0.12f});
    EXPECT_NEAR(anim.scale(0), 1.06f, 0.01f);

    anim.update(std::nullopt, 1, sgl::Seconds{0.12f});
    EXPECT_NEAR(anim.scale(0), 1.0f, 0.01f);
}

TEST(HoverAnimShould, returnIdleForIndexPastCount)
{
    sgl::HoverAnim anim;
    anim.update(0, 1, sgl::Seconds{0.12f});
    EXPECT_FLOAT_EQ(anim.scale(5), 1.f);
}
