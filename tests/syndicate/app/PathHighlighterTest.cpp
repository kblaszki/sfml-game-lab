#include <gtest/gtest.h>
#include <optional>
#include <set>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PathHighlighter.hpp>
#include <syndicate/sim/BoardLayouts.hpp>
#include <syndicate/sim/Pathing.hpp>

using sgl::syndicate::LayoutView;
using sgl::syndicate::MoveOption;
using sgl::syndicate::NodeId;
using sgl::syndicate::PathHighlighter;
using sgl::syndicate::moveOptions;
using sgl::syndicate::standardBoard;

TEST(PathHighlighterTest, pulseStaysInRange)
{
    PathHighlighter highlighter;
    const auto board = standardBoard();
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    highlighter.build({}, board, layout);

    for(int i = 0; i < 120; ++i)
    {
        highlighter.update(sgl::Seconds{1.f / 60.f}, sgl::Vec2f{0.f, 0.f});
        EXPECT_GE(highlighter.model().pulse, 0.88f);
        EXPECT_LE(highlighter.model().pulse, 1.12f);
    }
}

TEST(PathHighlighterTest, singleOptionYieldsNoArrows)
{
    const auto board = standardBoard();
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    const auto options = moveOptions(board, NodeId{0}, std::nullopt, 0);
    ASSERT_EQ(options.size(), 1u);

    PathHighlighter highlighter;
    highlighter.build(options, board, layout);
    EXPECT_TRUE(highlighter.model().arrows.empty());
    ASSERT_EQ(highlighter.model().destinations.size(), 1u);
    EXPECT_EQ(highlighter.model().destinations.front(), options.front().destination);
}

TEST(PathHighlighterTest, multiOptionMarksEveryDestinationAndHover)
{
    const auto board = standardBoard();
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    const auto options = moveOptions(board, NodeId{0}, std::nullopt, 6);
    ASSERT_GE(options.size(), 2u);

    std::set<NodeId> expected;
    for(const MoveOption& opt: options)
    {
        expected.insert(opt.destination);
    }
    ASSERT_GE(expected.size(), 2u);

    PathHighlighter highlighter;
    highlighter.build(options, board, layout);

    std::set<NodeId> got(highlighter.model().destinations.begin(), highlighter.model().destinations.end());
    EXPECT_EQ(got, expected);
    EXPECT_FALSE(highlighter.model().arrows.empty());

    const NodeId hoverDest = highlighter.model().destinations.front();
    const sgl::Vec2f center = layout.position(board.node(hoverDest).at);
    highlighter.update(sgl::Seconds{0.f}, center);
    ASSERT_TRUE(highlighter.model().hovered.has_value());
    EXPECT_EQ(highlighter.model().destinations[*highlighter.model().hovered], hoverDest);

    highlighter.update(sgl::Seconds{0.f}, sgl::Vec2f{-1000.f, -1000.f});
    EXPECT_FALSE(highlighter.model().hovered.has_value());
}
