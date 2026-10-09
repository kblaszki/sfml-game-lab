#include <gtest/gtest.h>
#include <optional>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/AuctionPanel.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>

using sgl::syndicate::AuctionPanel;
using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitRoll;
using sgl::syndicate::BuyShares;
using sgl::syndicate::ChooseDestination;
using sgl::syndicate::DeclinePurchase;
using sgl::syndicate::InAuction;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RollDice;
using sgl::syndicate::SectorTile;
using sgl::syndicate::defaultTheme;

namespace
{

[[nodiscard]] Match makeMatch()
{
    auto m = Match::create(
        MatchConfig{.seed = 7, .seats = 2, .startCash = Money{1500}, .maxRounds = 15, .targetNetWorth = Money{6000}});
    EXPECT_TRUE(m.has_value());
    return std::move(*m);
}

[[nodiscard]] bool isSector(const Match& m, sgl::syndicate::NodeId id)
{
    return std::holds_alternative<SectorTile>(m.state().board.node(id).kind);
}

[[nodiscard]] std::optional<ChooseDestination> findSectorDestination(const Match& m)
{
    const auto* path = std::get_if<AwaitPath>(&m.state().phase);
    if(!path)
    {
        return std::nullopt;
    }
    for(const auto& opt: path->options)
    {
        if(isSector(m, opt.destination))
        {
            return ChooseDestination{opt.destination, opt.passesPayday};
        }
    }
    return std::nullopt;
}

[[nodiscard]] bool driveToAuction(Match& m)
{
    for(int attempt = 0; attempt < 80; ++attempt)
    {
        if(std::holds_alternative<InAuction>(m.state().phase))
        {
            return true;
        }
        const PlayerId actor = m.actingPlayer();
        if(std::holds_alternative<AwaitRoll>(m.state().phase))
        {
            if(!m.apply(actor, RollDice{}))
            {
                return false;
            }
            continue;
        }
        if(std::holds_alternative<AwaitPath>(m.state().phase))
        {
            if(const auto dest = findSectorDestination(m))
            {
                if(!m.apply(actor, *dest))
                {
                    return false;
                }
                continue;
            }
            const auto* path = std::get_if<AwaitPath>(&m.state().phase);
            if(path == nullptr || path->options.empty())
            {
                return false;
            }
            if(!m.apply(actor, ChooseDestination{path->options.front().destination, path->options.front().passesPayday}))
            {
                return false;
            }
            continue;
        }
        if(std::holds_alternative<AwaitPurchase>(m.state().phase))
        {
            if(!m.apply(actor, DeclinePurchase{}))
            {
                return false;
            }
            continue;
        }
        return false;
    }
    return std::holds_alternative<InAuction>(m.state().phase);
}

} // namespace

TEST(AuctionPanelShould, ReportVisibleOnlyInAuction)
{
    AuctionPanel panel;
    Match match = makeMatch();
    EXPECT_FALSE(panel.visible(match.state().phase));
    ASSERT_TRUE(driveToAuction(match));
    EXPECT_TRUE(panel.visible(match.state().phase));
}

TEST(AuctionPanelShould, EmitOverlayCommandsWhenAuctionActive)
{
    AuctionPanel panel;
    Match match = makeMatch();
    sgl::RenderQueue idle;
    panel.render(idle, match, sgl::FontId{1}, defaultTheme());
    EXPECT_EQ(idle.size(), 0u);

    ASSERT_TRUE(driveToAuction(match));
    sgl::RenderQueue queue;
    panel.render(queue, match, sgl::FontId{1}, defaultTheme());
    EXPECT_GT(queue.size(), 0u);
}
