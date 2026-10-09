#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Bot.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Match.hpp>
#include <variant>
#include <vector>

using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitTradeResponse;
using sgl::syndicate::Bid;
using sgl::syndicate::BotPersona;
using sgl::syndicate::BuyCard;
using sgl::syndicate::BuyMomentum;
using sgl::syndicate::BuyShares;
using sgl::syndicate::chooseCommand;
using sgl::syndicate::ChooseDestination;
using sgl::syndicate::Command;
using sgl::syndicate::EndTurn;
using sgl::syndicate::GameOver;
using sgl::syndicate::Manage;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::ProposeTrade;
using sgl::syndicate::RespondTrade;
using sgl::syndicate::RollDice;
using sgl::syndicate::SectorTile;
using sgl::syndicate::ShareLot;
using sgl::syndicate::sharePrice;
using sgl::syndicate::TradeOffer;
using sgl::syndicate::Upgrade;

namespace
{

[[nodiscard]] std::size_t playerIndex(PlayerId id) noexcept
{
    return static_cast<std::size_t>(id);
}

void assertShareInvariant(const Match& m)
{
    for(std::size_t n = 0; n < m.state().board.size(); ++n)
    {
        const NodeId id{static_cast<std::uint16_t>(n)};
        if(!std::holds_alternative<SectorTile>(m.state().board.node(id).kind))
        {
            continue;
        }
        unsigned sum = m.state().ownership.holdings(id).bank;
        for(std::size_t p = 0; p < sgl::syndicate::kMaxPlayers; ++p)
        {
            sum += m.state().ownership.holdings(id).shares[p];
        }
        ASSERT_EQ(sum, 10u);
    }
}

[[nodiscard]] bool sameCommand(const Command& a, const Command& b)
{
    if(a.index() != b.index())
    {
        return false;
    }
    return std::visit(
        [&]<typename T>(const T& lhs) {
            const T* rhs = std::get_if<T>(&b);
            if(!rhs)
            {
                return false;
            }
            if constexpr(requires { lhs == *rhs; })
            {
                return lhs == *rhs;
            }
            else
            {
                return true;
            }
        },
        a);
}

[[nodiscard]] bool isMemberOf(const Command& cmd, const std::vector<Command>& legal)
{
    for(const Command& c: legal)
    {
        if(sameCommand(c, cmd))
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool isSpendingCommand(const Command& cmd)
{
    return std::holds_alternative<BuyShares>(cmd) || std::holds_alternative<Bid>(cmd)
           || std::holds_alternative<Upgrade>(cmd) || std::holds_alternative<BuyCard>(cmd)
           || std::holds_alternative<BuyMomentum>(cmd);
}

[[nodiscard]] bool isSectorNode(const Match& match, NodeId id)
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
        if(isSectorNode(match, opt.destination))
        {
            return ChooseDestination{opt.destination, opt.passesPayday};
        }
    }
    return std::nullopt;
}

struct ManageSetup
{
    Match match;
    NodeId tile{};
    bool bought{false};
};

[[nodiscard]] std::optional<ManageSetup> driveToManage()
{
    for(std::uint64_t seed = 1; seed <= 80; ++seed)
    {
        auto created = Match::create(MatchConfig{.seed = seed, .seats = 2, .startCash = Money{1500}});
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
                if(!std::holds_alternative<BuyMomentum>(cmd))
                {
                    continue;
                }
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
        if(std::holds_alternative<Manage>(match.state().phase))
        {
            return ManageSetup{.match = std::move(match), .bought = false};
        }
    }
    return std::nullopt;
}

[[nodiscard]] PlayerId otherSeat(const Match& match, PlayerId from)
{
    for(int i = 0; i < match.state().seats; ++i)
    {
        const PlayerId id{static_cast<std::uint8_t>(i)};
        if(id != from)
        {
            return id;
        }
    }
    return from;
}

} // namespace

TEST(BotTest, FreshMatchChoosesRollDice)
{
    auto created = Match::create(MatchConfig{});
    ASSERT_TRUE(created.has_value());
    Match m = std::move(*created);
    const BotPersona persona{};
    const Command cmd = chooseCommand(m, m.actingPlayer(), persona);
    EXPECT_TRUE(std::holds_alternative<RollDice>(cmd));
    const auto legal = m.legalCommands(m.actingPlayer());
    ASSERT_FALSE(legal.empty());
    EXPECT_TRUE(isMemberOf(cmd, legal));
}

TEST(BotTest, FiftySeedsFourBotsReachGameOver)
{
    const BotPersona persona{.riskAppetite = {100}, .cashReserve = Money{200}};
    for(std::uint64_t seed = 1; seed <= 50; ++seed)
    {
        auto created = Match::create(MatchConfig{
            .seed = seed,
            .seats = 4,
            .maxRounds = 6,
            .targetNetWorth = Money{100000000},
        });
        ASSERT_TRUE(created.has_value()) << "seed " << seed;
        Match m = std::move(*created);

        for(int step = 0; step < 800; ++step)
        {
            if(std::holds_alternative<GameOver>(m.state().phase))
            {
                break;
            }
            const PlayerId actor = m.actingPlayer();
            const Command cmd = chooseCommand(m, actor, persona);
            auto ok = m.apply(actor, cmd);
            ASSERT_TRUE(ok.has_value()) << "seed " << seed << " step " << step;
            assertShareInvariant(m);
        }
        ASSERT_TRUE(std::holds_alternative<GameOver>(m.state().phase)) << "seed " << seed << " did not finish";
    }
}

TEST(BotTest, DoesNotBuyIntoNegativeCash)
{
    const BotPersona persona{.riskAppetite = {100}, .cashReserve = Money{200}};
    auto created = Match::create(MatchConfig{
        .seed = 7,
        .seats = 4,
        .maxRounds = 6,
        .targetNetWorth = Money{100000000},
    });
    ASSERT_TRUE(created.has_value());
    Match m = std::move(*created);

    for(int step = 0; step < 800; ++step)
    {
        if(std::holds_alternative<GameOver>(m.state().phase))
        {
            break;
        }
        const PlayerId actor = m.actingPlayer();
        const Money cashBefore = m.state().cash[playerIndex(actor)];
        const Command cmd = chooseCommand(m, actor, persona);

        if(const auto* buy = std::get_if<BuyShares>(&cmd))
        {
            const auto* purch = std::get_if<sgl::syndicate::AwaitPurchase>(&m.state().phase);
            ASSERT_NE(purch, nullptr);
            const Money unit = sharePrice(m.state().board, m.state().market, purch->tile);
            const Money cost = unit * static_cast<int>(buy->count);
            ASSERT_FALSE(cashBefore < cost) << "BuyShares would go negative at step " << step;
        }

        auto ok = m.apply(actor, cmd);
        ASSERT_TRUE(ok.has_value()) << "step " << step;
        const Money cashAfter = m.state().cash[playerIndex(actor)];
        if(cashBefore.v >= 0)
        {
            if(cashAfter.v < 0)
            {
                EXPECT_FALSE(isSpendingCommand(cmd)) << "spending cmd caused debt at step " << step;
            }
        }
        if(std::holds_alternative<BuyShares>(cmd))
        {
            EXPECT_GE(cashAfter.v, 0) << "BuyShares left negative cash at step " << step;
        }
    }
}

TEST(BotTest, ChosenCommandIsAlwaysLegal)
{
    const BotPersona persona{.riskAppetite = {100}, .cashReserve = Money{200}};
    auto created = Match::create(MatchConfig{
        .seed = 11,
        .seats = 4,
        .maxRounds = 6,
        .targetNetWorth = Money{100000000},
    });
    ASSERT_TRUE(created.has_value());
    Match m = std::move(*created);

    int samples = 0;
    for(int step = 0; step < 800 && samples < 20; ++step)
    {
        if(std::holds_alternative<GameOver>(m.state().phase))
        {
            break;
        }
        const PlayerId actor = m.actingPlayer();
        const auto legal = m.legalCommands(actor);
        ASSERT_FALSE(legal.empty()) << "step " << step;
        const Command cmd = chooseCommand(m, actor, persona);
        EXPECT_TRUE(isMemberOf(cmd, legal)) << "step " << step;
        ++samples;
        ASSERT_TRUE(m.apply(actor, cmd).has_value()) << "step " << step;
    }
    EXPECT_EQ(samples, 20);
}

TEST(BotTest, AcceptsRespondTradeWhenNetFavorsSelf)
{
    auto setup = driveToManage();
    ASSERT_TRUE(setup.has_value());
    Match& match = setup->match;
    const PlayerId from = match.actingPlayer();
    const PlayerId to = otherSeat(match, from);
    TradeOffer offer{
        .from = from,
        .to = to,
        .cashFrom = Money{100},
        .cashTo = Money{0},
        .sharesFrom = {},
        .sharesTo = {},
    };
    if(setup->bought)
    {
        offer.sharesFrom.push_back(ShareLot{.tile = setup->tile, .count = 1});
    }
    ASSERT_TRUE(match.apply(from, ProposeTrade{offer}));
    ASSERT_TRUE(std::holds_alternative<AwaitTradeResponse>(match.state().phase));

    const Command cmd = chooseCommand(match, to, BotPersona{});
    const auto* respond = std::get_if<RespondTrade>(&cmd);
    ASSERT_NE(respond, nullptr);
    EXPECT_TRUE(respond->accept);
}

TEST(BotTest, RejectsRespondTradeWhenNetDoesNotFavorSelf)
{
    auto setup = driveToManage();
    ASSERT_TRUE(setup.has_value());
    Match& match = setup->match;
    const PlayerId from = match.actingPlayer();
    const PlayerId to = otherSeat(match, from);
    const TradeOffer offer{
        .from = from,
        .to = to,
        .cashFrom = Money{0},
        .cashTo = Money{400},
        .sharesFrom = {},
        .sharesTo = {},
    };
    ASSERT_TRUE(match.apply(from, ProposeTrade{offer}));

    const Command cmd = chooseCommand(match, to, BotPersona{});
    const auto* respond = std::get_if<RespondTrade>(&cmd);
    ASSERT_NE(respond, nullptr);
    EXPECT_FALSE(respond->accept);
}
