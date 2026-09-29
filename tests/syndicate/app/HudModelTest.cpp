#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <string>
#include <syndicate/app/HudModel.hpp>
#include <syndicate/sim/Auction.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Trade.hpp>
#include <variant>

using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitRoll;
using sgl::syndicate::AwaitTradeResponse;
using sgl::syndicate::GameOver;
using sgl::syndicate::HudModel;
using sgl::syndicate::InAuction;
using sgl::syndicate::Manage;
using sgl::syndicate::NodeId;
using sgl::syndicate::Phase;
using sgl::syndicate::TradeOffer;

TEST(HudModelTest, phasePromptContainsPhaseWords)
{
    HudModel hud;
    EXPECT_NE(hud.phasePrompt(Phase{AwaitRoll{}}).find("Roll"), std::string::npos);
    EXPECT_NE(hud.phasePrompt(Phase{AwaitPath{}}).find("Path"), std::string::npos);
    EXPECT_NE(hud.phasePrompt(Phase{AwaitPurchase{.tile = NodeId{0}, .maxCount = 1}}).find("Buy"), std::string::npos);
    EXPECT_NE(hud.phasePrompt(Phase{InAuction{}}).find("Bid"), std::string::npos);
    EXPECT_NE(hud.phasePrompt(Phase{AwaitTradeResponse{.offer = TradeOffer{}, .cardsPlayed = 0, .tradesProposed = 0}})
                  .find("Trade"),
              std::string::npos);
    EXPECT_NE(hud.phasePrompt(Phase{Manage{}}).find("Manage"), std::string::npos);
    EXPECT_NE(hud.phasePrompt(Phase{GameOver{}}).find("Over"), std::string::npos);
}

TEST(HudModelTest, notesExpireAfterTtl)
{
    HudModel hud;
    hud.pushNote("hello", 1.f);
    EXPECT_EQ(hud.noteCount(), 1u);

    hud.update(sgl::Seconds{0.5f});
    EXPECT_EQ(hud.noteCount(), 1u);

    hud.update(sgl::Seconds{1.1f});
    EXPECT_EQ(hud.noteCount(), 0u);
}
