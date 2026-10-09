#include <gtest/gtest.h>
#include <sgl/render/RenderQueue.hpp>
#include <syndicate/app/InsiderPeekBanner.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Match.hpp>
#include <syndicate/sim/Money.hpp>

using sgl::syndicate::InsiderPeekBanner;
using sgl::syndicate::MarketEventCard;
using sgl::syndicate::Match;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::Sector;
using sgl::syndicate::defaultTheme;

namespace
{

[[nodiscard]] Match makeMatch()
{
    auto m = Match::create(
        MatchConfig{.seed = 3, .seats = 2, .startCash = Money{1500}, .maxRounds = 15, .targetNetWorth = Money{6000}});
    EXPECT_TRUE(m.has_value());
    return std::move(*m);
}

} // namespace

TEST(InsiderPeekBannerShould, StayHiddenWithoutReveal)
{
    InsiderPeekBanner banner;
    Match match = makeMatch();
    EXPECT_FALSE(banner.visible(match.state()));
    sgl::RenderQueue queue;
    banner.render(queue, match.state(), sgl::FontId{1}, defaultTheme());
    EXPECT_EQ(queue.size(), 0u);
}

TEST(InsiderPeekBannerShould, DrawWhenRevealed)
{
    InsiderPeekBanner banner;
    Match match = makeMatch();
    sgl::syndicate::GameState state = match.state();
    state.revealed = MarketEventCard{.sector = Sector::AI, .delta = 20};
    EXPECT_TRUE(banner.visible(state));
    sgl::RenderQueue queue;
    banner.render(queue, state, sgl::FontId{1}, defaultTheme());
    EXPECT_GT(queue.size(), 0u);
}
