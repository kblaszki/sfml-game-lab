#include <gtest/gtest.h>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>
#include <utility>
#include <vector>

using sgl::syndicate::BoardGraph;
using sgl::syndicate::Edge;
using sgl::syndicate::Market;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::Payday;
using sgl::syndicate::Percent;
using sgl::syndicate::RingSlot;
using sgl::syndicate::Sector;
using sgl::syndicate::SectorTile;
using sgl::syndicate::TileNode;
using sgl::syndicate::applyShift;
using sgl::syndicate::clampIndex;
using sgl::syndicate::revertTowardCenter;
using sgl::syndicate::sharePrice;

namespace
{

BoardGraph twoNodeBoard(Money basePrice, Money baseFee)
{
    std::vector<TileNode> nodes{
        TileNode{NodeId{0}, Payday{}, RingSlot{0, 0}},
        TileNode{NodeId{1}, SectorTile{Sector::AI, basePrice, baseFee}, RingSlot{0, 1}},
    };
    const Edge edges[] = {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{0}}};
    auto built = BoardGraph::build(std::move(nodes), edges);
    EXPECT_TRUE(built.has_value());
    return std::move(*built);
}

} // namespace

TEST(MarketTest, ClampIndex)
{
    Market m;
    m.index[0] = Percent{40};
    m.index[1] = Percent{250};
    m.index[2] = Percent{100};
    clampIndex(m);
    EXPECT_EQ(m.index[0].v, 50);
    EXPECT_EQ(m.index[1].v, 200);
    EXPECT_EQ(m.index[2].v, 100);
}

TEST(MarketTest, RevertTowardCenter)
{
    Market m;
    m.index[0] = Percent{150}; // delta = (100-150)/10 = -5 -> 145
    m.index[1] = Percent{55};  // delta = (100-55)/10 = 4 -> 59
    m.index[2] = Percent{100}; // delta = 0
    m.index[3] = Percent{49};  // would go to 54 then... wait start 49: (100-49)/10=5 -> 54, then clamp not needed
    // Actually start below 50: revert first then clamp. 49 + 5 = 54.
    revertTowardCenter(m);
    EXPECT_EQ(m.index[0].v, 145);
    EXPECT_EQ(m.index[1].v, 59);
    EXPECT_EQ(m.index[2].v, 100);
    EXPECT_EQ(m.index[3].v, 54);

    m.index[0] = Percent{40};
    revertTowardCenter(m);
    // delta = (100-40)/10 = 6 -> 46, clamp -> 50
    EXPECT_EQ(m.index[0].v, 50);
}

TEST(MarketTest, ApplyShiftThenClamp)
{
    Market m;
    applyShift(m, Sector::AI, 30);
    EXPECT_EQ(m.index[std::to_underlying(Sector::AI)].v, 130);
    applyShift(m, Sector::AI, 100);
    EXPECT_EQ(m.index[std::to_underlying(Sector::AI)].v, 200);
    applyShift(m, Sector::Fintech, -100);
    EXPECT_EQ(m.index[std::to_underlying(Sector::Fintech)].v, 50);
}

TEST(MarketTest, SharePriceSectorAndNonSector)
{
    const auto board = twoNodeBoard(Money{100}, Money{100});
    Market m;
    EXPECT_EQ(sharePrice(board, m, NodeId{1}), (Money{10})); // 100/10 * 100%
    m.index[std::to_underlying(Sector::AI)] = Percent{150};
    EXPECT_EQ(sharePrice(board, m, NodeId{1}), (Money{15})); // 10 * 150/100
    EXPECT_EQ(sharePrice(board, m, NodeId{0}), (Money{0}));
}

TEST(MarketTest, DefaultIndexIs100)
{
    Market m{};
    for(const auto& p : m.index)
    {
        EXPECT_EQ(p.v, 100);
    }
}
