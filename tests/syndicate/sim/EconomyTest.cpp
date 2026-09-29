#include <array>
#include <gtest/gtest.h>
#include <sgl/core/Random.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Economy.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Money.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <utility>
#include <vector>

using sgl::Pcg32;
using sgl::syndicate::BoardGraph;
using sgl::syndicate::Edge;
using sgl::syndicate::FeeSplit;
using sgl::syndicate::Market;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::Ownership;
using sgl::syndicate::Payday;
using sgl::syndicate::Percent;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RingSlot;
using sgl::syndicate::Sale;
using sgl::syndicate::Sector;
using sgl::syndicate::SectorTile;
using sgl::syndicate::TileNode;
using sgl::syndicate::dividends;
using sgl::syndicate::kMaxPlayers;
using sgl::syndicate::landingFee;
using sgl::syndicate::liquidationPlan;
using sgl::syndicate::netWorth;
using sgl::syndicate::sharePrice;
using sgl::syndicate::upgradeCost;

namespace
{

BoardGraph makeBoard(std::vector<TileNode> nodes, std::vector<Edge> edges)
{
    auto built = BoardGraph::build(std::move(nodes), edges);
    EXPECT_TRUE(built.has_value());
    return std::move(*built);
}

// Payday + one AI sector tile.
BoardGraph oneSectorBoard(Money basePrice = Money{100}, Money baseFee = Money{100})
{
    return makeBoard(
        {
            TileNode{NodeId{0}, Payday{}, RingSlot{0, 0}},
            TileNode{NodeId{1}, SectorTile{Sector::AI, basePrice, baseFee}, RingSlot{0, 1}},
        },
        {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{0}}});
}

// Payday + two AI sector tiles (for synergy).
BoardGraph twoSectorBoard(Money baseFee = Money{100})
{
    return makeBoard(
        {
            TileNode{NodeId{0}, Payday{}, RingSlot{0, 0}},
            TileNode{NodeId{1}, SectorTile{Sector::AI, Money{100}, baseFee}, RingSlot{0, 1}},
            TileNode{NodeId{2}, SectorTile{Sector::AI, Money{100}, baseFee}, RingSlot{0, 2}},
        },
        {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{2}}, {NodeId{2}, NodeId{0}}});
}

void setShares(Ownership& own, NodeId tile, std::array<std::uint8_t, kMaxPlayers> shares, std::uint8_t bank)
{
    auto& h = own.holdings(tile);
    h.shares = shares;
    h.bank = bank;
}

} // namespace

TEST(EconomyTest, LandingFeeHandComputedBaseline)
{
    // level 0, index 100, synergy 100, baseFee 100, payer holds 0, other holds 10 -> F=100, total=100.
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {0, 10, 0, 0}, 0);
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    EXPECT_EQ(fee.total, (Money{100}));
    EXPECT_EQ(fee.toPlayer[0], (Money{0}));
    EXPECT_EQ(fee.toPlayer[1], (Money{100}));
}

TEST(EconomyTest, LandingFeeLevel2Doubles)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {0, 10, 0, 0}, 0);
    own.holdings(NodeId{1}).level = 2; // levelMul = 200
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    // F = 100 * 200% * 100% * 100% = 200; otherShares=10 -> total 200
    EXPECT_EQ(fee.total, (Money{200}));
    EXPECT_EQ(fee.toPlayer[1], (Money{200}));
}

TEST(EconomyTest, LandingFeeIndex150)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {0, 10, 0, 0}, 0);
    Market market{};
    market.index[std::to_underlying(Sector::AI)] = Percent{150};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    // F = 100 * 150/100 = 150
    EXPECT_EQ(fee.total, (Money{150}));
    EXPECT_EQ(fee.toPlayer[1], (Money{150}));
}

TEST(EconomyTest, LandingFeeSynergyTwoControlled)
{
    const auto board = twoSectorBoard();
    Ownership own{board.size()};
    // Player 1 controls both AI tiles (6+ each).
    setShares(own, NodeId{1}, {0, 6, 0, 0}, 4);
    setShares(own, NodeId{2}, {0, 6, 0, 0}, 4);
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    // controlled=2 -> synergy 120; F = 100 * 120/100 = 120; otherShares=6 -> total = 120*6/10 = 72
    EXPECT_EQ(fee.total, (Money{72}));
    EXPECT_EQ(fee.toPlayer[1], (Money{72}));
}

TEST(EconomyTest, LandingFeePayerPartialShares)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {4, 6, 0, 0}, 0);
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    // F=100, otherShares=6, total=60
    EXPECT_EQ(fee.total, (Money{60}));
    EXPECT_EQ(fee.toPlayer[0], (Money{0}));
    EXPECT_EQ(fee.toPlayer[1], (Money{60}));
}

TEST(EconomyTest, LandingFeeRemainderToLargerHolder)
{
    // F=100, otherShares=5+3=8, total=80.
    // floors: p1 -> 5*80/8=50, p2 -> 3*80/8=30; rem=0... need rem > 0.
    // Use baseFee 101: F=101, other 5+3=8, total=101*8/10=80.
    // Still even. baseFee 103: F=103, total=103*8/10=82.
    // p1: 5*82/8=51, p2: 3*82/8=30, sum=81, rem=1 -> larger holder p1.
    const auto board = oneSectorBoard(Money{100}, Money{103});
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {2, 5, 3, 0}, 0);
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    EXPECT_EQ(fee.total, (Money{82}));
    EXPECT_EQ(fee.toPlayer[1] + fee.toPlayer[2], fee.total);
    EXPECT_EQ(fee.toPlayer[1], (Money{52})); // 51 + rem 1
    EXPECT_EQ(fee.toPlayer[2], (Money{30}));
}

TEST(EconomyTest, LandingFeeRemainderTieGoesToLowerPlayerId)
{
    // Equal shares 4 and 4 among others; payer holds 2. otherShares=8.
    // baseFee 103: F=103, total=82. floors: 4*82/8=41 each, rem=0.
    // baseFee 101: F=101, total=80. floors 40 each, rem=0.
    // baseFee 107: F=107, total=107*8/10=85. floors: 4*85/8=42 each, sum=84, rem=1 -> lower id.
    const auto board = oneSectorBoard(Money{100}, Money{107});
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {2, 4, 4, 0}, 0);
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    EXPECT_EQ(fee.total, (Money{85}));
    EXPECT_EQ(fee.toPlayer[1], (Money{43})); // 42 + rem
    EXPECT_EQ(fee.toPlayer[2], (Money{42}));
}

TEST(EconomyTest, LandingFeeConservationRandom)
{
    const auto board = oneSectorBoard(Money{100}, Money{100});
    Market market{};
    Pcg32 rng{1};
    for(int trial = 0; trial < 1000; ++trial)
    {
        Ownership own{board.size()};
        std::array<std::uint8_t, kMaxPlayers> shares{};
        std::uint8_t remaining = 10;
        for(std::size_t p = 0; p < kMaxPlayers; ++p)
        {
            const auto take = static_cast<std::uint8_t>(rng.uniformBelow(remaining + 1u));
            shares[p] = take;
            remaining = static_cast<std::uint8_t>(remaining - take);
        }
        setShares(own, NodeId{1}, shares, remaining);
        own.holdings(NodeId{1}).level = static_cast<std::uint8_t>(rng.uniformBelow(4));

        const auto payer = PlayerId{static_cast<std::uint8_t>(rng.uniformBelow(static_cast<std::uint32_t>(kMaxPlayers)))};
        const auto fee = landingFee(board, own, market, NodeId{1}, payer);

        Money sum{};
        for(const auto& m : fee.toPlayer)
        {
            sum = sum + m;
        }
        EXPECT_EQ(sum, fee.total);

        int otherShares = 0;
        for(std::size_t p = 0; p < kMaxPlayers; ++p)
        {
            if(PlayerId{static_cast<std::uint8_t>(p)} != payer)
            {
                otherShares += shares[p];
            }
        }
        // One sector tile => synergy always 100 (controlled∈{0,1}).
        const Percent levelMul{100 + 50 * static_cast<int>(own.holdings(NodeId{1}).level)};
        const Money f = scale(scale(scale(Money{100}, levelMul), Percent{100}), Percent{100});
        EXPECT_EQ(fee.total, (Money{f.v * otherShares / 10}));
        EXPECT_EQ(fee.toPlayer[std::to_underlying(payer)], (Money{0}));
    }
}

TEST(EconomyTest, LandingFeeOwnSharesPayZero)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {10, 0, 0, 0}, 0);
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{1}, PlayerId{0});
    EXPECT_EQ(fee.total, (Money{0}));
}

TEST(EconomyTest, LandingFeeNonSectorIsZero)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    Market market{};
    const auto fee = landingFee(board, own, market, NodeId{0}, PlayerId{0});
    EXPECT_EQ(fee.total, (Money{0}));
}

TEST(EconomyTest, DividendsPerShare)
{
    const auto board = oneSectorBoard(Money{100}, Money{100});
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {3, 0, 0, 0}, 7);
    Market market{};
    market.index[0] = Percent{150};
    // per share = scale(100, 150).v / 10 = 150/10 = 15; *3 = 45
    EXPECT_EQ(dividends(board, own, market, PlayerId{0}), (Money{45}));
}

TEST(EconomyTest, UpgradeCost)
{
    const auto board = oneSectorBoard(Money{100}, Money{10});
    // scale(100, 50%) * (level+1) = 50 * (level+1)
    EXPECT_EQ(upgradeCost(board, NodeId{1}, 0), (Money{50}));
    EXPECT_EQ(upgradeCost(board, NodeId{1}, 1), (Money{100}));
    EXPECT_EQ(upgradeCost(board, NodeId{1}, 2), (Money{150}));
    EXPECT_EQ(upgradeCost(board, NodeId{1}, 3), (Money{0})); // not upgradable
    EXPECT_EQ(upgradeCost(board, NodeId{0}, 0), (Money{0})); // non-sector
}

TEST(EconomyTest, NetWorthNegativeCashNoAssets)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    Market market{};
    EXPECT_EQ(netWorth(board, own, market, PlayerId{0}, Money{-40}), (Money{-40}));
}

TEST(EconomyTest, NetWorthIncludesSharesAndUpgradeHalf)
{
    const auto board = oneSectorBoard(Money{100}, Money{100});
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {6, 0, 0, 0}, 4);
    own.holdings(NodeId{1}).level = 2;
    Market market{};
    // sharePrice = 10; *6 = 60
    // cumulative upgrade: cost(0)+cost(1) = 50+100 = 150; 50% = 75
    // cash 10 -> worth 10+60+75 = 145
    EXPECT_EQ(netWorth(board, own, market, PlayerId{0}, Money{10}), (Money{145}));
}

TEST(EconomyTest, LiquidationEmptyWhenNoDebt)
{
    const auto board = oneSectorBoard();
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {5, 0, 0, 0}, 5);
    Market market{};
    EXPECT_TRUE(liquidationPlan(board, own, market, PlayerId{0}, Money{0}).empty());
    EXPECT_TRUE(liquidationPlan(board, own, market, PlayerId{0}, Money{-1}).empty());
}

TEST(EconomyTest, LiquidationCheapestFirst)
{
    // Two sector tiles with different prices via different basePrice / same index.
    const auto board = makeBoard(
        {
            TileNode{NodeId{0}, Payday{}, RingSlot{0, 0}},
            TileNode{NodeId{1}, SectorTile{Sector::AI, Money{100}, Money{10}}, RingSlot{0, 1}},
            TileNode{NodeId{2}, SectorTile{Sector::CleanEnergy, Money{200}, Money{10}}, RingSlot{0, 2}},
        },
        {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{2}}, {NodeId{2}, NodeId{0}}});
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {4, 0, 0, 0}, 6); // price 10; 80% = 8
    setShares(own, NodeId{2}, {4, 0, 0, 0}, 6); // price 20; 80% = 16
    Market market{};

    // debt 8 -> sell 1 of cheapest (node 1)
    auto plan = liquidationPlan(board, own, market, PlayerId{0}, Money{8});
    ASSERT_EQ(plan.size(), 1u);
    EXPECT_EQ(plan[0].tile, NodeId{1});
    EXPECT_EQ(plan[0].count, 1);

    // debt 20 -> sell 3 of node1 (24) covers; not touch node2
    plan = liquidationPlan(board, own, market, PlayerId{0}, Money{20});
    ASSERT_EQ(plan.size(), 1u);
    EXPECT_EQ(plan[0].tile, NodeId{1});
    EXPECT_EQ(plan[0].count, 3);

    // debt 40 -> all 4 of node1 (32) then 1 of node2 (16) -> 48
    plan = liquidationPlan(board, own, market, PlayerId{0}, Money{40});
    ASSERT_EQ(plan.size(), 2u);
    EXPECT_EQ(plan[0].tile, NodeId{1});
    EXPECT_EQ(plan[0].count, 4);
    EXPECT_EQ(plan[1].tile, NodeId{2});
    EXPECT_EQ(plan[1].count, 1);
}

TEST(EconomyTest, LiquidationTieBreaksByNodeId)
{
    const auto board = makeBoard(
        {
            TileNode{NodeId{0}, Payday{}, RingSlot{0, 0}},
            TileNode{NodeId{1}, SectorTile{Sector::AI, Money{100}, Money{10}}, RingSlot{0, 1}},
            TileNode{NodeId{2}, SectorTile{Sector::CleanEnergy, Money{100}, Money{10}}, RingSlot{0, 2}},
        },
        {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{2}}, {NodeId{2}, NodeId{0}}});
    Ownership own{board.size()};
    setShares(own, NodeId{1}, {2, 0, 0, 0}, 8);
    setShares(own, NodeId{2}, {2, 0, 0, 0}, 8);
    Market market{};
    // Same price; sell node 1 first.
    const auto plan = liquidationPlan(board, own, market, PlayerId{0}, Money{8});
    ASSERT_EQ(plan.size(), 1u);
    EXPECT_EQ(plan[0].tile, NodeId{1});
    EXPECT_EQ(plan[0].count, 1);
}
