#include <gtest/gtest.h>
#include <syndicate/sim/Money.hpp>

using sgl::syndicate::Money;
using sgl::syndicate::Percent;
using sgl::syndicate::scale;

TEST(MoneyTest, ScaleTruncatesTowardZero)
{
    EXPECT_EQ(scale(Money{10}, Percent{15}), (Money{1}));
    EXPECT_EQ(scale(Money{-10}, Percent{15}), (Money{-1}));
    EXPECT_EQ(scale(Money{10}, Percent{9}), (Money{0}));
    EXPECT_EQ(scale(Money{-10}, Percent{9}), (Money{0}));
}

TEST(MoneyTest, Arithmetic)
{
    EXPECT_EQ(Money{3} + Money{4}, (Money{7}));
    EXPECT_EQ(Money{3} - Money{4}, (Money{-1}));
    EXPECT_EQ(Money{3} * 2, (Money{6}));
    EXPECT_EQ(2 * Money{3}, (Money{6}));
}
