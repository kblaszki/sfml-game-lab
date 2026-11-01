#include <gtest/gtest.h>
#include <sgl/core/Time.hpp>
#include <sgl/input/InputEvent.hpp>
#include <sgl/input/InputState.hpp>
#include <sgl/input/Key.hpp>
#include <sgl/scene/SceneStack.hpp>
#include <snake/app/Bindings.hpp>
#include <snake/app/PauseScene.hpp>
#include <snake/app/PlayScene.hpp>
#include <snake/app/Scenes.hpp>

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

TEST(ScenesTest, menuCancelQuits)
{
    Fixture f;
    f.stack.push(sgl::snake::mainMenu(f.services)());

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_TRUE(f.stack.quitRequested());
}

TEST(ScenesTest, playFactoryPushesPlayableScene)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    auto* play = dynamic_cast<sgl::snake::PlayScene*>(f.stack.top());
    ASSERT_NE(play, nullptr);
    EXPECT_TRUE(play->traits().pausable);
    EXPECT_EQ(f.stack.size(), 1u);
}

TEST(ScenesTest, escapeRequestsPause)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());

    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 2u);
    auto* pause = dynamic_cast<sgl::snake::PauseScene*>(f.stack.top());
    EXPECT_NE(pause, nullptr);
}

TEST(ScenesTest, pauseCancelResumes)
{
    Fixture f;
    f.stack.push(sgl::snake::play(f.services)());
    f.stack.update(inputWith(sgl::Key::Escape, f.map), sgl::kTick);
    ASSERT_EQ(f.stack.size(), 2u);

    f.stack.update(inputWith(sgl::Key::Backspace, f.map), sgl::kTick);

    EXPECT_EQ(f.stack.size(), 1u);
    EXPECT_NE(dynamic_cast<sgl::snake::PlayScene*>(f.stack.top()), nullptr);
}
