#include <gtest/gtest.h>
#include <syndicate/sim/BoardLayouts.hpp>
#include <variant>

using sgl::syndicate::BoardGraph;
using sgl::syndicate::NodeId;
using sgl::syndicate::Payday;
using sgl::syndicate::standardBoard;

TEST(BoardLayoutsTest, StandardBoardShapeAndPayday)
{
    const BoardGraph board = standardBoard();
    EXPECT_EQ(board.size(), 40u);
    ASSERT_TRUE(std::holds_alternative<Payday>(board.node(NodeId{0}).kind));

    std::size_t paydayCount = 0;
    std::size_t junctionCount = 0;
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const auto id = NodeId{static_cast<std::uint16_t>(i)};
        if(std::holds_alternative<Payday>(board.node(id).kind))
        {
            ++paydayCount;
        }
        if(board.successors(id).size() > 1)
        {
            ++junctionCount;
        }
    }
    EXPECT_EQ(paydayCount, 1u);
    // Bridge endpoints: outer 0/6/12/18, inner 0/3/6/9 (24/27/30/33), core 0/2 (36/38).
    EXPECT_EQ(junctionCount, 10u);

    for(const auto id: {NodeId{0}, NodeId{6}, NodeId{12}, NodeId{18}, NodeId{24}, NodeId{30}})
    {
        EXPECT_GT(board.successors(id).size(), 1u);
    }
    EXPECT_EQ(board.successors(NodeId{1}).size(), 1u);
}
