#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <snake/app/Bindings.hpp>
#include <snake/app/GameOverScene.hpp>
#include <snake/app/PlayScene.hpp>
#include <snake/app/Scenes.hpp>
#include <snake/sim/Types.hpp>

namespace
{

struct Fixture
{
    Fixture()
        : map{}
        , actions{sgl::snake::bindSnake(map)}
        , font{}
        , services{actions, font, 1}
        , stack{sgl::snake::pause(services)}
    {
    }

    sgl::ActionMap map;
    sgl::snake::SnakeActions actions;
    sgl::FontId font;
    sgl::snake::AppServices services;
    sgl::SceneStack stack;
};

sgl::InputState inputWith(sgl::Key key, const sgl::ActionMap& map)
{
    sgl::InputState input;
    input.apply(sgl::KeyDown{.key = key}, map);
    return input;
}

} // namespace

TEST(PlaySceneTest, stepsSimWithInputState)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    auto* play = dynamic_cast<sgl::snake::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    ASSERT_FALSE(play->game().body().empty());
    const sgl::snake::Cell before = play->game().body().front();
    EXPECT_EQ(play->game().dir(), sgl::snake::Dir::Right);

    f.stack.update(inputWith(sgl::Key::Up, f.map), sgl::Seconds{0.15f});

    EXPECT_EQ(play->game().dir(), sgl::snake::Dir::Up);
    EXPECT_EQ(play->game().body().front(), (sgl::snake::Cell{before.x, before.y + 1}));
    EXPECT_FALSE(play->game().over());
}

TEST(PlaySceneTest, diesAfterRunningIntoWall)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    sgl::InputState input;
    sgl::snake::GameOverScene* over = nullptr;
    for(int i = 0; i < 300 && over == nullptr; ++i)
    {
        f.stack.update(input, sgl::kTick);
        over = dynamic_cast<sgl::snake::GameOverScene*>(f.stack.top());
    }

    ASSERT_NE(over, nullptr);
    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_GE(over->score(), 0);
}
