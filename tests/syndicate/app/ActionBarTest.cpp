#include <gtest/gtest.h>
#include <optional>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/input/MouseButton.hpp>
#include <string>
#include <syndicate/app/ActionBar.hpp>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>
#include <type_traits>
#include <variant>
#include <vector>

using sgl::syndicate::ActionBar;
using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitRoll;
using sgl::syndicate::Bid;
using sgl::syndicate::bindSyndicate;
using sgl::syndicate::BuyCard;
using sgl::syndicate::BuyMomentum;
using sgl::syndicate::BuyShares;
using sgl::syndicate::ChooseDestination;
using sgl::syndicate::Command;
using sgl::syndicate::DeclinePurchase;
using sgl::syndicate::InAuction;
using sgl::syndicate::Manage;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::PassBid;
using sgl::syndicate::PlayCard;
using sgl::syndicate::PlayerId;
using sgl::syndicate::ProposeTrade;
using sgl::syndicate::RespondTrade;
using sgl::syndicate::SectorTile;
using sgl::syndicate::SellShares;
using sgl::syndicate::SyndicateActions;
using sgl::syndicate::Upgrade;

namespace
{

[[nodiscard]] PlayerId P(std::uint8_t i)
{
    return PlayerId{i};
}

[[nodiscard]] sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
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

[[nodiscard]] std::vector<Command> expectedBarCommands(const Match& m, PlayerId actor)
{
    std::vector<Command> legal = m.legalCommands(actor);
    std::erase_if(legal, [](const Command& c) { return std::holds_alternative<ChooseDestination>(c); });
    if(legal.size() > 9)
    {
        legal.resize(9);
    }
    return legal;
}

[[nodiscard]] bool labelMentionsMoney(const std::string& title)
{
    return title.find("Cost: $") != std::string::npos || title.find("Bid $") != std::string::npos
           || title.find("Gain: $") != std::string::npos;
}

[[nodiscard]] bool commandCostsMoney(const Command& cmd)
{
    return std::holds_alternative<BuyShares>(cmd) || std::holds_alternative<Bid>(cmd)
           || std::holds_alternative<Upgrade>(cmd) || std::holds_alternative<SellShares>(cmd)
           || std::holds_alternative<BuyCard>(cmd) || std::holds_alternative<BuyMomentum>(cmd);
}

[[nodiscard]] bool sameCommand(const Command& a, const Command& b)
{
    if(a.index() != b.index())
    {
        return false;
    }
    if(const auto* left = std::get_if<BuyShares>(&a))
    {
        return left->count == std::get<BuyShares>(b).count;
    }
    if(const auto* left = std::get_if<BuyMomentum>(&a))
    {
        return left->delta == std::get<BuyMomentum>(b).delta;
    }
    if(const auto* left = std::get_if<Bid>(&a))
    {
        return left->amount == std::get<Bid>(b).amount;
    }
    if(const auto* left = std::get_if<Upgrade>(&a))
    {
        return left->tile == std::get<Upgrade>(b).tile;
    }
    if(const auto* left = std::get_if<SellShares>(&a))
    {
        const auto& right = std::get<SellShares>(b);
        return left->tile == right.tile && left->count == right.count;
    }
    if(const auto* left = std::get_if<RespondTrade>(&a))
    {
        return left->accept == std::get<RespondTrade>(b).accept;
    }
    if(const auto* left = std::get_if<PlayCard>(&a))
    {
        return left->kind == std::get<PlayCard>(b).kind;
    }
    if(const auto* left = std::get_if<ChooseDestination>(&a))
    {
        const auto& right = std::get<ChooseDestination>(b);
        return left->destination == right.destination && left->passesPayday == right.passesPayday;
    }
    if(const auto* left = std::get_if<ProposeTrade>(&a))
    {
        const auto& right = std::get<ProposeTrade>(b);
        return left->offer.from == right.offer.from && left->offer.to == right.offer.to;
    }
    return true;
}

void expectBarMatchesLegal(
    ActionBar& bar, const Match& m, PlayerId actor, const SyndicateActions& actions, const sgl::ActionMap& map)
{
    bar.rebuild(m, actor, std::nullopt);
    const std::vector<Command> expected = expectedBarCommands(m, actor);
    ASSERT_EQ(bar.size(), expected.size());
    ASSERT_EQ(bar.hotspots().size(), expected.size());
    for(const auto& spot: bar.hotspots())
    {
        EXPECT_FALSE(spot.title.empty());
        EXPECT_FALSE(spot.body.empty());
    }
    for(std::size_t i = 0; i < expected.size(); ++i)
    {
        if(commandCostsMoney(expected[i]))
        {
            EXPECT_TRUE(labelMentionsMoney(std::string{bar.hotspots()[i].title})) << bar.hotspots()[i].title;
        }
        constexpr sgl::Key kNums[9] = {sgl::Key::Num1,
                                       sgl::Key::Num2,
                                       sgl::Key::Num3,
                                       sgl::Key::Num4,
                                       sgl::Key::Num5,
                                       sgl::Key::Num6,
                                       sgl::Key::Num7,
                                       sgl::Key::Num8,
                                       sgl::Key::Num9};
        const auto pressed = bar.update(inputWith(kNums[i], map), actions);
        ASSERT_TRUE(pressed.has_value());
        EXPECT_TRUE(sameCommand(*pressed, expected[i]));
        bar.rebuild(m, actor, std::nullopt);
    }
}

[[nodiscard]] Match mustCreate(MatchConfig cfg)
{
    auto created = Match::create(cfg);
    EXPECT_TRUE(created.has_value());
    return std::move(*created);
}

} // namespace

TEST(ActionBarTest, awaitRollShowsRollOnly)
{
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    ActionBar bar;
    Match m = mustCreate(MatchConfig{.seed = 1, .seats = 2});
    ASSERT_TRUE(std::holds_alternative<AwaitRoll>(m.state().phase));
    expectBarMatchesLegal(bar, m, P(0), actions, map);
}

TEST(ActionBarTest, awaitPurchaseShowsBuysAndDecline)
{
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    ActionBar bar;

    std::optional<Match> found;
    for(std::uint64_t seed = 1; seed <= 80 && !found; ++seed)
    {
        Match m = mustCreate(MatchConfig{.seed = seed, .seats = 2});
        ASSERT_TRUE(m.apply(P(0), sgl::syndicate::RollDice{}));
        auto dest = findSectorDestination(m);
        if(!dest)
        {
            for(const auto& cmd: m.legalCommands(P(0)))
            {
                if(std::holds_alternative<BuyMomentum>(cmd))
                {
                    ASSERT_TRUE(m.apply(P(0), cmd));
                    dest = findSectorDestination(m);
                    if(dest)
                    {
                        break;
                    }
                }
            }
        }
        if(!dest)
        {
            continue;
        }
        ASSERT_TRUE(m.apply(P(0), *dest));
        if(std::holds_alternative<AwaitPurchase>(m.state().phase))
        {
            found = std::move(m);
        }
    }
    ASSERT_TRUE(found.has_value());
    expectBarMatchesLegal(bar, *found, P(0), actions, map);

    const auto first = bar.update(inputWith(sgl::Key::Num1, map), actions);
    ASSERT_TRUE(first.has_value());
    const std::vector<Command> expected = expectedBarCommands(*found, P(0));
    ASSERT_FALSE(expected.empty());
    EXPECT_TRUE(sameCommand(*first, expected.front()));
}

TEST(ActionBarTest, inAuctionAndManagePhases)
{
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    ActionBar bar;

    std::optional<Match> auction;
    std::optional<Match> manage;
    for(std::uint64_t seed = 1; seed <= 120 && (!auction || !manage); ++seed)
    {
        Match m = mustCreate(MatchConfig{.seed = seed, .seats = 2});
        ASSERT_TRUE(m.apply(P(0), sgl::syndicate::RollDice{}));
        auto dest = findSectorDestination(m);
        if(!dest)
        {
            for(const auto& cmd: m.legalCommands(P(0)))
            {
                if(std::holds_alternative<BuyMomentum>(cmd))
                {
                    ASSERT_TRUE(m.apply(P(0), cmd));
                    dest = findSectorDestination(m);
                    if(dest)
                    {
                        break;
                    }
                }
            }
        }
        if(!dest)
        {
            continue;
        }
        ASSERT_TRUE(m.apply(P(0), *dest));
        if(std::holds_alternative<AwaitPurchase>(m.state().phase) && !auction)
        {
            Match auctionMatch = mustCreate(MatchConfig{.seed = seed, .seats = 2});
            ASSERT_TRUE(auctionMatch.apply(P(0), sgl::syndicate::RollDice{}));
            auto auctionDest = findSectorDestination(auctionMatch);
            if(!auctionDest)
            {
                for(const auto& cmd: auctionMatch.legalCommands(P(0)))
                {
                    if(std::holds_alternative<BuyMomentum>(cmd))
                    {
                        ASSERT_TRUE(auctionMatch.apply(P(0), cmd));
                        auctionDest = findSectorDestination(auctionMatch);
                        if(auctionDest)
                        {
                            break;
                        }
                    }
                }
            }
            if(auctionDest)
            {
                ASSERT_TRUE(auctionMatch.apply(P(0), *auctionDest));
                if(std::holds_alternative<AwaitPurchase>(auctionMatch.state().phase))
                {
                    ASSERT_TRUE(auctionMatch.apply(P(0), DeclinePurchase{}));
                    if(std::holds_alternative<InAuction>(auctionMatch.state().phase))
                    {
                        auction = std::move(auctionMatch);
                    }
                }
            }
        }
        if(std::holds_alternative<AwaitPurchase>(m.state().phase) && !manage)
        {
            ASSERT_TRUE(m.apply(P(0), BuyShares{1}));
            if(std::holds_alternative<Manage>(m.state().phase))
            {
                manage = std::move(m);
            }
        }
        else if(std::holds_alternative<Manage>(m.state().phase) && !manage)
        {
            manage = std::move(m);
        }
    }

    ASSERT_TRUE(auction.has_value());
    expectBarMatchesLegal(bar, *auction, auction->actingPlayer(), actions, map);

    ASSERT_TRUE(manage.has_value());
    expectBarMatchesLegal(bar, *manage, manage->actingPlayer(), actions, map);
}

TEST(ActionBarTest, slotZeroReturnsFirstNonDestination)
{
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    ActionBar bar;
    Match m = mustCreate(MatchConfig{.seed = 1, .seats = 2});
    ASSERT_TRUE(m.apply(P(0), sgl::syndicate::RollDice{}));
    ASSERT_TRUE(std::holds_alternative<AwaitPath>(m.state().phase));

    bar.rebuild(m, P(0), std::nullopt);
    const std::vector<Command> expected = expectedBarCommands(m, P(0));
    ASSERT_FALSE(expected.empty());
    const auto got = bar.update(inputWith(sgl::Key::Num1, map), actions);
    ASSERT_TRUE(got.has_value());
    EXPECT_TRUE(sameCommand(*got, expected.front()));
    EXPECT_FALSE(std::holds_alternative<ChooseDestination>(*got));
}

TEST(ActionBarTest, offerTradeAddsAppSlotWithinCap)
{
    sgl::ActionMap map{};
    const SyndicateActions actions = bindSyndicate(map);
    ActionBar bar;
    Match m = mustCreate(MatchConfig{.seed = 1, .seats = 2});

    bar.rebuild(m, P(0), std::nullopt, false);
    const std::size_t base = bar.size();

    bar.rebuild(m, P(0), std::nullopt, true);
    EXPECT_EQ(bar.size(), base + 1);
    EXPECT_LE(bar.size(), 9u);
    ASSERT_FALSE(bar.hotspots().empty());
    EXPECT_NE(bar.hotspots().back().title.find("Trade"), std::string::npos);

    const auto command = bar.update(inputWith(sgl::Key::Num1, map), actions);
    ASSERT_TRUE(command.has_value());
    EXPECT_TRUE(std::holds_alternative<sgl::syndicate::RollDice>(*command));
    EXPECT_FALSE(bar.consumeTradeRequest());

    bar.rebuild(m, P(0), std::nullopt, true);
    constexpr sgl::Key kNums[9] = {sgl::Key::Num1,
                                   sgl::Key::Num2,
                                   sgl::Key::Num3,
                                   sgl::Key::Num4,
                                   sgl::Key::Num5,
                                   sgl::Key::Num6,
                                   sgl::Key::Num7,
                                   sgl::Key::Num8,
                                   sgl::Key::Num9};
    const auto tradeKey = bar.update(inputWith(kNums[bar.size() - 1], map), actions);
    EXPECT_FALSE(tradeKey.has_value());
    EXPECT_TRUE(bar.consumeTradeRequest());
    EXPECT_FALSE(bar.consumeTradeRequest());

    bar.rebuild(m, P(0), std::nullopt, true);
    const sgl::Rect<float> area = bar.hotspots().back().area;
    const sgl::Vec2f pos{area.pos.x + area.size.x * 0.5f, area.pos.y + area.size.y * 0.5f};
    sgl::InputState click;
    click.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = pos}, map);
    EXPECT_FALSE(bar.update(click, actions).has_value());
    EXPECT_TRUE(bar.consumeTradeRequest());
}
