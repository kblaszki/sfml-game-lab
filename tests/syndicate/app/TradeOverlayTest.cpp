#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/input/MouseButton.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <string>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/Theme.hpp>
#include <syndicate/app/TradeComposeOverlay.hpp>
#include <syndicate/app/TradeRespondPanel.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>
#include <syndicate/sim/Trade.hpp>
#include <variant>
#include <vector>

using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitTradeResponse;
using sgl::syndicate::bindSyndicate;
using sgl::syndicate::BuyMomentum;
using sgl::syndicate::BuyShares;
using sgl::syndicate::ChooseDestination;
using sgl::syndicate::defaultTheme;
using sgl::syndicate::kBoardOrigin;
using sgl::syndicate::kBoardRadius;
using sgl::syndicate::LayoutView;
using sgl::syndicate::Manage;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::ProposeTrade;
using sgl::syndicate::RollDice;
using sgl::syndicate::SectorTile;
using sgl::syndicate::ShareLot;
using sgl::syndicate::SyndicateActions;
using sgl::syndicate::TradeComposeOverlay;
using sgl::syndicate::TradeOffer;
using sgl::syndicate::TradeRespondPanel;

namespace
{

struct ManageSetup
{
    Match match;
    NodeId tile{};
    bool bought{false};
};

[[nodiscard]] bool isSector(const Match& match, NodeId id)
{
    return std::holds_alternative<SectorTile>(match.state().board.node(id).kind);
}

[[nodiscard]] std::optional<ChooseDestination> findSectorDestination(const Match& match)
{
    const auto* path = std::get_if<AwaitPath>(&match.state().phase);
    if(path == nullptr)
    {
        return std::nullopt;
    }
    for(const auto& opt: path->options)
    {
        if(isSector(match, opt.destination))
        {
            return ChooseDestination{opt.destination, opt.passesPayday};
        }
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<ManageSetup> driveToManage(int seats, bool needShare)
{
    for(std::uint64_t seed = 1; seed <= 80; ++seed)
    {
        auto created = Match::create(MatchConfig{
            .seed = seed,
            .seats = seats,
            .startCash = Money{1500},
            .maxRounds = 15,
            .targetNetWorth = Money{6000},
        });
        if(!created.has_value())
        {
            continue;
        }
        Match match = std::move(*created);
        const PlayerId actor = match.actingPlayer();
        if(!match.apply(actor, RollDice{}))
        {
            continue;
        }
        auto dest = findSectorDestination(match);
        if(!dest.has_value())
        {
            for(const auto& cmd: match.legalCommands(actor))
            {
                if(std::holds_alternative<BuyMomentum>(cmd))
                {
                    if(!match.apply(actor, cmd))
                    {
                        break;
                    }
                    dest = findSectorDestination(match);
                    if(dest.has_value())
                    {
                        break;
                    }
                }
            }
        }
        if(!dest.has_value() || !match.apply(actor, *dest))
        {
            continue;
        }
        if(const auto* purch = std::get_if<AwaitPurchase>(&match.state().phase))
        {
            const NodeId tile = purch->tile;
            if(match.apply(actor, BuyShares{1}) && std::holds_alternative<Manage>(match.state().phase))
            {
                return ManageSetup{.match = std::move(match), .tile = tile, .bought = true};
            }
            continue;
        }
        if(!needShare && std::holds_alternative<Manage>(match.state().phase))
        {
            return ManageSetup{.match = std::move(match), .bought = false};
        }
    }
    return std::nullopt;
}

[[nodiscard]] sgl::InputState keyDown(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

[[nodiscard]] sgl::InputState clickAt(sgl::Vec2f pos, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = pos}, map);
    return input;
}

[[nodiscard]] sgl::Vec2f rectCenter(sgl::Rect<float> rect)
{
    return {rect.pos.x + rect.size.x * 0.5f, rect.pos.y + rect.size.y * 0.5f};
}

[[nodiscard]] PlayerId partnerAfterRight(PlayerId from, int seats)
{
    std::vector<PlayerId> others;
    for(int i = 0; i < seats; ++i)
    {
        const PlayerId id{static_cast<std::uint8_t>(i)};
        if(id != from)
        {
            others.push_back(id);
        }
    }
    if(others.size() < 2)
    {
        return others.empty() ? from : others.front();
    }
    return others[1];
}

} // namespace

TEST(TradeComposeOverlayShould, ApplyCashOfferAfterPartnerAndAmountEdits)
{
    auto setup = driveToManage(4, false);
    ASSERT_TRUE(setup.has_value());
    Match& match = setup->match;
    ASSERT_TRUE(std::holds_alternative<Manage>(match.state().phase));

    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    const LayoutView layout{kBoardOrigin, kBoardRadius};
    const PlayerId from = match.actingPlayer();

    TradeComposeOverlay overlay;
    EXPECT_FALSE(overlay.active());
    overlay.open(match, from);
    EXPECT_TRUE(overlay.active());

    overlay.update(keyDown(sgl::Key::Up, map), match, layout, actions);
    overlay.update(keyDown(sgl::Key::Up, map), match, layout, actions);
    overlay.update(keyDown(sgl::Key::Down, map), match, layout, actions);
    overlay.update(keyDown(sgl::Key::Right, map), match, layout, actions);
    overlay.update(clickAt(rectCenter(overlay.confirmRect()), map), match, layout, actions);

    EXPECT_FALSE(overlay.active());
    const auto* pending = std::get_if<AwaitTradeResponse>(&match.state().phase);
    ASSERT_NE(pending, nullptr);
    EXPECT_EQ(pending->offer.from, from);
    EXPECT_EQ(pending->offer.to, partnerAfterRight(from, match.state().seats));
    EXPECT_EQ(pending->offer.cashFrom, Money{25});
    EXPECT_EQ(pending->offer.cashTo, Money{0});
    EXPECT_TRUE(pending->offer.sharesFrom.empty());
}

TEST(TradeComposeOverlayShould, EmptyConfirmStaysOpenAndCancelCloses)
{
    auto setup = driveToManage(2, false);
    ASSERT_TRUE(setup.has_value());
    Match& match = setup->match;
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    const LayoutView layout{kBoardOrigin, kBoardRadius};

    TradeComposeOverlay overlay;
    overlay.open(match, match.actingPlayer());
    overlay.update(clickAt(rectCenter(overlay.confirmRect()), map), match, layout, actions);
    EXPECT_TRUE(overlay.active());
    EXPECT_TRUE(std::holds_alternative<Manage>(match.state().phase));

    overlay.update(clickAt(rectCenter(overlay.cancelRect()), map), match, layout, actions);
    EXPECT_FALSE(overlay.active());
    EXPECT_TRUE(std::holds_alternative<Manage>(match.state().phase));
}

TEST(TradeComposeOverlayShould, ClickOwnedSectorTogglesShareIntoOffer)
{
    auto setup = driveToManage(2, true);
    ASSERT_TRUE(setup.has_value());
    ASSERT_TRUE(setup->bought);
    Match& match = setup->match;
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    const LayoutView layout{kBoardOrigin, kBoardRadius};

    TradeComposeOverlay overlay;
    overlay.open(match, match.actingPlayer());
    const sgl::Vec2f tilePos = layout.position(match.state().board.node(setup->tile).at);
    overlay.update(clickAt(tilePos, map), match, layout, actions);
    overlay.update(clickAt(rectCenter(overlay.confirmRect()), map), match, layout, actions);

    EXPECT_FALSE(overlay.active());
    const auto* pending = std::get_if<AwaitTradeResponse>(&match.state().phase);
    ASSERT_NE(pending, nullptr);
    ASSERT_EQ(pending->offer.sharesFrom.size(), 1u);
    EXPECT_EQ(pending->offer.sharesFrom.front().tile, setup->tile);
    EXPECT_EQ(pending->offer.sharesFrom.front().count, 1);
    EXPECT_EQ(pending->offer.cashFrom, Money{0});
}

TEST(TradeRespondPanelShould, EmitCommandsOnlyWhileAwaitingResponse)
{
    auto setup = driveToManage(2, false);
    ASSERT_TRUE(setup.has_value());
    Match& match = setup->match;
    const PlayerId from = match.actingPlayer();
    const PlayerId to = partnerAfterRight(from, match.state().seats);

    TradeRespondPanel panel;
    sgl::RenderQueue idle;
    panel.render(idle, match, sgl::FontId{1}, defaultTheme());
    EXPECT_EQ(idle.size(), 0u);

    const TradeOffer offer{
        .from = from,
        .to = to,
        .cashFrom = Money{100},
        .cashTo = Money{0},
        .sharesFrom = {},
        .sharesTo = {},
    };
    ASSERT_TRUE(match.apply(from, ProposeTrade{offer}));
    ASSERT_TRUE(std::holds_alternative<AwaitTradeResponse>(match.state().phase));

    sgl::RenderQueue queue;
    panel.render(queue, match, sgl::FontId{1}, defaultTheme());
    EXPECT_GT(queue.size(), 0u);

    bool sawCash = false;
    bool sawParties = false;
    bool sawLots = false;
    for(const sgl::Entry& entry: queue.sorted())
    {
        const auto* text = std::get_if<sgl::TextCmd>(&entry.command);
        if(text == nullptr)
        {
            continue;
        }
        if(text->text.find("$100k") != std::string::npos)
        {
            sawCash = true;
        }
        if(text->text.find("P") != std::string::npos && text->text.find("offers") != std::string::npos)
        {
            sawParties = true;
        }
        if(text->text.find("none") != std::string::npos)
        {
            sawLots = true;
        }
    }
    EXPECT_TRUE(sawCash);
    EXPECT_TRUE(sawParties);
    EXPECT_TRUE(sawLots);
}
