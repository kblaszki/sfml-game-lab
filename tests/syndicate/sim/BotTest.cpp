#include <cstdint>
#include <gtest/gtest.h>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Bot.hpp>
#include <syndicate/sim/Market.hpp>
#include <syndicate/sim/Match.hpp>
#include <variant>
#include <vector>

using sgl::syndicate::Bid;
using sgl::syndicate::BotPersona;
using sgl::syndicate::BuyCard;
using sgl::syndicate::BuyMomentum;
using sgl::syndicate::BuyShares;
using sgl::syndicate::Command;
using sgl::syndicate::EndTurn;
using sgl::syndicate::GameOver;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RollDice;
using sgl::syndicate::SectorTile;
using sgl::syndicate::Upgrade;
using sgl::syndicate::chooseCommand;
using sgl::syndicate::sharePrice;

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
    return std::holds_alternative<BuyShares>(cmd) || std::holds_alternative<Bid>(cmd) ||
           std::holds_alternative<Upgrade>(cmd) || std::holds_alternative<BuyCard>(cmd) ||
           std::holds_alternative<BuyMomentum>(cmd);
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
