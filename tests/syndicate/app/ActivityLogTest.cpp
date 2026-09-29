#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <syndicate/app/ActivityLog.hpp>
#include <syndicate/sim/Ids.hpp>

using sgl::syndicate::ActivityLog;
using sgl::syndicate::PlayerId;

TEST(ActivityLogTest, capacityDropsOldest)
{
    ActivityLog log;
    for(int i = 0; i < 70; ++i)
    {
        log.push(1, PlayerId{0}, "e" + std::to_string(i));
    }
    EXPECT_EQ(log.visible(100).size(), 64u);
}

TEST(ActivityLogTest, visibleReturnsNewestInChronologicalOrder)
{
    ActivityLog log;
    for(int i = 0; i < 70; ++i)
    {
        log.push(1, std::nullopt, "e" + std::to_string(i));
    }
    const auto span = log.visible(10);
    ASSERT_EQ(span.size(), 10u);
    for(std::size_t i = 0; i < span.size(); ++i)
    {
        EXPECT_EQ(span[i].text, "e" + std::to_string(60 + static_cast<int>(i))) << i;
    }
    EXPECT_EQ(span.back().text, "e69");
}
