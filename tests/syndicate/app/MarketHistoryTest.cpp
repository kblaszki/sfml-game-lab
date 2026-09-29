#include <gtest/gtest.h>
#include <syndicate/app/MarketHistory.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>

using sgl::syndicate::Market;
using sgl::syndicate::MarketHistory;
using sgl::syndicate::Percent;
using sgl::syndicate::Sector;

TEST(MarketHistoryTest, dropsOldestAfterCapacity)
{
    MarketHistory history;
    EXPECT_TRUE(history.series(Sector::AI).empty());

    for(int i = 0; i < 33; ++i)
    {
        Market market{};
        for(std::size_t s = 0; s < market.index.size(); ++s)
        {
            market.index[s] = Percent{100 + i + static_cast<int>(s)};
        }
        history.record(market);
    }

    const auto ai = history.series(Sector::AI);
    ASSERT_EQ(ai.size(), MarketHistory::kCapacity);
    EXPECT_EQ(ai.front().v, 101); // first kept sample is i=1 (i=0 dropped)
    EXPECT_EQ(ai.back().v, 100 + 32);

    const auto bio = history.series(Sector::Biotech);
    ASSERT_EQ(bio.size(), MarketHistory::kCapacity);
    EXPECT_EQ(bio.front().v, 101 + static_cast<int>(Sector::Biotech));
    EXPECT_EQ(bio.back().v, 100 + 32 + static_cast<int>(Sector::Biotech));
}

TEST(MarketHistoryTest, seriesMatchesRecordedIndex)
{
    MarketHistory history;
    Market a{};
    a.index = {Percent{101}, Percent{102}, Percent{103}, Percent{104}, Percent{105}, Percent{106}};
    Market b{};
    b.index = {Percent{111}, Percent{112}, Percent{113}, Percent{114}, Percent{115}, Percent{116}};
    history.record(a);
    history.record(b);

    EXPECT_EQ(history.series(Sector::CleanEnergy)[0].v, 102);
    EXPECT_EQ(history.series(Sector::CleanEnergy)[1].v, 112);
    EXPECT_EQ(history.series(Sector::Fintech).size(), 2u);
    EXPECT_EQ(history.series(Sector::Fintech).back().v, 116);
}
