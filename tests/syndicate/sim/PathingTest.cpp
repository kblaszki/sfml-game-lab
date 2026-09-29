#include <algorithm>
#include <gtest/gtest.h>
#include <optional>
#include <set>
#include <syndicate/sim/BoardLayouts.hpp>
#include <syndicate/sim/Pathing.hpp>

using sgl::syndicate::BoardGraph;
using sgl::syndicate::MoveOption;
using sgl::syndicate::moveOptions;
using sgl::syndicate::NodeId;
using sgl::syndicate::standardBoard;

namespace
{

[[nodiscard]] bool hasImmediateReversal(const std::vector<NodeId>& path)
{
    for(std::size_t i = 2; i < path.size(); ++i)
    {
        if(path[i] == path[i - 2])
        {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(PathingTest, FromPaydaySteps6HasMultipleDestinations)
{
    const BoardGraph board = standardBoard();
    const auto options = moveOptions(board, NodeId{0}, std::nullopt, 6);
    ASSERT_GE(options.size(), 2u);

    std::set<NodeId> destinations;
    for(const auto& opt: options)
    {
        destinations.insert(opt.destination);
        EXPECT_EQ(opt.path.size(), 7u);
        EXPECT_EQ(opt.path.front(), NodeId{0});
        EXPECT_EQ(opt.path.back(), opt.destination);
        EXPECT_FALSE(hasImmediateReversal(opt.path));
    }
    EXPECT_GE(destinations.size(), 2u);
}

TEST(PathingTest, DeterministicOrder)
{
    const BoardGraph board = standardBoard();
    const auto a = moveOptions(board, NodeId{0}, std::nullopt, 6);
    const auto b = moveOptions(board, NodeId{0}, std::nullopt, 6);
    ASSERT_EQ(a.size(), b.size());
    for(std::size_t i = 0; i < a.size(); ++i)
    {
        EXPECT_EQ(a[i].destination, b[i].destination);
        EXPECT_EQ(a[i].passesPayday, b[i].passesPayday);
        EXPECT_EQ(a[i].path, b[i].path);
    }
}

TEST(PathingTest, PassesPaydayFlag)
{
    const BoardGraph board = standardBoard();

    // From outer slot 12, twelve CW steps along the outer ring land on Payday (node 0).
    const auto hitsPayday = moveOptions(board, NodeId{12}, std::nullopt, 12);
    const auto hit = std::find_if(hitsPayday.begin(), hitsPayday.end(), [](const MoveOption& o) {
        return o.destination == NodeId{0} && o.passesPayday;
    });
    ASSERT_NE(hit, hitsPayday.end());
    EXPECT_TRUE(hit->passesPayday);
    EXPECT_NE(std::find(hit->path.begin() + 1, hit->path.end(), NodeId{0}), hit->path.end());

    const auto shortHop = moveOptions(board, NodeId{0}, std::nullopt, 1);
    ASSERT_FALSE(shortHop.empty());
    const auto noPass =
        std::find_if(shortHop.begin(), shortHop.end(), [](const MoveOption& o) { return !o.passesPayday; });
    ASSERT_NE(noPass, shortHop.end());
    EXPECT_FALSE(noPass->passesPayday);
}

TEST(PathingTest, StepsZeroAndNegative)
{
    const BoardGraph board = standardBoard();
    const auto stay = moveOptions(board, NodeId{0}, std::nullopt, 0);
    ASSERT_EQ(stay.size(), 1u);
    EXPECT_EQ(stay[0].destination, NodeId{0});
    EXPECT_EQ(stay[0].path, (std::vector<NodeId>{NodeId{0}}));
    EXPECT_FALSE(stay[0].passesPayday);

    EXPECT_TRUE(moveOptions(board, NodeId{0}, std::nullopt, -1).empty());
}

TEST(PathingTest, NoImmediateReversalFirstStep)
{
    const BoardGraph board = standardBoard();
    // Standing as if we arrived at 1 from 0; first step must not go back to 0.
    const auto options = moveOptions(board, NodeId{1}, NodeId{0}, 1);
    ASSERT_FALSE(options.empty());
    for(const auto& opt: options)
    {
        EXPECT_NE(opt.destination, NodeId{0});
        EXPECT_FALSE(hasImmediateReversal(opt.path));
    }
}
