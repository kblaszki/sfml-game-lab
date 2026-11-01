#include <dodger/app/Bindings.hpp>
#include <dodger/app/GameOverScene.hpp>
#include <dodger/app/PlayScene.hpp>
#include <dodger/app/Scenes.hpp>
#include <dodger/sim/Tuning.hpp>
#include <dodger/sim/Types.hpp>
#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/render/DrawCommand.hpp>
#include <sgl/render/RenderQueue.hpp>
#include <sgl/scene/SceneStack.hpp>

namespace
{

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::dodger::bindDodger(map)}
        , font{}
        , services{actions, font, 1}
        , stack{sgl::dodger::pause(services)}
    {
    }

    sgl::ActionMap map;
    sgl::dodger::DodgerActions actions;
    sgl::FontId font;
    sgl::dodger::AppServices services;
    sgl::SceneStack stack;
};

} // namespace

TEST(PlaySceneTest, rightMovesThePaddle)
{
    Fixture f;
    f.stack.push(sgl::dodger::play(f.services)());

    auto* play = dynamic_cast<sgl::dodger::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    const float before = play->game().paddle().pos.x;

    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = sgl::Key::Right}, f.map);
    f.stack.update(input, sgl::kTick);

    EXPECT_GT(play->game().paddle().pos.x, before);
    EXPECT_FALSE(play->game().over());
}

TEST(PlaySceneTest, drawsPaddleAndHazardsAsRects)
{
    Fixture f;
    f.services.tuning.spawnInterval = 0.01f;
    f.services.tuning.hazardSpeed = 0.f;
    f.stack.push(sgl::dodger::play(f.services)());

    f.stack.update(sgl::InputState{}, sgl::kTick);

    auto* play = dynamic_cast<sgl::dodger::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    const sgl::dodger::PaddleView paddle = play->game().paddle();
    ASSERT_EQ(play->game().hazards().size(), 1u);
    const sgl::dodger::HazardView hazard = play->game().hazards().front();

    sgl::RenderQueue queue;
    play->render(queue);

    int paddleRects = 0;
    int hazardRects = 0;
    for(const sgl::Entry& entry: queue.sorted())
    {
        const auto* rect = std::get_if<sgl::RectCmd>(&entry.command);
        if(rect == nullptr)
        {
            continue;
        }
        if(rect->rect.pos.x == paddle.pos.x && rect->rect.pos.y == paddle.pos.y && rect->rect.size.x == paddle.size.x
           && rect->rect.size.y == paddle.size.y)
        {
            ++paddleRects;
        }
        if(rect->rect.pos.x == hazard.pos.x && rect->rect.pos.y == hazard.pos.y && rect->rect.size.x == hazard.size.x
           && rect->rect.size.y == hazard.size.y)
        {
            ++hazardRects;
        }
    }

    EXPECT_EQ(paddleRects, 1);
    EXPECT_EQ(hazardRects, 1);
}

TEST(PlaySceneTest, diedReachesGameOver)
{
    Fixture f;
    // Full-width paddle, and one 0.5 s step drops a freshly spawned hazard onto it.
    f.services.tuning.paddleW = sgl::dodger::kWorldW;
    f.services.tuning.hazardH = 80.f;
    f.services.tuning.spawnInterval = 0.5f;
    f.services.tuning.hazardSpeed = (sgl::dodger::kPaddleY - 20.f + f.services.tuning.hazardH) / 0.5f;
    f.stack.push(sgl::dodger::play(f.services)());

    auto* play = dynamic_cast<sgl::dodger::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_FALSE(play->game().over());

    f.stack.update(sgl::InputState{}, sgl::Seconds{0.5f});

    auto* over = dynamic_cast<sgl::dodger::GameOverScene*>(f.stack.top());
    ASSERT_NE(over, nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_EQ(over->score(), 0);
    EXPECT_FALSE(over->traits().pausable);
}
