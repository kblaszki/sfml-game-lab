#include <cstdint>
#include <gtest/gtest.h>
#include <set>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/BoardRender.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/sim/BoardLayouts.hpp>
#include <syndicate/sim/Ownership.hpp>
#include <utility>
#include <variant>
#include <vector>

using sgl::syndicate::LayoutView;
using sgl::syndicate::NodeId;
using sgl::syndicate::Ownership;
using sgl::syndicate::defaultTheme;
using sgl::syndicate::renderBoard;
using sgl::syndicate::standardBoard;

namespace
{

[[nodiscard]] std::size_t countCircles(sgl::RenderQueue& queue)
{
    std::size_t n = 0;
    for(const sgl::Entry& e: queue.sorted())
    {
        if(std::holds_alternative<sgl::CircleCmd>(e.command))
        {
            ++n;
        }
    }
    return n;
}

[[nodiscard]] std::size_t countLines(sgl::RenderQueue& queue)
{
    std::size_t n = 0;
    for(const sgl::Entry& e: queue.sorted())
    {
        if(std::holds_alternative<sgl::LineCmd>(e.command))
        {
            ++n;
        }
    }
    return n;
}

[[nodiscard]] std::size_t uniqueUnorderedEdgeCount(const sgl::syndicate::BoardGraph& board)
{
    std::set<std::pair<std::uint16_t, std::uint16_t>> pairs;
    std::size_t directed = 0;
    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId from{static_cast<std::uint16_t>(i)};
        for(const NodeId to: board.successors(from))
        {
            ++directed;
            const auto a = static_cast<std::uint16_t>(from);
            const auto b = static_cast<std::uint16_t>(to);
            pairs.emplace(std::min(a, b), std::max(a, b));
        }
    }
    EXPECT_GT(directed, 0u);
    EXPECT_LE(pairs.size(), directed);
    return pairs.size();
}

} // namespace

TEST(BoardRenderTest, circlesAndUniqueEdges)
{
    const auto board = standardBoard();
    ASSERT_EQ(board.size(), 40u);

    Ownership ownership{board.size()};
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 320.f};
    const auto theme = defaultTheme();

    sgl::RenderQueue queue;
    renderBoard(queue, board, ownership, layout, theme, {});

    EXPECT_EQ(countCircles(queue), 40u);

    const auto expectedLines = uniqueUnorderedEdgeCount(board);
    EXPECT_EQ(countLines(queue), expectedLines);
    EXPECT_GT(countLines(queue), 0u);

    for(std::size_t i = 0; i < board.size(); ++i)
    {
        const NodeId id{static_cast<std::uint16_t>(i)};
        const auto expected = layout.position(board.node(id).at);
        bool found = false;
        for(const sgl::Entry& e: queue.sorted())
        {
            if(const auto* c = std::get_if<sgl::CircleCmd>(&e.command))
            {
                if(c->center == expected)
                {
                    found = true;
                    break;
                }
            }
        }
        EXPECT_TRUE(found) << "missing circle for node " << i;
    }
}
