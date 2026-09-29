#include <gtest/gtest.h>
#include <syndicate/sim/BoardGraph.hpp>
#include <vector>

using sgl::syndicate::BoardError;
using sgl::syndicate::BoardGraph;
using sgl::syndicate::Edge;
using sgl::syndicate::Exchange;
using sgl::syndicate::NodeId;
using sgl::syndicate::Payday;
using sgl::syndicate::RingSlot;
using sgl::syndicate::TileNode;

namespace
{

TileNode makePayday(std::uint16_t id)
{
    return TileNode{NodeId{id}, Payday{}, RingSlot{0, static_cast<std::uint8_t>(id)}};
}

TileNode makeExchange(std::uint16_t id)
{
    return TileNode{NodeId{id}, Exchange{}, RingSlot{0, static_cast<std::uint8_t>(id)}};
}

} // namespace

TEST(BoardGraphTest, DuplicateId)
{
    std::vector<TileNode> nodes{makePayday(0), makePayday(0)};
    const Edge edges[] = {{NodeId{0}, NodeId{0}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BoardError::DuplicateId);
}

TEST(BoardGraphTest, NonDenseIds)
{
    std::vector<TileNode> nodes{makePayday(0), makeExchange(2)};
    const Edge edges[] = {{NodeId{0}, NodeId{2}}, {NodeId{2}, NodeId{0}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BoardError::NonDenseIds);
}

TEST(BoardGraphTest, DanglingEdge)
{
    std::vector<TileNode> nodes{makePayday(0), makeExchange(1)};
    const Edge edges[] = {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{9}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BoardError::DanglingEdge);
}

TEST(BoardGraphTest, DeadEnd)
{
    std::vector<TileNode> nodes{makePayday(0), makeExchange(1)};
    const Edge edges[] = {{NodeId{0}, NodeId{1}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BoardError::DeadEnd);
}

TEST(BoardGraphTest, NotStronglyConnected)
{
    // Two nodes with a one-way edge cycle broken: 0->1 and 1->1 (1 cannot reach 0).
    std::vector<TileNode> nodes{makePayday(0), makeExchange(1)};
    const Edge edges[] = {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{1}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BoardError::NotStronglyConnected);
}

TEST(BoardGraphTest, PaydayCount)
{
    std::vector<TileNode> nodes{makeExchange(0), makeExchange(1)};
    const Edge edges[] = {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{0}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), BoardError::PaydayCount);
}

TEST(BoardGraphTest, BuildSucceedsForTinyCycle)
{
    std::vector<TileNode> nodes{makePayday(0), makeExchange(1)};
    const Edge edges[] = {{NodeId{0}, NodeId{1}}, {NodeId{1}, NodeId{0}}};
    const auto result = BoardGraph::build(std::move(nodes), edges);
    ASSERT_TRUE(result);
    EXPECT_EQ(result->size(), 2u);
    EXPECT_EQ(result->successors(NodeId{0}).size(), 1u);
}
