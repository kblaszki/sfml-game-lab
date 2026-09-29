#include <gtest/gtest.h>
#include <memory>
#include <sgl/core/Time.hpp>
#include <sgl/input/ActionMap.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/AssetIds.hpp>
#include <sgl/scene/SceneI.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <syndicate/app/Bindings.hpp>
#include <syndicate/app/PlayScene.hpp>
#include <syndicate/sim/GameState.hpp>
#include <syndicate/sim/Money.hpp>
#include <variant>
#include <vector>

using sgl::syndicate::AwaitRoll;
using sgl::syndicate::MatchConfig;
using sgl::syndicate::Money;
using sgl::syndicate::PlayScene;
using sgl::syndicate::Seat;
using sgl::syndicate::SyndicateActions;
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
