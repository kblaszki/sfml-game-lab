#include <cstdint>
#include <gtest/gtest.h>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Match.hpp>
#include <variant>

using sgl::syndicate::Command;
using sgl::syndicate::EndTurn;
using sgl::syndicate::GameOver;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayerId;
using sgl::syndicate::SectorTile;

namespace
{

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

} // namespace

TEST(MatchPropertyTest, SeededGamesReachGameOverWithInvariants)
{
    for(std::uint64_t seed = 1; seed <= 200; ++seed)
    {
        auto created = Match::create(MatchConfig{
            .seed = seed,
            .seats = 4,
            .maxRounds = 8,
            .targetNetWorth = Money{100000000},
        });
        ASSERT_TRUE(created.has_value()) << "seed " << seed;
        Match m = std::move(*created);

        for(int step = 0; step < 400; ++step)
        {
            if(std::holds_alternative<GameOver>(m.state().phase))
            {
                break;
            }
            const PlayerId actor = m.actingPlayer();
            auto legal = m.legalCommands(actor);
            ASSERT_FALSE(legal.empty()) << "seed " << seed << " step " << step << " empty legal";
            const std::size_t pick = static_cast<std::size_t>(seed + static_cast<std::uint64_t>(step)) % legal.size();
            auto ok = m.apply(actor, legal[pick]);
            ASSERT_TRUE(ok.has_value()) << "seed " << seed << " step " << step;
            assertShareInvariant(m);
            ASSERT_LT(m.state().phase.index(), std::variant_size_v<sgl::syndicate::Phase>);
        }
        ASSERT_TRUE(std::holds_alternative<GameOver>(m.state().phase)) << "seed " << seed << " did not finish";
    }
}
