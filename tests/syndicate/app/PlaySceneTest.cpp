#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <sgl/core/Time.hpp>
#include <sgl/core/Vec2.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <string>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/Describe.hpp>
#include <syndicate/app/LayoutView.hpp>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/sim/BoardGraph.hpp>
#include <syndicate/sim/Commands.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Ids.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>
#include <vector>

using sgl::syndicate::AwaitPath;
using sgl::syndicate::AwaitPurchase;
using sgl::syndicate::AwaitRoll;
using sgl::syndicate::BuyShares;
using sgl::syndicate::Command;
using sgl::syndicate::Hub;
using sgl::syndicate::LayoutView;
using sgl::syndicate::Manage;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::NodeId;
using sgl::syndicate::PlayCard;
using sgl::syndicate::PlayScene;
using sgl::syndicate::PlayerId;
using sgl::syndicate::RespondTrade;
using sgl::syndicate::Seat;
using sgl::syndicate::SectorTile;
using sgl::syndicate::SyndicateActions;
using sgl::syndicate::Upgrade;
using sgl::syndicate::bindSyndicate;

namespace
{

[[nodiscard]] sgl::SceneFactory unusedPauseFactory()
{
    return []() -> std::unique_ptr<sgl::SceneI> { return nullptr; };
}

struct Fixture
{
    sgl::ActionMap map{};
    SyndicateActions actions = bindSyndicate(map);
    sgl::FontId font{.id = 1};
};

[[nodiscard]] sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

[[nodiscard]] sgl::InputState pointerUpAt(sgl::Vec2f pos, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::MouseUp{.button = sgl::MouseButton::Left, .pos = pos}, map);
    return input;
}

[[nodiscard]] std::vector<Seat> allBots(int n)
{
    std::vector<Seat> seats;
    seats.reserve(static_cast<std::size_t>(n));
    for(int i = 0; i < n; ++i)
    {
        seats.push_back(Seat{.bot = true});
    }
    return seats;
}

[[nodiscard]] std::size_t buySharesSlot(const std::vector<Command>& legal, std::uint8_t count)
{
    for(std::size_t i = 0; i < legal.size(); ++i)
    {
        if(const auto* buy = std::get_if<BuyShares>(&legal[i]))
        {
            if(buy->count == count)
            {
                return i;
            }
        }
    }
    return legal.size();
}

[[nodiscard]] std::size_t manageActionSlot(const std::vector<Command>& legal)
{
    for(std::size_t i = 0; i < legal.size(); ++i)
    {
        if(std::holds_alternative<Upgrade>(legal[i]) || std::holds_alternative<PlayCard>(legal[i]) ||
           std::holds_alternative<RespondTrade>(legal[i]))
        {
            return i;
        }
    }
    return legal.size();
}

[[nodiscard]] std::vector<Command> barLegal(const std::vector<Command>& legal)
{
    std::vector<Command> out;
    out.reserve(legal.size());
    for(const Command& cmd: legal)
    {
        if(!std::holds_alternative<sgl::syndicate::ChooseDestination>(cmd))
        {
            out.push_back(cmd);
        }
    }
    if(out.size() > 9)
    {
        out.resize(9);
    }
    return out;
}

[[nodiscard]] sgl::Key slotKey(std::size_t index)
{
    constexpr sgl::Key kNums[9] = {sgl::Key::Num1,
                                   sgl::Key::Num2,
                                   sgl::Key::Num3,
                                   sgl::Key::Num4,
                                   sgl::Key::Num5,
                                   sgl::Key::Num6,
                                   sgl::Key::Num7,
                                   sgl::Key::Num8,
                                   sgl::Key::Num9};
    return kNums[index];
}

} // namespace

TEST(PlaySceneTest, fourBotsReachGameOverAndPushRanking)
{
    Fixture f;
    MatchConfig cfg{
        .seed = 1,
        .seats = 4,
        .maxRounds = 3,
        .targetNetWorth = Money{100000000},
    };
    auto scene = std::make_unique<PlayScene>(cfg, allBots(4), f.font, f.actions);
    PlayScene* play = scene.get();

    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::move(scene));

    for(int i = 0; i < 5000 && !play->gameOver(); ++i)
    {
        stack.update(sgl::InputState{}, sgl::Seconds{0.5f});
    }

    ASSERT_TRUE(play->gameOver());
    EXPECT_EQ(stack.size(), 2u);
    EXPECT_EQ(dynamic_cast<PlayScene*>(stack.top()), nullptr);

    EXPECT_FALSE(play->log().visible(64).empty());
    bool sawRound = false;
    for(const auto& entry: play->log().visible(64))
    {
        if(entry.text.find("Round") != std::string::npos)
        {
            sawRound = true;
            break;
        }
    }
    EXPECT_TRUE(sawRound);
}

TEST(PlaySceneTest, humanRollIgnoredOnBotTurnWithZeroDt)
{
    Fixture f;
    MatchConfig cfg{
        .seed = 1,
        .seats = 2,
        .maxRounds = 5,
        .targetNetWorth = Money{100000000},
    };
    // Bot acts first so a short think window is enough without driving a human turn.
    std::vector<Seat> seats{Seat{.bot = true}, Seat{.bot = false}};
    auto scene = std::make_unique<PlayScene>(cfg, seats, f.font, f.actions);
    PlayScene* play = scene.get();

    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::move(scene));

    stack.update(sgl::InputState{}, sgl::Seconds{0.1f});
    ASSERT_NE(play->match(), nullptr);
    ASSERT_TRUE(std::holds_alternative<AwaitRoll>(play->match()->state().phase));
    const auto actorBefore = play->match()->actingPlayer();

    stack.update(inputWith(sgl::Key::Space, f.map), sgl::Seconds{0.f});

    ASSERT_NE(play->match(), nullptr);
    EXPECT_EQ(play->match()->actingPlayer(), actorBefore);
    EXPECT_TRUE(std::holds_alternative<AwaitRoll>(play->match()->state().phase));
}

TEST(PlaySceneTest, humanBuysViaNumAndAppliesManageUpgradeOrPlayCard)
{
    Fixture f;
    MatchConfig cfg{
        .seed = 7,
        .seats = 2,
        .startCash = Money{5000},
        .maxRounds = 15,
        .targetNetWorth = Money{100000000},
    };
    std::vector<Seat> seats{Seat{.bot = false}, Seat{.bot = true}};
    auto scene = std::make_unique<PlayScene>(cfg, seats, f.font, f.actions);
    PlayScene* play = scene.get();

    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::move(scene));

    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    bool bought = false;
    bool manageAction = false;
    std::uint8_t boughtCount = 0;

    for(int step = 0; step < 400 && !play->gameOver(); ++step)
    {
        ASSERT_NE(play->match(), nullptr);
        const auto& match = *play->match();
        const PlayerId actor = match.actingPlayer();
        const bool human = std::to_underlying(actor) == 0;

        if(!human)
        {
            stack.update(sgl::InputState{}, sgl::Seconds{0.5f});
            continue;
        }

        const auto& phase = match.state().phase;
        if(std::holds_alternative<AwaitRoll>(phase))
        {
            stack.update(inputWith(sgl::Key::Space, f.map), sgl::Seconds{0.05f});
            continue;
        }

        if(const auto* path = std::get_if<AwaitPath>(&phase))
        {
            ASSERT_FALSE(path->options.empty());
            const sgl::syndicate::MoveOption* pick = &path->options.front();
            for(const auto& opt: path->options)
            {
                if(std::holds_alternative<Hub>(match.state().board.node(opt.destination).kind))
                {
                    pick = &opt;
                    break;
                }
                if(std::holds_alternative<SectorTile>(match.state().board.node(opt.destination).kind) &&
                   match.state().ownership.freeShares(opt.destination) > 0)
                {
                    pick = &opt;
                }
            }
            const sgl::Vec2f center = layout.position(match.state().board.node(pick->destination).at);
            stack.update(pointerUpAt(center, f.map), sgl::Seconds{0.05f});
            for(int i = 0; i < 20; ++i)
            {
                stack.update(sgl::InputState{}, sgl::Seconds{0.5f});
            }
            continue;
        }

        if(std::holds_alternative<AwaitPurchase>(phase))
        {
            const auto legal = barLegal(match.legalCommands(actor));
            ASSERT_FALSE(legal.empty());
            std::size_t slot = buySharesSlot(legal, 1);
            boughtCount = 1;
            for(int n = 3; n >= 1; --n)
            {
                const std::size_t found = buySharesSlot(legal, static_cast<std::uint8_t>(n));
                if(found < legal.size())
                {
                    slot = found;
                    boughtCount = static_cast<std::uint8_t>(n);
                    break;
                }
            }
            ASSERT_LT(slot, legal.size());
            const auto* purch = std::get_if<AwaitPurchase>(&match.state().phase);
            ASSERT_NE(purch, nullptr);
            const NodeId tile = purch->tile;
            const auto bankBefore = match.state().ownership.freeShares(tile);
            stack.update(inputWith(slotKey(slot), f.map), sgl::Seconds{0.05f});
            ASSERT_NE(play->match(), nullptr);
            EXPECT_EQ(play->match()->state().ownership.freeShares(tile),
                      static_cast<std::uint8_t>(bankBefore - boughtCount));
            bought = true;
            continue;
        }

        if(std::holds_alternative<Manage>(phase))
        {
            const auto legal = barLegal(match.legalCommands(actor));
            const std::size_t actionSlot = manageActionSlot(legal);
            if(actionSlot < legal.size() && !manageAction)
            {
                stack.update(inputWith(slotKey(actionSlot), f.map), sgl::Seconds{0.05f});
                manageAction = true;
                continue;
            }
            stack.update(inputWith(sgl::Key::Enter, f.map), sgl::Seconds{0.05f});
            continue;
        }

        if(std::holds_alternative<sgl::syndicate::InAuction>(phase))
        {
            const auto legal = barLegal(match.legalCommands(actor));
            if(!legal.empty())
            {
                stack.update(inputWith(slotKey(0), f.map), sgl::Seconds{0.05f});
            }
            else
            {
                stack.update(sgl::InputState{}, sgl::Seconds{0.05f});
            }
            continue;
        }

        if(std::holds_alternative<sgl::syndicate::AwaitTradeResponse>(phase))
        {
            const auto legal = barLegal(match.legalCommands(actor));
            ASSERT_FALSE(legal.empty());
            stack.update(inputWith(slotKey(0), f.map), sgl::Seconds{0.05f});
            manageAction = true;
            continue;
        }

        stack.update(sgl::InputState{}, sgl::Seconds{0.05f});
    }

    EXPECT_TRUE(bought) << "human never reached AwaitPurchase to buy via Num";
    EXPECT_GE(boughtCount, 1);
    EXPECT_TRUE(manageAction) << "human never applied Upgrade/PlayCard/RespondTrade via slot";
}

TEST(PlaySceneTest, hoveringTileShowsDescribeTooltip)
{
    Fixture f;
    MatchConfig cfg{
        .seed = 1,
        .seats = 2,
        .maxRounds = 5,
        .targetNetWorth = Money{100000000},
    };
    std::vector<Seat> seats{Seat{.bot = false}, Seat{.bot = true}};
    auto scene = std::make_unique<PlayScene>(cfg, seats, f.font, f.actions);
    PlayScene* play = scene.get();

    sgl::SceneStack stack{unusedPauseFactory()};
    stack.push(std::move(scene));

    ASSERT_NE(play->match(), nullptr);
    const LayoutView layout{sgl::Vec2f{640.f, 360.f}, 280.f};
    const NodeId node{5};
    const sgl::Vec2f center = layout.position(play->match()->state().board.node(node).at);
    const std::string expected = sgl::syndicate::describeTile(play->match()->state(), node);

    sgl::InputState hover;
    hover.apply(sgl::MouseMove{.pos = center}, f.map);

    for(int i = 0; i < 8; ++i)
    {
        stack.update(hover, sgl::Seconds{0.05f});
    }

    sgl::RenderQueue queue;
    play->render(queue);

    bool found = false;
    for(const sgl::Entry& e: queue.sorted())
    {
        if(const auto* text = std::get_if<sgl::TextCmd>(&e.command))
        {
            if(text->text.find(expected) != std::string::npos || expected.find(text->text) != std::string::npos)
            {
                found = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found) << "tooltip missing describeTile for node 5: " << expected;
}
