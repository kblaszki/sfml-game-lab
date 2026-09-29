#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <syndicate/app/Describe.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>

using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::BuyMomentum;
using sgl::syndicate::BuyShares;
using sgl::syndicate::ChooseDestination;
using sgl::syndicate::Label;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::Sector;
using sgl::syndicate::SectorTile;
using sgl::syndicate::describeCommand;
using sgl::syndicate::formatMoney;
using sgl::syndicate::sectorName;

namespace
{

[[nodiscard]] PlayerId P(std::uint8_t i)
{
    return PlayerId{i};
}

[[nodiscard]] bool isBiotech(const Match& m, NodeId id)
{
    const auto* tile = std::get_if<SectorTile>(&m.state().board.node(id).kind);
    return tile != nullptr && tile->sector == Sector::Biotech;
}

[[nodiscard]] std::optional<ChooseDestination> findBiotechDestination(const Match& m)
{
    const auto* path = std::get_if<AwaitPath>(&m.state().phase);
    if(!path)
    {
        return std::nullopt;
    }
    for(const auto& opt: path->options)
    {
        if(isBiotech(m, opt.destination))
        {
            return ChooseDestination{opt.destination, opt.passesPayday};
        }
    }
    return std::nullopt;
}

} // namespace

TEST(DescribeTest, formatMoneyScales)
{
    EXPECT_EQ(formatMoney(Money{0}), "$0");
    EXPECT_EQ(formatMoney(Money{150}), "$150k");
    EXPECT_EQ(formatMoney(Money{1000}), "$1M");
    EXPECT_EQ(formatMoney(Money{1500}), "$1.5M");
    EXPECT_EQ(formatMoney(Money{2000}), "$2M");
    EXPECT_EQ(formatMoney(Money{-150}), "-$150k");
    EXPECT_EQ(formatMoney(Money{-1500}), "-$1.5M");
}

TEST(DescribeTest, sectorNamesMatchEnum)
{
    EXPECT_EQ(sectorName(Sector::AI), "AI");
    EXPECT_EQ(sectorName(Sector::CleanEnergy), "Clean Energy");
    EXPECT_EQ(sectorName(Sector::Biotech), "Biotech");
    EXPECT_EQ(sectorName(Sector::CyberSec), "CyberSec");
    EXPECT_EQ(sectorName(Sector::Logistics), "Logistics");
    EXPECT_EQ(sectorName(Sector::Fintech), "Fintech");
}

TEST(DescribeTest, buySharesOnBiotechMentionsShareCostAndPercent)
{
    std::optional<Match> found;
    for(std::uint64_t seed = 1; seed <= 200 && !found; ++seed)
    {
        auto created = Match::create(MatchConfig{.seed = seed, .seats = 2});
        ASSERT_TRUE(created.has_value());
        Match m = std::move(*created);
        ASSERT_TRUE(m.apply(P(0), sgl::syndicate::RollDice{}));
        auto dest = findBiotechDestination(m);
        if(!dest)
        {
            for(const auto& cmd: m.legalCommands(P(0)))
            {
                if(std::holds_alternative<BuyMomentum>(cmd))
                {
                    ASSERT_TRUE(m.apply(P(0), cmd));
                    dest = findBiotechDestination(m);
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
        if(!isBiotech(m, std::get<AwaitPurchase>(m.state().phase).tile))
        {
            continue;
        }
        found = std::move(m);
    }
    ASSERT_TRUE(found.has_value()) << "no Biotech AwaitPurchase within seed search";

    const Label label = describeCommand(found->state(), P(0), BuyShares{1});
    EXPECT_NE(label.text.find("Buy 1 share"), std::string::npos) << label.text;
    EXPECT_NE(label.text.find("Biotech"), std::string::npos) << label.text;
    EXPECT_NE(label.text.find("10%"), std::string::npos) << label.text;
    EXPECT_NE(label.text.find("Cost: $"), std::string::npos) << label.text;
    EXPECT_FALSE(label.tooltip.empty());
}
