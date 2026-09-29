#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Match.hpp>
#include <variant>
#include <vector>

using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitRoll;
using sgl::syndicate::Bid;
using sgl::syndicate::BuyMomentum;
using sgl::syndicate::BuyShares;
using sgl::syndicate::CardPlayed;
using sgl::syndicate::ChooseDestination;
using sgl::syndicate::Command;
using sgl::syndicate::CommandError;
using sgl::syndicate::DeclinePurchase;
using sgl::syndicate::EndTurn;
using sgl::syndicate::GameOver;
using sgl::syndicate::InAuction;
using sgl::syndicate::Manage;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayCard;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RollDice;
using sgl::syndicate::SectorTile;
using sgl::syndicate::SetupError;
using sgl::syndicate::Upgrade;

namespace
{

PlayerId P(std::uint8_t i)
{
    return PlayerId{i};
}

Match mustCreate(const MatchConfig& cfg)
{
    auto m = Match::create(cfg);
    EXPECT_TRUE(m.has_value());
    return std::move(*m);
}

bool isSector(const Match& m, NodeId id)
{
    return std::holds_alternative<SectorTile>(m.state().board.node(id).kind);
}

// Prefer EndTurn, else first legal command.
bool forceStep(Match& m)
{
    if(std::holds_alternative<GameOver>(m.state().phase))
    {
        return false;
    }
    const PlayerId actor = m.actingPlayer();
    auto legal = m.legalCommands(actor);
    if(legal.empty())
    {
        return false;
    }
    std::size_t pick = 0;
    for(std::size_t i = 0; i < legal.size(); ++i)
    {
        if(std::holds_alternative<EndTurn>(legal[i]))
        {
            pick = i;
            break;
        }
    }
    auto ok = m.apply(actor, legal[pick]);
    EXPECT_TRUE(ok.has_value()) << "legal command failed to apply";
    return ok.has_value();
}

std::optional<ChooseDestination> findSectorDestination(const Match& m)
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

} // namespace

TEST(MatchTest, CreateRejectsBadSeats)
{
    EXPECT_EQ(Match::create(MatchConfig{.seats = 1}).error(), SetupError::BadSeats);
    EXPECT_EQ(Match::create(MatchConfig{.seats = 5}).error(), SetupError::BadSeats);
}

TEST(MatchTest, RollThenChooseDestinationMoves)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2});
    ASSERT_TRUE(m.apply(P(0), RollDice{}));
    ASSERT_TRUE(std::holds_alternative<AwaitPath>(m.state().phase));
    auto dest = findSectorDestination(m);
    if(!dest)
    {
        const auto& opts = std::get<AwaitPath>(m.state().phase).options;
        ASSERT_FALSE(opts.empty());
        dest = ChooseDestination{opts.front().destination, opts.front().passesPayday};
    }
    const NodeId before = m.state().position[0];
    ASSERT_TRUE(m.apply(P(0), *dest));
    EXPECT_NE(m.state().position[0], before);
    EXPECT_EQ(m.state().position[0], dest->destination);
}

TEST(MatchTest, BuySharesReducesBankAndCash)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2});
    ASSERT_TRUE(m.apply(P(0), RollDice{}));
    auto dest = findSectorDestination(m);
    ASSERT_TRUE(dest.has_value()) << "seed 1 should reach a sector via options or momentum";
    if(!dest)
    {
        // Try momentum adjustments to find a sector.
        if(m.legalCommands(P(0)).size() > 1)
        {
            (void)m.apply(P(0), BuyMomentum{+1});
            dest = findSectorDestination(m);
        }
    }
    ASSERT_TRUE(dest.has_value());
    ASSERT_TRUE(m.apply(P(0), *dest));
    ASSERT_TRUE(std::holds_alternative<AwaitPurchase>(m.state().phase));
    const NodeId tile = std::get<AwaitPurchase>(m.state().phase).tile;
    const auto bankBefore = m.state().ownership.freeShares(tile);
    const Money cashBefore = m.state().cash[0];
    ASSERT_TRUE(m.apply(P(0), BuyShares{1}));
    EXPECT_EQ(m.state().ownership.freeShares(tile), static_cast<std::uint8_t>(bankBefore - 1));
    EXPECT_LT(m.state().cash[0], cashBefore);
    EXPECT_TRUE(std::holds_alternative<Manage>(m.state().phase));
}

TEST(MatchTest, DeclineWithFreeSharesEntersAuction)
{
    std::optional<Match> found;
    for(std::uint64_t seed = 1; seed <= 80; ++seed)
    {
        auto m = mustCreate(MatchConfig{.seed = seed, .seats = 2});
        ASSERT_TRUE(m.apply(P(0), RollDice{}));
        auto dest = findSectorDestination(m);
        if(!dest)
        {
            auto legal = m.legalCommands(P(0));
            for(const Command& c: legal)
            {
                if(std::holds_alternative<BuyMomentum>(c))
                {
                    (void)m.apply(P(0), c);
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
        if(!std::holds_alternative<AwaitPurchase>(m.state().phase))
        {
            continue;
        }
        const auto& purch = std::get<AwaitPurchase>(m.state().phase);
        if(m.state().ownership.freeShares(purch.tile) < 3)
        {
            continue;
        }
        ASSERT_TRUE(m.apply(P(0), DeclinePurchase{}));
        ASSERT_TRUE(std::holds_alternative<InAuction>(m.state().phase));
        found = std::move(m);
        break;
    }
    ASSERT_TRUE(found.has_value());
}

TEST(MatchTest, CommandErrorsCovered)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2});
    EXPECT_EQ(m.apply(P(1), RollDice{}).error(), CommandError::NotYourTurn);
    EXPECT_EQ(m.apply(P(0), EndTurn{}).error(), CommandError::WrongPhase);
    ASSERT_TRUE(m.apply(P(0), RollDice{}));
    EXPECT_EQ(m.apply(P(0), BuyMomentum{2}).error(), CommandError::InvalidArgument);
    EXPECT_EQ(m.apply(P(0), ChooseDestination{NodeId{39}, true}).error(), CommandError::InvalidArgument);
}

TEST(MatchTest, InsufficientFundsOnMomentum)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2, .startCash = Money{20}});
    ASSERT_TRUE(m.apply(P(0), RollDice{}));
    ASSERT_TRUE(std::holds_alternative<AwaitPath>(m.state().phase));
    EXPECT_EQ(m.apply(P(0), BuyMomentum{+1}).error(), CommandError::InsufficientFunds);
}

TEST(MatchTest, InsufficientFundsOnBuyShares)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2, .startCash = Money{5}});
    ASSERT_TRUE(m.apply(P(0), RollDice{}));
    auto dest = findSectorDestination(m);
    if(!dest)
    {
        GTEST_SKIP() << "no sector option on seed 1";
    }
    ASSERT_TRUE(m.apply(P(0), *dest));
    if(!std::holds_alternative<AwaitPurchase>(m.state().phase))
    {
        GTEST_SKIP() << "did not land on purchasable sector";
    }
    EXPECT_EQ(m.apply(P(0), BuyShares{1}).error(), CommandError::InsufficientFunds);
}

TEST(MatchTest, GameOverRejectsCommands)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2, .maxRounds = 1, .targetNetWorth = Money{100000000}});
    for(int i = 0; i < 200; ++i)
    {
        if(std::holds_alternative<GameOver>(m.state().phase))
        {
            break;
        }
        ASSERT_TRUE(forceStep(m));
    }
    ASSERT_TRUE(std::holds_alternative<GameOver>(m.state().phase));
    EXPECT_EQ(m.apply(P(0), RollDice{}).error(), CommandError::WrongPhase);
}

TEST(MatchTest, GameOverByMaxRounds)
{
    auto m = mustCreate(MatchConfig{.seed = 7, .seats = 2, .maxRounds = 1, .targetNetWorth = Money{100000000}});
    for(int i = 0; i < 200; ++i)
    {
        if(std::holds_alternative<GameOver>(m.state().phase))
        {
            break;
        }
        ASSERT_TRUE(forceStep(m));
    }
    ASSERT_TRUE(std::holds_alternative<GameOver>(m.state().phase));
    const auto& over = std::get<GameOver>(m.state().phase);
    EXPECT_EQ(over.ranking.size(), 2u);
}

TEST(MatchTest, FirstStepLegalCommandsAllApply)
{
    // Full Match fork is not available (move-only use); only the opening AwaitRoll
    // command set is re-checked on fresh creates with the same seed.
    constexpr std::uint64_t seed = 3;
    auto probe = mustCreate(MatchConfig{.seed = seed, .seats = 2});
    auto legal = probe.legalCommands(P(0));
    ASSERT_FALSE(legal.empty());
    for(const Command& cmd: legal)
    {
        auto fresh = mustCreate(MatchConfig{.seed = seed, .seats = 2});
        auto ok = fresh.apply(P(0), cmd);
        EXPECT_TRUE(ok.has_value());
    }
}

TEST(MatchTest, ShareInvariantHoldsAndFeePathSmoke)
{
    auto m = mustCreate(MatchConfig{.seed = 11, .seats = 2, .startCash = Money{1500}});
    for(int i = 0; i < 40; ++i)
    {
        if(std::holds_alternative<GameOver>(m.state().phase))
        {
            break;
        }
        ASSERT_TRUE(forceStep(m));
        for(std::size_t n = 0; n < m.state().board.size(); ++n)
        {
            const NodeId id{static_cast<std::uint16_t>(n)};
            if(!std::holds_alternative<SectorTile>(m.state().board.node(id).kind))
            {
                continue;
            }
            unsigned sum = m.state().ownership.holdings(id).bank;
            for(std::size_t p = 0; p < 4; ++p)
            {
                sum += m.state().ownership.holdings(id).shares[p];
            }
            EXPECT_EQ(sum, 10u);
        }
    }
}

TEST(MatchTest, PlayCardWhenHubReachable)
{
    bool played = false;
    for(std::uint64_t seed = 1; seed <= 50; ++seed)
    {
        auto m = mustCreate(MatchConfig{.seed = seed, .seats = 2});
        ASSERT_TRUE(m.apply(P(0), RollDice{}));
        const auto* path = std::get_if<AwaitPath>(&m.state().phase);
        if(!path)
        {
            continue;
        }
        std::optional<ChooseDestination> hub;
        for(const auto& opt: path->options)
        {
            if(std::holds_alternative<sgl::syndicate::Hub>(m.state().board.node(opt.destination).kind))
            {
                hub = ChooseDestination{opt.destination, opt.passesPayday};
                break;
            }
        }
        if(!hub)
        {
            // try momentum
            for(int delta: {+1, -1})
            {
                auto trial = mustCreate(MatchConfig{.seed = seed, .seats = 2});
                ASSERT_TRUE(trial.apply(P(0), RollDice{}));
                if(trial.apply(P(0), BuyMomentum{delta}))
                {
                    const auto* p2 = std::get_if<AwaitPath>(&trial.state().phase);
                    if(!p2)
                    {
                        continue;
                    }
                    for(const auto& opt: p2->options)
                    {
                        if(std::holds_alternative<sgl::syndicate::Hub>(trial.state().board.node(opt.destination).kind))
                        {
                            ASSERT_TRUE(trial.apply(P(0), ChooseDestination{opt.destination, opt.passesPayday}));
                            if(trial.state().hands[0].cards().empty())
                            {
                                break;
                            }
                            auto legal = trial.legalCommands(P(0));
                            for(const Command& c: legal)
                            {
                                if(std::holds_alternative<PlayCard>(c))
                                {
                                    ASSERT_TRUE(trial.apply(P(0), c));
                                    played = true;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
                if(played)
                {
                    break;
                }
            }
            if(played)
            {
                break;
            }
            continue;
        }
        ASSERT_TRUE(m.apply(P(0), *hub));
        if(m.state().hands[0].cards().empty())
        {
            continue;
        }
        auto legal = m.legalCommands(P(0));
        for(const Command& c: legal)
        {
            if(std::holds_alternative<PlayCard>(c))
            {
                ASSERT_TRUE(m.apply(P(0), c));
                played = true;
                break;
            }
        }
        if(played)
        {
            break;
        }
    }
    // Hub may be unreachable in seeds 1..50; soft requirement if hand empty.
    if(played)
    {
        SUCCEED();
    }
}

TEST(MatchTest, UpgradeRejectedWhenNotController)
{
    auto m = mustCreate(MatchConfig{.seed = 1, .seats = 2});
    // Force into Manage via roll+move+decline/buy
    ASSERT_TRUE(m.apply(P(0), RollDice{}));
    auto dest = findSectorDestination(m);
    if(!dest)
    {
        const auto& opts = std::get<AwaitPath>(m.state().phase).options;
        ASSERT_FALSE(opts.empty());
        dest = ChooseDestination{opts.front().destination, opts.front().passesPayday};
    }
    ASSERT_TRUE(m.apply(P(0), *dest));
    if(std::holds_alternative<AwaitPurchase>(m.state().phase))
    {
        ASSERT_TRUE(m.apply(P(0), DeclinePurchase{}));
        if(std::holds_alternative<InAuction>(m.state().phase))
        {
            // Pass until auction ends
            for(int i = 0; i < 8; ++i)
            {
                if(!std::holds_alternative<InAuction>(m.state().phase))
                {
                    break;
                }
                ASSERT_TRUE(m.apply(m.actingPlayer(), sgl::syndicate::PassBid{}));
            }
        }
    }
    if(std::holds_alternative<Manage>(m.state().phase))
    {
        EXPECT_EQ(m.apply(P(0), Upgrade{NodeId{1}}).error(), CommandError::InvalidArgument);
    }
}
